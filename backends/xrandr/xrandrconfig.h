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
    void addNewOutput(const RROutput id);

    KScreen::ConfigPtr toKScreenConfig() const;
    void updateKScreenConfig(KScreen::ConfigPtr &config) const;
    void applyKScreenConfig(const KScreen::ConfigPtr &config);

    int m_primaryOutput;
private:
    /**
     * We need to print stuff to discover the damn bug
     * where currentMode is null
     */
    void printConfig(const KScreen::ConfigPtr &config) const;
    void printInternalCond() const;
    QSize screenSize(const KScreen::ConfigPtr &config) const;
    bool setScreenSize(const QSize& size) const;
    void setPrimaryOutput(int outputId) const;
    bool disableOutput(const KScreen::OutputPtr &output) const;
    bool enableOutput(const KScreen::OutputPtr &output) const;
    bool changeOutput(const KScreen::OutputPtr &output, int crtcId) const;
    XRandROutput* createNewOutput(RROutput id, bool primary);
    XRandROutput::Map m_outputs;
    XRandRScreen *m_screen;

Q_SIGNALS:
    void outputRemoved(int id);

private Q_SLOTS:
    void outputRemovedSlot(int id);
};

#endif // XRANDRCONFIG_H
