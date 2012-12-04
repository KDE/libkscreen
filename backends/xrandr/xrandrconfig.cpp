/*
 * Copyright 2012  Dan Vrátil <dvratil@redhat.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "xrandrconfig.h"
#include "xrandrscreen.h"
#include "xrandr.h"
#include "xrandrmode.h"
#include "xrandroutput.h"
#include "config.h"
#include "output.h"

#include <QX11Info>
#include <QDebug>

using namespace KScreen;

XRandRConfig::XRandRConfig(XRandR *xrandr)
    : QObject(xrandr)
    , m_screen(new XRandRScreen(this))
{
    XRRScreenResources* resources = xrandr->screenResources();

    RROutput id, primary;
    XRROutputInfo* outputInfo;
    primary = XRRGetOutputPrimary(backend()->display(), backend()->rootWindow());

    qDebug() << "\t" << "Primary: " << primary;

    XRandROutput::Map outputs;
    for (int i = 0; i < resources->noutput; ++i)
    {
        id = resources->outputs[i];
        outputInfo = xrandr->XRROutput(id);

        XRandROutput *output = new XRandROutput(id, outputInfo, this);
        output->setOutputProperty(XRandROutput::PropertyPrimary, id == primary);
        m_outputs.insert(id, output);

        XRRFreeOutputInfo(outputInfo);
    }

    XRRFreeScreenResources(resources);
}

XRandRConfig::~XRandRConfig()
{
}

XRandR *XRandRConfig::backend() const
{
    return qobject_cast<XRandR*>(parent());
}

XRandROutput::Map XRandRConfig::outputs() const
{
    return m_outputs;
}

KScreen::Config *XRandRConfig::toKScreenConfig() const
{
    KScreen::Config *config = new KScreen::Config();
    KScreen::OutputList kscreenOutputs;

    Q_FOREACH (const XRandROutput *output, m_outputs) {
        KScreen::Output *kscreenOutput = output->toKScreenOutput(config);
        kscreenOutputs.insert(kscreenOutput->id(), kscreenOutput);
    }

    config->setOutputs(kscreenOutputs);
    config->setScreen(m_screen->toKScreenScreen(config));

    return config;
}

void XRandRConfig::applyKScreenConfig(KScreen::Config *config)
{
    KScreen::OutputList outputs = config->outputs();
    QSize newSize = screenSize(config);

    int neededCrtc = 0;
    int primaryOutput = 0;
    KScreen::OutputList toDisable, toEnable, toChange;
    QHash<int, int> currentCrtc;
    Q_FOREACH(KScreen::Output *output, outputs) {
        XRandROutput *currentOutput = m_outputs.value(output->id());

        if (output->isPrimary()) {
            primaryOutput = currentOutput->outputProperty(XRandROutput::PropertyId).toInt();
        }

        bool currentEnabled = currentOutput->outputProperty(XRandROutput::PropertyEnabled).toBool();
        if (!output->isEnabled() && currentEnabled) {
            toDisable.insert(output->id(), output);
            continue;
        } else if (output->isEnabled() && !currentEnabled) {
            toEnable.insert(output->id(), output);
            neededCrtc ++;
            continue;
        } else if (!output->isEnabled() && !currentEnabled) {
            continue;
        }

        neededCrtc ++;

        if (output->currentMode() != currentOutput->outputProperty(XRandROutput::PropertyCurrentMode).toInt()) {
            if (!toChange.contains(output->id())) {
                currentCrtc.insert(output->id(), backend()->outputCrtc(output->id()));
                toChange.insert(output->id(), output);
                if (!toDisable.contains(output->id())) {
                    toDisable.insert(output->id(), output);
                }
            }
        }

        if (output->pos() != currentOutput->outputProperty(XRandROutput::PropertyPos).toPoint()) {
            if (!toChange.contains(output->id())) {
                currentCrtc.insert(output->id(), backend()->outputCrtc(output->id()));
                toChange.insert(output->id(), output);
                if (!toDisable.contains(output->id())) {
                    toDisable.insert(output->id(), output);
                }
            }
        }

        if (output->rotation() != (KScreen::Output::Rotation) currentOutput->outputProperty(XRandROutput::PropertyRotation).toInt()) {
            if( !toChange.contains(output->id())) {
                currentCrtc.insert(output->id(), backend()->outputCrtc(output->id()));
                toChange.insert(output->id(), output);
            }
        }
    }

    KScreen::Screen* screen = config->screen();
    if (newSize.width() > screen->maxSize().width() ||
        newSize.height() > screen->maxSize().height()) {
        qDebug() << "The new size is too big: " << newSize << " - " << screen->maxSize();
        return;//Too big
    }

    qDebug() << neededCrtc;
    XRRScreenResources *screenResources = backend()->screenResources();
    if (neededCrtc > screenResources->ncrtc) {
        qDebug() << "We need more crtc than we have: " << neededCrtc << " - " << screenResources->ncrtc;
        XRRFreeScreenResources(screenResources);
        return;//We don't have enough crtc
    }
    XRRFreeScreenResources(screenResources);

    if (primaryOutput) {
        setPrimaryOutput(primaryOutput);
    }

    Q_FOREACH(KScreen::Output* output, toDisable) {
        disableOutput(output);
    }

    setScreenSize(newSize);

    Q_FOREACH(KScreen::Output* output, toEnable) {
        enableOutput(output);
    }

    Q_FOREACH(KScreen::Output* output, toChange) {
        changeOutput(output, currentCrtc[output->id()]);
    }
}


QSize XRandRConfig::screenSize(Config* config) const
{
    int cord;
    QSize size, outputSize;
    OutputList outputs = config->outputs();
    Q_FOREACH(const Output* output, outputs) {
        if (!output->isEnabled() || !output->isConnected()) {
            continue;
        }

        Mode *currentMode = output->mode(output->currentMode());
        if (currentMode) {
            outputSize = currentMode->size();
            cord = output->pos().y() + outputSize.height();
            if (cord > size.height()) {
                size.setHeight(cord);
            }

            cord = output->pos().x() + outputSize.width();
            if (cord > size.width()) {
                size.setWidth(cord);
            }
        }
    }

    return size;
}

bool XRandRConfig::setScreenSize(const QSize& size) const
{
    double dpi;
    int widthMM, heightMM;
    dpi = (25.4 * DisplayHeight(backend()->display(), backend()->screen())) / DisplayHeightMM(backend()->display(), backend()->screen());

    qDebug() << "DPI: " << dpi;
    qDebug() << "Size: " << size;

    widthMM =  ((25.4 * size.width()) / dpi);
    heightMM = ((25.4 * size.height()) / dpi);

    qDebug() << "MM: " << widthMM << "x" << heightMM;

    qDebug() << size << " " << widthMM << "x" << heightMM;
    XRRSetScreenSize(backend()->display(), backend()->rootWindow(),
                     size.width(), size.height(), widthMM, heightMM);

    return true;
}

void XRandRConfig::setPrimaryOutput(int outputId) const
{
    XRRSetOutputPrimary(backend()->display(), backend()->rootWindow(), outputId);
}

void XRandRConfig::disableOutput(Output* output) const
{
    qDebug() << "Disabling: " << output->id();
    int crtcId = backend()->outputCrtc(output->id());
    qDebug() << crtcId;
    XRRSetCrtcConfig (backend()->display(), backend()->screenResources(), crtcId, CurrentTime,
                 0, 0, None, RR_Rotate_0, NULL, 0);
}

void XRandRConfig::enableOutput(Output* output) const
{
    qDebug() << "Enabling: " << output->id();
    RROutput *outputs = new RROutput[1];
    outputs[0] = output->id();
    Status s = XRRSetCrtcConfig(backend()->display(), backend()->screenResources(), backend()->freeCrtc(),
        CurrentTime, output->pos().rx(), output->pos().ry(), output->currentMode(),
        output->rotation(), outputs, 1);

    Q_UNUSED(s);
}

void XRandRConfig::changeOutput(Output* output, int crtcId) const
{
    qDebug() << "Updating: " << output->id();

    RROutput *outputs = new RROutput[1];
    outputs[0] = output->id();
    Status s = XRRSetCrtcConfig(backend()->display(), backend()->screenResources(), crtcId,
        CurrentTime, output->pos().rx(), output->pos().ry(), output->currentMode(),
        output->rotation(), outputs, 1);

    Q_UNUSED(s);
}


#include "xrandrconfig.moc"
