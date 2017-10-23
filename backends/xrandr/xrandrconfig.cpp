/*************************************************************************************
 *  Copyright (C) 2012 by Alejandro Fiestas Olivares <afiestas@kde.org>              *
 *  Copyright (C) 2012 - 2015 by Daniel Vrátil <dvratil@redhat.com>                  *
 *                                                                                   *
 *  This library is free software; you can redistribute it and/or                    *
 *  modify it under the terms of the GNU Lesser General Public                       *
 *  License as published by the Free Software Foundation; either                     *
 *  version 2.1 of the License, or (at your option) any later version.               *
 *                                                                                   *
 *  This library is distributed in the hope that it will be useful,                  *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of                   *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU                *
 *  Lesser General Public License for more details.                                  *
 *                                                                                   *
 *  You should have received a copy of the GNU Lesser General Public                 *
 *  License along with this library; if not, write to the Free Software              *
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA       *
 *************************************************************************************/

#include "xrandrconfig.h"
#include "xrandrscreen.h"
#include "xrandr.h"
#include "xrandrmode.h"
#include "xrandroutput.h"
#include "xrandrcrtc.h"
#include "config.h"
#include "output.h"
#include "edid.h"

#include "../xcbwrapper.h"

#include <QX11Info>
#include <QRect>
#include <QScopedPointer>

using namespace KScreen;

XRandRConfig::XRandRConfig()
    : QObject()
    , m_screen(nullptr)
{
    m_screen = new XRandRScreen(this);

    XCB::ScopedPointer<xcb_randr_get_screen_resources_reply_t> resources(XRandR::screenResources());
    xcb_randr_crtc_t *crtcs = xcb_randr_get_screen_resources_crtcs(resources.data());
    for (int i = 0, c = xcb_randr_get_screen_resources_crtcs_length(resources.data()); i < c; ++i) {
        addNewCrtc(crtcs[i]);
    }

    xcb_randr_output_t *outputs = xcb_randr_get_screen_resources_outputs(resources.data());
    for (int i = 0, c = xcb_randr_get_screen_resources_outputs_length(resources.data()); i < c; ++i) {
        addNewOutput(outputs[i]);
    }
}

XRandRConfig::~XRandRConfig()
{
    qDeleteAll(m_outputs);
    qDeleteAll(m_crtcs);
    delete m_screen;
}

XRandROutput::Map XRandRConfig::outputs() const
{
    return m_outputs;
}

XRandROutput* XRandRConfig::output(xcb_randr_output_t output) const
{
    return m_outputs[output];
}

XRandRCrtc::Map XRandRConfig::crtcs() const
{
    return m_crtcs;
}

XRandRCrtc* XRandRConfig::crtc(xcb_randr_crtc_t crtc) const
{
    return m_crtcs[crtc];
}

XRandRScreen* XRandRConfig::screen() const
{
    return m_screen;
}


void XRandRConfig::addNewOutput(xcb_randr_output_t id)
{
    XRandROutput *xOutput = new XRandROutput(id, this);
    m_outputs.insert(id, xOutput);
}

void XRandRConfig::addNewCrtc(xcb_randr_crtc_t crtc)
{
    m_crtcs.insert(crtc, new XRandRCrtc(crtc, this));
}

void XRandRConfig::removeOutput(xcb_randr_output_t id)
{
    delete m_outputs.take(id);
}

KScreen::ConfigPtr XRandRConfig::toKScreenConfig() const
{
    KScreen::ConfigPtr config(new KScreen::Config);
    auto features = Config::Feature::Writable | Config::Feature::PrimaryDisplay;
    config->setSupportedFeatures(features);
    KScreen::OutputList kscreenOutputs;

    for (auto iter = m_outputs.constBegin(); iter != m_outputs.constEnd(); ++iter) {
        KScreen::OutputPtr kscreenOutput = (*iter)->toKScreenOutput();
        kscreenOutputs.insert(kscreenOutput->id(), kscreenOutput);
    }
    config->setOutputs(kscreenOutputs);
    config->setScreen(m_screen->toKScreenScreen());

    return config;
}

void XRandRConfig::applyKScreenConfig(const KScreen::ConfigPtr &config)
{
    const KScreen::OutputList kscreenOutputs = config->outputs();
    const QSize newScreenSize = screenSize(config);
    const QSize currentScreenSize = m_screen->currentSize();
    // When the current screen configuration is bigger than the new size (like
    // when rotating an output), the XSetScreenSize can fail or apply the smaller
    // size only partially, because we apply the size (we have to) before the
    // output changes. To prevent all kinds of weird screen sizes from happening,
    // we initially set such screen size, that it can take the current as well
    // as the new configuration, then we apply the output changes, and finally then
    // (if necessary) we reduce the screen size to fix the new configuration precisely.
    const QSize intermediateScreenSize = QSize(qMax(newScreenSize.width(), currentScreenSize.width()),
                                               qMax(newScreenSize.height(), currentScreenSize.height()));
    int neededCrtcs = 0;
    xcb_randr_output_t primaryOutput = 0;
    xcb_randr_output_t oldPrimaryOutput = 0;

    Q_FOREACH (const XRandROutput *xrandrOutput, m_outputs) {
        if (xrandrOutput->isPrimary()) {
            oldPrimaryOutput = xrandrOutput->id();
            break;
        }
    }

    KScreen::OutputList toDisable, toEnable, toChange;
    Q_FOREACH(const KScreen::OutputPtr &kscreenOutput, kscreenOutputs) {
        xcb_randr_output_t outputId = kscreenOutput->id();
        XRandROutput *currentOutput = output(outputId);
        //Only set the output as primary if it is enabled.
        if (kscreenOutput->isPrimary() && kscreenOutput->isEnabled()) {
            primaryOutput = outputId;
        }

        const bool currentEnabled = currentOutput->isEnabled();
        if (!kscreenOutput->isEnabled() && currentEnabled) {
            toDisable.insert(outputId, kscreenOutput);
            continue;
        } else if (kscreenOutput->isEnabled() && !currentEnabled) {
            toEnable.insert(outputId, kscreenOutput);
            ++neededCrtcs;
            continue;
        } else if (!kscreenOutput->isEnabled() && !currentEnabled) {
            continue;
        }

        ++neededCrtcs;

        if (kscreenOutput->currentModeId() != currentOutput->currentModeId()) {
            if (!toChange.contains(outputId)) {
                toChange.insert(outputId, kscreenOutput);
            }
        }

        if (kscreenOutput->pos() != currentOutput->position()) {
            if (!toChange.contains(outputId)) {
                toChange.insert(outputId, kscreenOutput);
            }
        }

        if (kscreenOutput->rotation() != currentOutput->rotation()) {
            if (!toChange.contains(outputId)) {
                toChange.insert(outputId, kscreenOutput);
            }
        }

        XRandRMode *currentMode = currentOutput->modes().value(kscreenOutput->currentModeId().toInt());

        // For some reason, in some environments currentMode is null
        // which doesn't make sense because it is the *current* mode...
        // Since we haven't been able to figure out the reason why
        // this happens, we are adding this debug code to try to
        // figure out how this happened.
        if (!currentMode) {
            qWarning() << "Current mode is null:"
                       << "ModeId:" << currentOutput->currentModeId()
                       << "Mode: " << currentOutput->currentMode()
                       << "Output: " << currentOutput->id();
//             qDebug() << kRealBacktrace(256);
            printConfig(config);
            printInternalCond();
            continue;
        }

        // If the output would not fit into new screen size, we need to disable
        // it and reposition it
        const QRect geom = kscreenOutput->geometry();
        if (geom.right() > newScreenSize.width() || geom.bottom() > newScreenSize.height()) {
            if (!toDisable.contains(outputId)) {
                qCDebug(KSCREEN_XRANDR) << "The new output would not fit into screen - new geometry: " << geom << ", new screen size:" << newScreenSize;
                toDisable.insert(outputId, kscreenOutput);
            }
        }
    } // Q_FOREACH (const KScreen::OutputPtr &kscreenOutput, kscreenOutputs)

    const KScreen::ScreenPtr kscreenScreen = config->screen();
    if (newScreenSize.width() > kscreenScreen->maxSize().width() ||
        newScreenSize.height() > kscreenScreen->maxSize().height()) {
        qCDebug(KSCREEN_XRANDR) << "The new screen size is too big - requested: " << newScreenSize << ", maximum: " << kscreenScreen->maxSize();
        return;
    }

    qCDebug(KSCREEN_XRANDR) << "Needed CRTCs: " << neededCrtcs;
    XCB::ScopedPointer<xcb_randr_get_screen_resources_reply_t> screenResources(XRandR::screenResources());
    if (neededCrtcs > screenResources->num_crtcs) {
        qCDebug(KSCREEN_XRANDR) << "We need more CRTCs than we have available - requested: " << neededCrtcs << ", available: " << screenResources->num_crtcs;
            return;
    }

    qCDebug(KSCREEN_XRANDR) << "Actions to perform:";
    qCDebug(KSCREEN_XRANDR) << "\tPrimary Output:" << (primaryOutput != oldPrimaryOutput);
    if (primaryOutput != oldPrimaryOutput) {
        qCDebug(KSCREEN_XRANDR) << "\t\tOld:" << oldPrimaryOutput;
        qCDebug(KSCREEN_XRANDR) << "\t\tNew:" << primaryOutput;
    }
    qCDebug(KSCREEN_XRANDR) << "\tChange Screen Size:" << (newScreenSize != currentScreenSize);
    if (newScreenSize != currentScreenSize) {
        qCDebug(KSCREEN_XRANDR) << "\t\tOld:" << currentScreenSize;
        qCDebug(KSCREEN_XRANDR) << "\t\tIntermediate:" << intermediateScreenSize;
        qCDebug(KSCREEN_XRANDR) << "\t\tNew:" << newScreenSize;
    }
    qCDebug(KSCREEN_XRANDR) << "\tDisable outputs:" << !toDisable.isEmpty();
    if (!toDisable.isEmpty()) {
        qCDebug(KSCREEN_XRANDR) << "\t\t" << toDisable.keys();
    }
    qCDebug(KSCREEN_XRANDR) << "\tChange outputs:" << !toChange.isEmpty();
    if (!toChange.isEmpty()) {
        qCDebug(KSCREEN_XRANDR) << "\t\t" << toChange.keys();
    }
    qCDebug(KSCREEN_XRANDR) << "\tEnable outputs:" << !toEnable.isEmpty();
    if (!toEnable.isEmpty()) {
        qCDebug(KSCREEN_XRANDR) << "\t\t" << toEnable.keys();
    }

    // Grab the server so that no-one else can do changes to XRandR and to block
    // change notifications until we are done
    XCB::GrabServer grabber;

    //If there is nothing to do, not even bother
    if (oldPrimaryOutput == primaryOutput && toDisable.isEmpty() && toEnable.isEmpty() && toChange.isEmpty()) {
        if (newScreenSize != currentScreenSize) {
            setScreenSize(newScreenSize);
        }
        return;
    }

    Q_FOREACH(const KScreen::OutputPtr &output, toDisable) {
        disableOutput(output);
    }

    if (intermediateScreenSize != currentScreenSize) {
        setScreenSize(intermediateScreenSize);
    }

    bool forceScreenSizeUpdate = false;
    Q_FOREACH(const KScreen::OutputPtr &output, toChange) {
        if (!changeOutput(output)) {
            /* If we disabled the output before changing it and XRandR failed
             * to re-enable it, then update screen size too */
            if (toDisable.contains(output->id())) {
                //output->setEnabled(false);
                qCDebug(KSCREEN_XRANDR) << "Output failed to change: " << output->name();
                forceScreenSizeUpdate = true;
            }
        }
    }

    Q_FOREACH(const KScreen::OutputPtr &output, toEnable) {
        if (!enableOutput(output)) {
            //output->setEnabled(false);
            qCDebug(KSCREEN_XRANDR) << "Output failed to be Enabled: " << output->name();
            forceScreenSizeUpdate = true;
        }
    }

    if (oldPrimaryOutput != primaryOutput) {
        setPrimaryOutput(primaryOutput);
    }

    if (forceScreenSizeUpdate || intermediateScreenSize != newScreenSize) {
        QSize newSize = newScreenSize;
        if (forceScreenSizeUpdate) {
            newSize = screenSize(config);
            qCDebug(KSCREEN_XRANDR) << "Forced to change screen size: " << newSize;
        }
        setScreenSize(newSize);
    }
}

void XRandRConfig::printConfig(const ConfigPtr &config) const
{
    qCDebug(KSCREEN_XRANDR) << "KScreen version:" /*<< LIBKSCREEN_VERSION*/;

    if (!config) {
        qCDebug(KSCREEN_XRANDR) << "Config is invalid";
        return;
    }
    if (!config->screen()) {
        qCDebug(KSCREEN_XRANDR) << "No screen in the configuration, broken backend";
        return;
    }

    qCDebug(KSCREEN_XRANDR) << "Screen:";
    qCDebug(KSCREEN_XRANDR) << "\tmaxSize:" << config->screen()->maxSize();
    qCDebug(KSCREEN_XRANDR) << "\tminSize:" << config->screen()->minSize();
    qCDebug(KSCREEN_XRANDR) << "\tcurrentSize:" << config->screen()->currentSize();

    OutputList outputs = config->outputs();
    Q_FOREACH(const OutputPtr &output, outputs) {
        qCDebug(KSCREEN_XRANDR) << "\n-----------------------------------------------------\n";
        qCDebug(KSCREEN_XRANDR) << "Id: " << output->id();
        qCDebug(KSCREEN_XRANDR) << "Name: " << output->name();
        qCDebug(KSCREEN_XRANDR) << "Type: " << output->type();
        qCDebug(KSCREEN_XRANDR) << "Connected: " << output->isConnected();
        if (!output->isConnected()) {
            continue;
        }
        qCDebug(KSCREEN_XRANDR) << "Enabled: " << output->isEnabled();
        qCDebug(KSCREEN_XRANDR) << "Primary: " << output->isPrimary();
        qCDebug(KSCREEN_XRANDR) << "Rotation: " << output->rotation();
        qCDebug(KSCREEN_XRANDR) << "Pos: " << output->pos();
        qCDebug(KSCREEN_XRANDR) << "MMSize: " << output->sizeMm();
        if (output->currentMode()) {
            qCDebug(KSCREEN_XRANDR) << "Size: " << output->currentMode()->size();
        }
        if (output->clones().isEmpty()) {
            qCDebug(KSCREEN_XRANDR) << "Clones: " << "None";
        } else {
            qCDebug(KSCREEN_XRANDR) << "Clones: " << output->clones().count();
        }
        qCDebug(KSCREEN_XRANDR) << "Mode: " << output->currentModeId();
        qCDebug(KSCREEN_XRANDR) << "Preferred Mode: " << output->preferredModeId();
        qCDebug(KSCREEN_XRANDR) << "Preferred modes: " << output->preferredModes();
        qCDebug(KSCREEN_XRANDR) << "Modes: ";

        ModeList modes = output->modes();
        Q_FOREACH(const ModePtr &mode, modes) {
            qCDebug(KSCREEN_XRANDR) << "\t" << mode->id() << "  " << mode->name() << " " << mode->size() << " " << mode->refreshRate();
        }

        Edid* edid = output->edid();
        qCDebug(KSCREEN_XRANDR) << "EDID Info: ";
        if (edid && edid->isValid()) {
            qCDebug(KSCREEN_XRANDR) << "\tDevice ID: " << edid->deviceId();
            qCDebug(KSCREEN_XRANDR) << "\tName: " << edid->name();
            qCDebug(KSCREEN_XRANDR) << "\tVendor: " << edid->vendor();
            qCDebug(KSCREEN_XRANDR) << "\tSerial: " << edid->serial();
            qCDebug(KSCREEN_XRANDR) << "\tEISA ID: " << edid->eisaId();
            qCDebug(KSCREEN_XRANDR) << "\tHash: " << edid->hash();
            qCDebug(KSCREEN_XRANDR) << "\tWidth: " << edid->width();
            qCDebug(KSCREEN_XRANDR) << "\tHeight: " << edid->height();
            qCDebug(KSCREEN_XRANDR) << "\tGamma: " << edid->gamma();
            qCDebug(KSCREEN_XRANDR) << "\tRed: " << edid->red();
            qCDebug(KSCREEN_XRANDR) << "\tGreen: " << edid->green();
            qCDebug(KSCREEN_XRANDR) << "\tBlue: " << edid->blue();
            qCDebug(KSCREEN_XRANDR) << "\tWhite: " << edid->white();
        } else {
            qCDebug(KSCREEN_XRANDR) << "\tUnavailable";
        }
    }
}

void XRandRConfig::printInternalCond() const
{
    qCDebug(KSCREEN_XRANDR) << "Internal config in xrandr";
    Q_FOREACH(XRandROutput *output, m_outputs) {
        qCDebug(KSCREEN_XRANDR) << "Id: " << output->id();
        qCDebug(KSCREEN_XRANDR) << "Current Mode: " << output->currentMode();
        qCDebug(KSCREEN_XRANDR) << "Current mode id: " << output->currentModeId();
        qCDebug(KSCREEN_XRANDR) << "Connected: " << output->isConnected();
        qCDebug(KSCREEN_XRANDR) << "Enabled: " << output->isEnabled();
        qCDebug(KSCREEN_XRANDR) << "Primary: " << output->isPrimary();
        if (!output->isEnabled()) {
            continue;
        }
        XRandRMode::Map modes = output->modes();
        Q_FOREACH(XRandRMode *mode, modes) {
            qCDebug(KSCREEN_XRANDR) << "\t" << mode->id();
            qCDebug(KSCREEN_XRANDR) << "\t" << mode->name();
            qCDebug(KSCREEN_XRANDR) << "\t" << mode->size() << mode->refreshRate();
        }
    }
}

QSize XRandRConfig::screenSize(const KScreen::ConfigPtr &config) const
{
    QRect rect;
    Q_FOREACH(const KScreen::OutputPtr &output, config->outputs()) {
        if (!output->isConnected() || !output->isEnabled()) {
            continue;
        }

        const ModePtr currentMode = output->currentMode();
        if (!currentMode) {
            qCDebug(KSCREEN_XRANDR) << "Output: " << output->name() << " has no current Mode!";
            continue;
        }

        const QRect outputGeom = output->geometry();
        rect = rect.united(outputGeom);
    }

    const QSize size = QSize(rect.width(), rect.height());
    qCDebug(KSCREEN_XRANDR) << "Requested screen size is" << size;
    return size;
}

bool XRandRConfig::setScreenSize(const QSize &size) const
{
    const double dpi = (25.4 * XRandR::screen()->height_in_pixels / XRandR::screen()->height_in_millimeters);
    const int widthMM =  ((25.4 * size.width()) / dpi);
    const int heightMM = ((25.4 * size.height()) / dpi);

    qCDebug(KSCREEN_XRANDR) << "RRSetScreenSize";
    qCDebug(KSCREEN_XRANDR) << "\tDPI:" << dpi;
    qCDebug(KSCREEN_XRANDR) << "\tSize:" << size;
    qCDebug(KSCREEN_XRANDR) << "\tSizeMM:" << QSize(widthMM, heightMM);

    xcb_randr_set_screen_size(XCB::connection(), XRandR::rootWindow(),
                              size.width(), size.height(), widthMM, heightMM);
    m_screen->update(size);
    return true;
}

void XRandRConfig::setPrimaryOutput(xcb_randr_output_t outputId) const
{
    qCDebug(KSCREEN_XRANDR) << "RRSetOutputPrimary";
    qCDebug(KSCREEN_XRANDR) << "\tNew primary:" << outputId;
    xcb_randr_set_output_primary(XCB::connection(), XRandR::rootWindow(), outputId);

    for (XRandROutput *output : m_outputs) {
        output->setIsPrimary(output->id() == outputId);
    }
}

bool XRandRConfig::disableOutput(const OutputPtr &kscreenOutput) const
{
    XRandROutput *xOutput = output(kscreenOutput->id());
    Q_ASSERT(xOutput);
    Q_ASSERT(xOutput->crtc());
    if (!xOutput->crtc()) {
        qCWarning(KSCREEN_XRANDR) << "Attempting to disable output without CRTC, wth?";
        return false;
    }

    const xcb_randr_crtc_t crtc = xOutput->crtc()->crtc();

    qCDebug(KSCREEN_XRANDR) << "RRSetCrtcConfig (disable output)";
    qCDebug(KSCREEN_XRANDR) << "\tCRTC:" << crtc;

    auto cookie = xcb_randr_set_crtc_config(XCB::connection(), crtc,
            XCB_CURRENT_TIME, XCB_CURRENT_TIME,
            0, 0,
            XCB_NONE,
            XCB_RANDR_ROTATION_ROTATE_0,
            0, NULL);
    XCB::ScopedPointer<xcb_randr_set_crtc_config_reply_t> reply(xcb_randr_set_crtc_config_reply(XCB::connection(), cookie, NULL));
    if (!reply) {
        qCDebug(KSCREEN_XRANDR) << "\tResult: unknown (error)";
        return false;
    }
    qCDebug(KSCREEN_XRANDR) << "\tResult:" << reply->status;

    // Update the cached output now, otherwise we get RRNotify_CrtcChange notification
    // for an outdated output, which can lead to a crash.
    if (reply->status == XCB_RANDR_SET_CONFIG_SUCCESS) {
        xOutput->update(XCB_NONE, XCB_NONE, xOutput->isConnected() ? XCB_RANDR_CONNECTION_CONNECTED : XCB_RANDR_CONNECTION_DISCONNECTED,
                        kscreenOutput->isPrimary());
    }
    return (reply->status == XCB_RANDR_SET_CONFIG_SUCCESS);
}

bool XRandRConfig::enableOutput(const OutputPtr &kscreenOutput) const
{
    xcb_randr_output_t outputs[1] { static_cast<xcb_randr_output_t>(kscreenOutput->id()) };

    XRandRCrtc *freeCrtc = nullptr;
    qCDebug(KSCREEN_XRANDR) << m_crtcs;
    Q_FOREACH (XRandRCrtc *crtc, m_crtcs) {
        crtc->update();
        qCDebug(KSCREEN_XRANDR) << "Testing CRTC" << crtc->crtc();
        qCDebug(KSCREEN_XRANDR) << "\tFree:" << crtc->isFree();
        qCDebug(KSCREEN_XRANDR) << "\tMode:" << crtc->mode();
        qCDebug(KSCREEN_XRANDR) << "\tPossible outputs:" << crtc->possibleOutputs();
        qCDebug(KSCREEN_XRANDR) << "\tConnected outputs:" << crtc->outputs();
        qCDebug(KSCREEN_XRANDR) << "\tGeometry:" << crtc->geometry();
        if (crtc->isFree() && crtc->possibleOutputs().contains(kscreenOutput->id())) {
            freeCrtc = crtc;
            break;
        }
    }
    if (!freeCrtc) {
        qCWarning(KSCREEN_XRANDR) << "Failed to get free CRTC for output" << kscreenOutput->id();
        return false;
    }

    const int modeId = kscreenOutput->currentMode() ? kscreenOutput->currentModeId().toInt() : kscreenOutput->preferredModeId().toInt();

    qCDebug(KSCREEN_XRANDR) << "RRSetCrtcConfig (enable output)";
    qCDebug(KSCREEN_XRANDR) << "\tOutput:" << kscreenOutput->id() << "(" << kscreenOutput->name() << ")";
    qCDebug(KSCREEN_XRANDR) << "\tNew CRTC:" << freeCrtc->crtc();
    qCDebug(KSCREEN_XRANDR) << "\tPos:" << kscreenOutput->pos();
    qCDebug(KSCREEN_XRANDR) << "\tMode:" << kscreenOutput->currentMode() << "Preferred:" << kscreenOutput->preferredModeId();
    qCDebug(KSCREEN_XRANDR) << "\tRotation:" << kscreenOutput->rotation();

    auto cookie = xcb_randr_set_crtc_config(XCB::connection(), freeCrtc->crtc(),
            XCB_CURRENT_TIME, XCB_CURRENT_TIME,
            kscreenOutput->pos().rx(), kscreenOutput->pos().ry(),
            modeId,
            kscreenOutput->rotation(),
            1, outputs);
    XCB::ScopedPointer<xcb_randr_set_crtc_config_reply_t> reply(xcb_randr_set_crtc_config_reply(XCB::connection(), cookie, NULL));
    if (!reply) {
        qCDebug(KSCREEN_XRANDR) << "Result: unknown (error)";
        return false;
    }
    qCDebug(KSCREEN_XRANDR) << "\tResult:" << reply->status;

    if (reply->status == XCB_RANDR_SET_CONFIG_SUCCESS) {
        XRandROutput *xOutput = output(kscreenOutput->id());
        xOutput->update(freeCrtc->crtc(), modeId,
                        XCB_RANDR_CONNECTION_CONNECTED, kscreenOutput->isPrimary());
    }
    return (reply->status == XCB_RANDR_SET_CONFIG_SUCCESS);
}

bool XRandRConfig::changeOutput(const OutputPtr &kscreenOutput) const
{
    XRandROutput *xOutput = output(kscreenOutput->id());
    Q_ASSERT(xOutput);
    if (!xOutput->crtc()) {
        qCDebug(KSCREEN_XRANDR) << "Output" << kscreenOutput->id() << "has no CRTC, falling back to enableOutput()";
        return enableOutput(kscreenOutput);
    }

    int modeId = kscreenOutput->currentMode() ? kscreenOutput->currentModeId().toInt() : kscreenOutput->preferredModeId().toInt();

    qCDebug(KSCREEN_XRANDR) << "RRSetCrtcConfig (change output)";
    qCDebug(KSCREEN_XRANDR) << "\tOutput:" << kscreenOutput->id() << "(" << kscreenOutput->name() << ")";
    qCDebug(KSCREEN_XRANDR) << "\tCRTC:" << xOutput->crtc()->crtc();
    qCDebug(KSCREEN_XRANDR) << "\tPos:" << kscreenOutput->pos();
    qCDebug(KSCREEN_XRANDR) << "\tMode:" << modeId  << kscreenOutput->currentMode();
    qCDebug(KSCREEN_XRANDR) << "\tRotation:" << kscreenOutput->rotation();

    xcb_randr_output_t outputs[1] { static_cast<xcb_randr_output_t>(kscreenOutput->id()) };

    auto cookie = xcb_randr_set_crtc_config(XCB::connection(), xOutput->crtc()->crtc(),
            XCB_CURRENT_TIME, XCB_CURRENT_TIME,
            kscreenOutput->pos().rx(), kscreenOutput->pos().ry(),
            modeId,
            kscreenOutput->rotation(),
            1, outputs);
    XCB::ScopedPointer<xcb_randr_set_crtc_config_reply_t> reply(xcb_randr_set_crtc_config_reply(XCB::connection(), cookie, NULL));
    if (!reply) {
        qCDebug(KSCREEN_XRANDR) << "\tResult: unknown (error)";
        return false;
    }
    qCDebug(KSCREEN_XRANDR) << "\tResult: " << reply->status;

    if (reply->status == XCB_RANDR_SET_CONFIG_SUCCESS) {
        xOutput->update(xOutput->crtc()->crtc(), modeId,
                        XCB_RANDR_CONNECTION_CONNECTED, kscreenOutput->isPrimary());
    }
    return (reply->status == XCB_RANDR_SET_CONFIG_SUCCESS);
}
