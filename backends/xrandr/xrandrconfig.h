/*************************************************************************************
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

#ifndef XRANDRCONFIG_H
#define XRANDRCONFIG_H

#include <QObject>

#include "xrandr.h"
#include "xrandroutput.h"

class XRandRScreen;
namespace KScreen {
class Config;
}

class XRandRConfig : public QObject
{
    Q_OBJECT

public:
    explicit XRandRConfig();
    virtual ~XRandRConfig();

    void update();

    XRandROutput::Map outputs() const;

    KScreen::Config *toKScreenConfig() const;
    void updateKScreenConfig(KScreen::Config *config) const;
    void applyKScreenConfig(KScreen::Config *config);

private:
    QSize screenSize(KScreen::Config* config) const;
    bool setScreenSize(const QSize& size) const;
    void setPrimaryOutput(int outputId) const;
    bool disableOutput(KScreen::Output* output) const;
    bool enableOutput(KScreen::Output* output) const;
    bool changeOutput(KScreen::Output* output, int crtcId) const;

    XRandROutput::Map m_outputs;
    XRandRScreen *m_screen;
};

#endif // XRANDRCONFIG_H
