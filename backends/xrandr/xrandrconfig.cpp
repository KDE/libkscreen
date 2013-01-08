/*************************************************************************************
 *  Copyright (C) 2012 by Alejandro Fiestas Olivares <afiestas@kde.org>              *
 *  Copyright (C) 2012 by Dan Vrátil <dvratil@redhat.com>                            *
 *                                                                                   *
 *  This program is free software; you can redistribute it and/or                    *
 *  modify it under the terms of the GNU General Public License                      *
 *  as published by the Free Software Foundation; either version 2                   *
 *  of the License, or (at your option) any later version.                           *
 *                                                                                   *
 *  This program is distributed in the hope that it will be useful,                  *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of                   *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                    *
 *  GNU General Public License for more details.                                     *
 *                                                                                   *
 *  You should have received a copy of the GNU General Public License                *
 *  along with this program; if not, write to the Free Software                      *
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA   *
 *************************************************************************************/

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

XRandRConfig::XRandRConfig()
    : QObject()
    , m_screen(new XRandRScreen(this))
{
    XRRScreenResources* resources = XRandR::screenResources();

    RROutput id, primary;
    primary = XRRGetOutputPrimary(XRandR::display(), XRandR::rootWindow());

    XRandROutput::Map outputs;
    for (int i = 0; i < resources->noutput; ++i)
    {
        id = resources->outputs[i];

        XRandROutput *output = new XRandROutput(id, (id == primary), this);
        m_outputs.insert(id, output);
    }

    XRRFreeScreenResources(resources);
}

XRandRConfig::~XRandRConfig()
{
}

void XRandRConfig::update()
{
    m_screen->update();

    RROutput primary = XRRGetOutputPrimary(XRandR::display(), XRandR::rootWindow());

    XRandROutput::Map::Iterator iter;
    for (iter = m_outputs.begin(); iter != m_outputs.end(); iter++) {
        XRandROutput *output = iter.value();
        output->update(iter.key() == (int) primary);
    }
}

XRandROutput::Map XRandRConfig::outputs() const
{
    return m_outputs;
}

KScreen::Config *XRandRConfig::toKScreenConfig() const
{
    KScreen::Config *config = new KScreen::Config();
    KScreen::OutputList kscreenOutputs;

    XRandROutput::Map::ConstIterator iter;
    for (iter = m_outputs.constBegin(); iter != m_outputs.constEnd(); iter++) {
        XRandROutput *output = iter.value();
        KScreen::Output *kscreenOutput = output->toKScreenOutput(config);
        kscreenOutputs.insert(kscreenOutput->id(), kscreenOutput);
    }

    config->setOutputs(kscreenOutputs);
    config->setScreen(m_screen->toKScreenScreen(config));

    return config;
}

void XRandRConfig::updateKScreenConfig(Config *config) const
{
    KScreen::Screen *kscreenScreen = config->screen();
    m_screen->updateKScreenScreen(kscreenScreen);

    XRandROutput::Map::ConstIterator iter;
    for (iter = m_outputs.constBegin(); iter != m_outputs.constEnd(); iter++) {
        XRandROutput *output = iter.value();
        KScreen::Output *kscreenOutput = config->output(output->id());
        output->updateKScreenOutput(kscreenOutput);
    }
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
        currentOutput->update(currentOutput->isPrimary());

        if (output->isPrimary()) {
            primaryOutput = currentOutput->id();
        }

        bool currentEnabled = currentOutput->isEnabled();
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

        if (output->currentMode() != currentOutput->currentMode()) {
            if (!toChange.contains(output->id())) {
                currentCrtc.insert(output->id(), XRandR::outputCrtc(output->id()));
                toChange.insert(output->id(), output);
            }
        }

        if (output->pos() != currentOutput->position()) {
            if (!toChange.contains(output->id())) {
                currentCrtc.insert(output->id(), XRandR::outputCrtc(output->id()));
                toChange.insert(output->id(), output);
            }
        }

        if (output->rotation() != currentOutput->rotation()) {
            if( !toChange.contains(output->id())) {
                currentCrtc.insert(output->id(), XRandR::outputCrtc(output->id()));
                toChange.insert(output->id(), output);
            }
        }

        QSize size = output->mode(currentOutput->currentMode())->size();

        int x, y;

        //TODO: Move this code within libkscreen
        y = currentOutput->position().y();
        if (currentOutput->rotation() == Output::Left || currentOutput->rotation() == Output::Right) {
            y += size.width();
        } else {
            y += size.height();
        }

        x = currentOutput->position().x();
        if (currentOutput->rotation() == Output::Left || currentOutput->rotation() == Output::Right) {
            x += size.height();
        } else {
            x += size.width();
        }

        if (x > newSize.width() || y > newSize.height()) {
            if (!toDisable.contains(output->id())) {
                qDebug() << "Output doesn't fit: " << x << "x" << y << newSize;
                toDisable.insert(output->id(), output);
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
    XRRScreenResources *screenResources = XRandR::screenResources();
    if (neededCrtc > screenResources->ncrtc) {
        qDebug() << "We need more crtc than we have: " << neededCrtc << " - " << screenResources->ncrtc;
        XRRFreeScreenResources(screenResources);
        return;//We don't have enough crtc
    }
    XRRFreeScreenResources(screenResources);

    if (primaryOutput) {
        setPrimaryOutput(primaryOutput);
    }

    //If there is nothing to do, not even bother
    if (toDisable.isEmpty() && toEnable.isEmpty() && toChange.isEmpty()) {
        return;
    }

    Q_FOREACH(KScreen::Output* output, toDisable) {
        disableOutput(output);
    }

    if (newSize != m_screen->currentSize()) {
        setScreenSize(newSize);
    }

    bool forceScreenSizeUpdate = false;
    Q_FOREACH(KScreen::Output* output, toChange) {
        if (!changeOutput(output, currentCrtc[output->id()])) {

            /* If we disabled the output before changing it and XRandR failed
             * to re-enable it, then update screen size too */
            if (toDisable.contains(output->id())) {
                output->setEnabled(false);
                forceScreenSizeUpdate = true;
            }
        }
    }

    Q_FOREACH(KScreen::Output* output, toEnable) {
        if (!enableOutput(output)) {
            output->setEnabled(false);
            forceScreenSizeUpdate = true;
        }
    }


    if (forceScreenSizeUpdate) {
        newSize = screenSize(config);
        setScreenSize(newSize);
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
            cord = output->pos().y();
            if (output->rotation() == Output::Left || output->rotation() == Output::Right) {
                cord += outputSize.width();
            } else {
                cord += outputSize.height();
            }
            if (cord > size.height()) {
                size.setHeight(cord);
            }

            cord = output->pos().x();
            if (output->rotation() == Output::Left || output->rotation() == Output::Right) {
                cord += outputSize.height();
            } else {
                cord += outputSize.width();
            }
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
    dpi = (25.4 * DisplayHeight(XRandR::display(), XRandR::screen())) / DisplayHeightMM(XRandR::display(), XRandR::screen());

    qDebug() << "DPI: " << dpi;
    qDebug() << "Size: " << size;

    widthMM =  ((25.4 * size.width()) / dpi);
    heightMM = ((25.4 * size.height()) / dpi);

    qDebug() << "MM: " << widthMM << "x" << heightMM;

    qDebug() << size << " " << widthMM << "x" << heightMM;
    XRRSetScreenSize(XRandR::display(), XRandR::rootWindow(),
                     size.width(), size.height(), widthMM, heightMM);

    return true;
}

void XRandRConfig::setPrimaryOutput(int outputId) const
{
    XRRSetOutputPrimary(XRandR::display(), XRandR::rootWindow(), outputId);
}

bool XRandRConfig::disableOutput(Output* output) const
{
    int crtcId = XRandR::outputCrtc(output->id());
    qDebug() << "Disabling: " << output->id() << "(CRTC" << crtcId << ")";
    Status s = XRRSetCrtcConfig (XRandR::display(), XRandR::screenResources(), crtcId, CurrentTime,
                 0, 0, None, RR_Rotate_0, NULL, 0);

    qDebug() << "XRRSetCrtcConfig() returned" << s;
    return (s == RRSetConfigSuccess);
}

bool XRandRConfig::enableOutput(Output* output) const
{
    qDebug() << "Enabling: " << output->id();
    RROutput *outputs = new RROutput[1];
    outputs[0] = output->id();
    Status s = XRRSetCrtcConfig(XRandR::display(), XRandR::screenResources(), XRandR::freeCrtc(output->id()),
        CurrentTime, output->pos().rx(), output->pos().ry(), output->currentMode(),
        output->rotation(), outputs, 1);

    qDebug() << "XRRSetCrtcConfig() returned" << s;
    return (s == RRSetConfigSuccess);
}

bool XRandRConfig::changeOutput(Output* output, int crtcId) const
{
    qDebug() << "Updating: " << output->id();

    RROutput *outputs = new RROutput[1];
    outputs[0] = output->id();
    Status s = XRRSetCrtcConfig(XRandR::display(), XRandR::screenResources(), crtcId,
        CurrentTime, output->pos().rx(), output->pos().ry(), output->currentMode(),
        output->rotation(), outputs, 1);

    qDebug() << "XRRSetCrtcConfig() returned" << s;
    return (s == RRSetConfigSuccess);
}


#include "xrandrconfig.moc"
