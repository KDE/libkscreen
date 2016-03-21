/*************************************************************************************
 *  Copyright 2015 Sebastian Kügler <sebas@kde.org>                                  *
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

#ifndef KSCREEN_DPMSCLIENT_H
#define KSCREEN_DPMSCLIENT_H

#include <QCommandLineParser>
#include <QObject>
#include "../config.h"

#include <KWayland/Client/registry.h>

class QThread;

namespace KWayland
{
    namespace Client {
        class ConnectionThread;
    }
}

namespace KScreen
{
class ConfigOperation;
//static const QString s_socketName = QStringLiteral("libkscreen-test-wayland-backend-0");

class DpmsClient : public QObject
{
    Q_OBJECT

public:
    explicit DpmsClient(QObject *parent = 0);
    virtual ~DpmsClient();

    void connect();

    void setTimeout(int msec);

    void off();
    void on();

    void show();

Q_SIGNALS:
    void ready();

private Q_SLOTS:
    void connected();

private:
    QThread *m_thread;
    KWayland::Client::ConnectionThread *m_connection;
    KWayland::Client::Registry m_registry;
    bool m_setOff = true;
    bool m_setOn = false;
};

} // namespace

#endif // KSCREEN_WAYLAND_SCREEN_H
