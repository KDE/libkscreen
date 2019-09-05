/*************************************************************************************
 *  Copyright 2012, 2013  Daniel Vrátil <dvratil@redhat.com>                         *
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
#pragma once

#include <QObject>

#include "xrandr.h"
#include "xrandrcrtc.h"
#include "xrandroutput.h"

class XRandRScreen;
namespace KScreen {
class Config;
}

class XRandRConfig : public QObject
{
    Q_OBJECT

public:
    XRandRConfig();
    ~XRandRConfig() override;

    XRandROutput::Map outputs() const;
    XRandROutput *output(xcb_randr_output_t output) const;

    XRandRCrtc::Map crtcs() const;
    XRandRCrtc *crtc(xcb_randr_crtc_t crtc) const;

    XRandRScreen *screen() const;

    void addNewOutput(xcb_randr_output_t id);
    void addNewCrtc(xcb_randr_crtc_t crtc);
    void removeOutput(xcb_randr_output_t id);

    KScreen::ConfigPtr toKScreenConfig() const;
    void applyKScreenConfig(const KScreen::ConfigPtr &config);

private:
    QSize screenSize(const KScreen::ConfigPtr &config) const;
    bool setScreenSize(const QSize &size) const;

    void setPrimaryOutput(xcb_randr_output_t outputId) const;

    bool disableOutput(const KScreen::OutputPtr &output) const;
    bool enableOutput(const KScreen::OutputPtr &output) const;
    bool changeOutput(const KScreen::OutputPtr &output) const;
    bool replicateOutput(const KScreen::OutputPtr &output) const;

    bool sendConfig(const KScreen::OutputPtr &kscreenOutput, XRandRCrtc *crtc) const;

    /**
     * We need to print stuff to discover the damn bug
     * where currentMode is null
     */
    void printConfig(const KScreen::ConfigPtr &config) const;
    void printInternalCond() const;

    XRandROutput::Map m_outputs;
    XRandRCrtc::Map m_crtcs;
    XRandRScreen *m_screen;
};
