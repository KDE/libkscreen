/*************************************************************************************
 *  Copyright 2014 Sebastian Kügler <sebas@kde.org>                                  *
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

#ifndef QSCREEN_CONFIG_H
#define QSCREEN_CONFIG_H

#include "../abstractbackend.h"
#include "config.h"

#include <QDir>
#include <QScreen>
#include <QtCore/QSize>
#include <QLoggingCategory>
#include <QSocketNotifier>

// wayland
#include <wayland-client.h>

class wl_output;

namespace KScreen
{
class Output;
class WaylandOutput;
//class QScreenScreen;

class WaylandConfig : public QObject
{
    Q_OBJECT

public:
    explicit WaylandConfig(QObject *parent = 0);
    virtual ~WaylandConfig();

    KScreen::Config *toKScreenConfig() const;
    void updateKScreenConfig(KScreen::Config *config) const;

    QMap<int, WaylandOutput *> outputMap() const;
    int outputId(const QScreen *qscreen);

    void addOutput(wl_output *o);


    wl_display *display() const;

private Q_SLOTS:
    void screenAdded(const QScreen *qscreen);
    void screenDestroyed(QObject *qscreen = 0);
    void readEvents();


private:
    void initConnection();
    wl_display *m_display;
    wl_registry *m_registry;
    QString m_socketName;
    QDir m_runtimeDir;

    QMap<wl_output*, WaylandOutput *> m_outputMap;
    //QScreenScreen *m_screen;
    int m_lastOutputId = -1;
    bool m_blockSignals;
};

} // namespace

#endif // QSCREEN_CONFIG_H
