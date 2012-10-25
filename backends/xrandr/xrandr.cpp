/*************************************************************************************
 *  Copyright (C) 2012 by Alejandro Fiestas Olivares <afiestas@kde.org>              *
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

#include "xrandr.h"
#include "config.h"

#include <QtCore/QDebug>
#include <QtCore/QFile>
#include <QtCore/qplugin.h>
#include <QtCore/QRect>

#include <QtGui/QX11Info>

Q_EXPORT_PLUGIN2(XRandR, XRandR)

XRandR::XRandR(QObject* parent): QObject(parent)
{
    m_display = QX11Info::display();
    m_screen = DefaultScreen(m_display);
    m_rootWindow = XRootWindow(m_display, m_screen);

    qDebug() << "XRandR Cto";
    qDebug() << "\t" << "Display: " << m_display;
    qDebug() << "\t" << "Screen: " << m_screen;
    qDebug() << "\t" << "RootWindow: " << m_rootWindow;
}

XRandR::~XRandR()
{

}

QString XRandR::name() const
{
    return QString("XRandR");
}

Config* XRandR::config() const
{
    qDebug() << "XRandR::Config";
    QSize min, max;
    XRRGetScreenSizeRange (m_display, m_rootWindow, &min.rwidth(), &min.rheight(),
                           &max.rwidth(), &max.rheight());

    CScreen* screen = new CScreen(m_screen);
    screen->setMaxSize(max);
    screen->setMinSize(min);
    screen->setCurrentSize(QSize(DisplayWidth(m_display, m_screen),DisplayHeight(m_display, m_screen)));

    XRRScreenResources* resources = screenResources();

    RROutput id, primary;
    XRROutputInfo* outputInfo;
    primary = XRRGetOutputPrimary(m_display, m_rootWindow);

    qDebug() << "\t" << "Primary: " << primary;

    OutputList outputList;
    for (int i = 0; i < resources->noutput; ++i)
    {
        id = resources->outputs[i];
        outputInfo = XRROutput(id);

        Output *output = new Output(id);
        output->setName(outputInfo->name);
        output->setConnected(outputInfo->connection == RR_Connected);
        output->setEnabled(outputInfo->crtc != None);
        output->setPrimary(id == primary);
        output->setType("unknown");

        if (outputInfo->crtc) {
            XRRCrtcInfo* crtcInfo = XRRCrtc(outputInfo->crtc);
            QRect rect;
            rect.setRect(crtcInfo->x, crtcInfo->y, crtcInfo->width, crtcInfo->height);
            output->setPos(rect.topLeft());

            if (crtcInfo->mode) {
                output->setCurrentMode(crtcInfo->mode);
            }
        }

        ModeList modeList;
        XRRModeInfo* modeInfo;
        for (int i = 0; i < outputInfo->nmode; ++i)
        {
            modeInfo = &resources->modes[i];
            Mode *mode = new Mode(modeInfo->id);
            mode->setName(QString::fromUtf8(modeInfo->name));
            mode->setSize(QSize(modeInfo->width, modeInfo->height));
            mode->setRefreshRate(((float) modeInfo->dotClock / ((float) modeInfo->hTotal * (float) modeInfo->vTotal)));

            modeList.insert(mode->id(), mode);
        }

        XRRFreeOutputInfo(outputInfo);

        output->setModes(modeList);
        outputList.insert(output->id(), output);
    }

    XRRFreeScreenResources(resources);

    Config *config = new Config();
    config->setScreen(screen);
    config->setOutputs(outputList);

    return config;
}

void XRandR::setConfig(Config* config) const
{
    Config *currentConfig = this->config();
    OutputList outputs = config->outputs();
    QSize newSize = screenSize(config);

    int neededCrtc = 0;
    int primaryOutput = 0;
    Output *cOutput = 0;
    OutputList toDisable, toEnable, toChange;
    QHash<int, int> currentCrtc;
    Q_FOREACH(Output *output, outputs) {
        cOutput = currentConfig->output(output->id());

        if (output->isPrimary()) {
            primaryOutput = cOutput->id();
        }

        if (!output->isEnabled() && cOutput->isEnabled()) {
            toDisable.insert(output->id(), output);
            continue;
        } else if(output->isEnabled() && !cOutput->isEnabled()) {
            toEnable.insert(output->id(), output);
            neededCrtc ++;
            continue;
        } else if (!output->isEnabled() && !cOutput->isEnabled()) {
            continue;
        }

        neededCrtc ++;

        if (output->currentMode() != cOutput->currentMode()) {
            if (!toChange.contains(output->id())) {
                currentCrtc.insert(output->id(), outputCrtc(output->id()));
                toChange.insert(output->id(), output);
                if (!toDisable.contains(output->id())) {
                    toDisable.insert(output->id(), output);
                }
            }
        }

        if (output->pos() != cOutput->pos()) {
            if (!toChange.contains(output->id())) {
                currentCrtc.insert(output->id(), outputCrtc(output->id()));
                toChange.insert(output->id(), output);
                if (!toDisable.contains(output->id())) {
                    toDisable.insert(output->id(), output);
                }
            }
        }

        if (output->rotation() != cOutput->rotation()) {
            if( !toChange.contains(output->id())) {
                currentCrtc.insert(output->id(), outputCrtc(output->id()));
                toChange.insert(output->id(), output);
            }
        }
    }

//     if (newSize.width() > screen->maxSize().width() ||
//         newSize.height() > screen->maxSize().height()) {
//         qDebug() << "The new size is too big: " << newSize << " - " << screen->maxSize();
//         return;//Too big
//     }

//     qDebug() << neededCrtc;
//     if (neededCrtc > screen->crtc().count()) {
//         qDebug() << "We need more crtc than we have: " << neededCrtc << " - " << screen->crtc().count();
//         return;//We don't have enough crtc
//     }

    if (primaryOutput) {
        setPrimaryOutput(primaryOutput);
    }

    Q_FOREACH(Output* output, toDisable) {
        disableOutput(output);
    }

    setScreenSize(newSize);

    Q_FOREACH(Output* output, toEnable) {
        enableOutput(output);
    }

    Q_FOREACH(Output* output, toChange) {
        changeOutput(output, currentCrtc[output->id()]);
    }
}

QSize XRandR::screenSize(Config* config) const
{
    int cord;
    QSize size, outputSize;
    OutputList outputs = config->outputs();
    Q_FOREACH(const Output* output, outputs) {
        if (!output->isEnabled()) {
            continue;
        }

        outputSize = output->mode(output->currentMode())->size();
        cord = output->pos().y() + outputSize.height();
        if (cord > size.height()) {
            size.setHeight(cord);
        }

        cord = output->pos().x() + outputSize.width();
        if (cord > size.width()) {
            size.setWidth(cord);
        }
    }

    return size;
}

bool XRandR::setScreenSize(const QSize& size) const
{
    double dpi;
    int widthMM, heightMM;
    dpi = (25.4 * DisplayHeight(m_display, m_screen)) / DisplayHeightMM(m_display, m_screen);

    qDebug() << "DPI: " << dpi;
    qDebug() << "Size: " << size;

    widthMM =  ((25.4 * size.width()) / dpi);
    heightMM = ((25.4 * size.height()) / dpi);

    qDebug() << "MM: " << widthMM << "x" << heightMM;

    qDebug() << size << " " << widthMM << "x" << heightMM;
    XRRSetScreenSize(m_display, m_rootWindow, size.width(), size.height(), widthMM, heightMM);

    return true;
}

void XRandR::setPrimaryOutput(int outputId) const
{
    XRRSetOutputPrimary(m_display, m_rootWindow, outputId);
}

void XRandR::disableOutput(Output* output) const
{
    qDebug() << "Disabling: " << output->id();
    int crtcId = outputCrtc(output->id());
    qDebug() << crtcId;
    XRRSetCrtcConfig (QX11Info::display(), screenResources(), crtcId, CurrentTime,
                 0, 0, None, RR_Rotate_0, NULL, 0);
}

void XRandR::enableOutput(Output* output) const
{
    qDebug() << "Enabling: " << output->id();
    RROutput *outputs = new RROutput[1];
    outputs[0] = output->id();
    Status s = XRRSetCrtcConfig(m_display, screenResources(), freeCrtc(),
        CurrentTime, output->pos().rx(), output->pos().ry(), output->currentMode(),
        output->rotation(), outputs, 1);

    Q_UNUSED(s);
}

void XRandR::changeOutput(Output* output, int crtcId) const
{
    qDebug() << "Updating: " << output->id();

    RROutput *outputs = new RROutput[1];
    outputs[0] = output->id();
    Status s = XRRSetCrtcConfig(m_display, screenResources(), crtcId,
        CurrentTime, output->pos().rx(), output->pos().ry(), output->currentMode(),
        output->rotation(), outputs, 1);

    Q_UNUSED(s);
}

bool XRandR::isValid() const
{
    return true;
}

RRCrtc XRandR::outputCrtc(int outputId) const
{
    RRCrtc crtcId;
    XRROutputInfo* outputInfo = XRROutput(outputId);

    crtcId = outputInfo->crtc;
    XRRFreeOutputInfo(outputInfo);

    return crtcId;
}

RRCrtc XRandR::freeCrtc() const
{
    XRRScreenResources* resources = screenResources();

    XRRCrtcInfo *crtc;
    for (int i = 0; i < resources->ncrtc; ++i)
    {
       RRCrtc crtcId = resources->crtcs[i];
       crtc = XRRCrtc(crtcId);
       if (!crtc->noutput) {
           qDebug() << "Returning: " << crtcId;
           XRRFreeCrtcInfo(crtc);
           return crtcId;
       }
       XRRFreeCrtcInfo(crtc);
    }

    qDebug() << "Returning: " << "ZERO";
    return 0;
}

XRRScreenResources* XRandR::screenResources() const
{
    return XRRGetScreenResources(m_display, m_rootWindow);
}

XRROutputInfo* XRandR::XRROutput(int outputId) const
{
    return XRRGetOutputInfo(m_display, screenResources(), outputId);
}

XRRCrtcInfo* XRandR::XRRCrtc(int crtcId) const
{
    return XRRGetCrtcInfo(m_display, screenResources(), crtcId);
}

#include "xrandr.moc"
