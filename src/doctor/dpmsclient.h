/*************************************************************************************
 *  Copyright 2016 Sebastian Kügler <sebas@kde.org>                                  *
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
#include <KWayland/Client/dpms.h>

class QThread;

namespace KWayland
{
    namespace Client {
        class ConnectionThread;
    }
}

namespace KScreen
{

class DpmsClient : public QObject
{
    Q_OBJECT

public:
    explicit DpmsClient(QObject *parent = nullptr);
    ~DpmsClient() override;

    void connect();
    void off();
    void on();

Q_SIGNALS:
    void ready();
    void finished();

private Q_SLOTS:
    void connected();
    void modeChanged();

private:
    void changeMode(KWayland::Client::Dpms::Mode mode);
    QThread *m_thread;
    KWayland::Client::ConnectionThread *m_connection = nullptr;
    KWayland::Client::DpmsManager *m_dpmsManager = nullptr;
    KWayland::Client::Registry *m_registry = nullptr;
    bool m_setOff = true;
    bool m_setOn = false;

    bool m_supportedOututCount = 0;
    int m_modeChanges = 0;
};

} // namespace

#endif // KSCREEN_DPSMCLIENT_H
