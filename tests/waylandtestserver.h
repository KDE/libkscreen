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

#ifndef KSCREEN_WAYLAND_TESTSERVER_H
#define KSCREEN_WAYLAND_TESTSERVER_H

#include <QObject>

// KWayland
#include <KWayland/Server/compositor_interface.h>
#include <KWayland/Server/display.h>
#include <KWayland/Server/outputdevice_interface.h>
#include <KWayland/Server/outputconfiguration_interface.h>
#include <KWayland/Server/seat_interface.h>
#include <KWayland/Server/shell_interface.h>
#include <KWayland/Server/outputmanagement_interface.h>

// KConfigCore
#include <KConfigGroup>

class KDirWatch;

namespace KScreen
{
class WaylandConfig;
class WaylandOutput;

static const QString s_socketName = QStringLiteral("libkscreen-test-wayland-backend-0");

using namespace KWayland::Server;

class WaylandTestServer : public QObject
{
    Q_OBJECT

public:
    explicit WaylandTestServer(QObject *parent = 0);
    virtual ~WaylandTestServer();

    void setConfig(const QString &configfile);
    void start();
    void stop();
    void pickupConfigFile(const QString &configfile);

    KWayland::Server::Display* display();
    QList<KWayland::Server::OutputDeviceInterface*> outputs() const;

    int outputCount() const;

Q_SIGNALS:
    void outputsChanged();

    void started();

    void configChanged();

private Q_SLOTS:
    void configurationChangeRequested(KWayland::Server::OutputConfigurationInterface *configurationInterface);

private:
    bool outputFromConfigGroup(const KConfigGroup& config, KWayland::Server::OutputDeviceInterface* output);

    QString m_configFile;
    KWayland::Server::Display *m_display;
    QList<KWayland::Server::OutputDeviceInterface*> m_outputs;
    KWayland::Server::OutputManagementInterface *m_outputManagement;
};

} // namespace

#endif // KSCREEN_WAYLAND_SCREEN_H
