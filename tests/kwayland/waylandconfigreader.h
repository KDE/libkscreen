/*************************************************************************************
 *  Copyright 2014-2015 Sebastian Kügler <sebas@kde.org>                             *
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

#ifndef KSCREEN_WAYLAND_CONFIGREADER_H
#define KSCREEN_WAYLAND_CONFIGREADER_H

#include <QObject>
#include <QRect>

// KWayland
#include <KWayland/Server/display.h>
#include <KWayland/Server/output_interface.h>
#include <KWayland/Server/outputdevice_interface.h>

namespace KScreen
{

using namespace KWayland::Server;

class WaylandConfigReader
{

public:
    //static QList<KWayland::Server::OutputInterface*> outputsFromConfig(const QString &configfile, KWayland::Server::Display *display);
    static void outputsFromConfig(const QString &configfile, KWayland::Server::Display *display, QList<KWayland::Server::OutputDeviceInterface*>& outputs);
    static OutputDeviceInterface* createOutputDevice(const QVariantMap &outputConfig, KWayland::Server::Display *display);
    static QList<KWayland::Server::OutputInterface*> createOutputs(KWayland::Server::Display *display, QList<KWayland::Server::OutputDeviceInterface*>& outputdevices);

    static QSize sizeFromJson(const QVariant &data);
    static QRect rectFromJson(const QVariant &data);
    static QPoint pointFromJson(const QVariant &data);
};

} // namespace

#endif // KSCREEN_WAYLAND_CONFIGREADER_H
