/*************************************************************************************
 *  Copyright 2015 by Sebastian Kügler <sebas@kde.org>                           *
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

#include <QCoreApplication>
#include <QtTest>
#include <QObject>
#include <QSignalSpy>

#include <KSharedConfig>
#include <KConfigGroup>

// KWayland
#include <KWayland/Client/connection_thread.h>
#include <KWayland/Client/event_queue.h>
#include <KWayland/Client/registry.h>

#include <KWayland/Server/display.h>
#include <KWayland/Server/output_interface.h>

#include "waylandconfigwriter.h"
#include "waylandconfigreader.h"

#include "../src/backendmanager_p.h"
#include "../src/getconfigoperation.h"
#include "../src/setconfigoperation.h"
#include "../src/config.h"
#include "../src/configmonitor.h"
#include "../src/output.h"
#include "../src/mode.h"
#include "../src/edid.h"

#include "../tests/waylandtestserver.h"

Q_LOGGING_CATEGORY(KSCREEN_WAYLAND, "kscreen.wayland");

//static const QString s_socketName = QStringLiteral("libkscreen-test-wayland-backend-0");

using namespace KScreen;

class TestKWaylandConfig : public QObject
{
    Q_OBJECT

public:
    explicit TestKWaylandConfig(QObject *parent = nullptr);

private Q_SLOTS:
    void init();
    void cleanup();

    void changeConfig();

private:

    WaylandTestServer *m_server;

};

TestKWaylandConfig::TestKWaylandConfig(QObject *parent)
    : QObject(parent)
    , m_server(nullptr)
{
}

void TestKWaylandConfig::init()
{
    setenv("KSCREEN_BACKEND", "kwayland", 1);
    KScreen::BackendManager::instance()->shutdownBackend();

    // This is how KWayland will pick up the right socket,
    // and thus connect to our internal test server.
    setenv("WAYLAND_DISPLAY", s_socketName.toLocal8Bit(), 1);

    m_server = new WaylandTestServer(this);
    m_server->start();
}

void TestKWaylandConfig::cleanup()
{
    qDebug() << "Shutting down";
    KScreen::BackendManager::instance()->shutdownBackend();
    delete m_server;
}

void TestKWaylandConfig::changeConfig()
{
    auto op = new GetConfigOperation();
    QVERIFY(op->exec());
    auto config = op->config();
    QVERIFY(config);

    // Prepare monitor & spy
    KScreen::ConfigMonitor *monitor = KScreen::ConfigMonitor::instance();
    monitor->addConfig(config);
    QSignalSpy configSpy(monitor, &KScreen::ConfigMonitor::configurationChanged);


    // The first output is currently disabled, let's try to enable it
    auto output = config->outputs().first();
    qDebug() << "Changing output: " << output << "enabled?" << output->isEnabled();
    output->setEnabled(true);

    QSignalSpy serverSpy(m_server, &WaylandTestServer::configChanged);
    auto sop = new SetConfigOperation(config, this);
    sop->exec(); // fire and forget...

    QVERIFY(configSpy.wait(2000));
    // check if the server changed
    QCOMPARE(serverSpy.count(), 1);

    QCOMPARE(configSpy.count(), 1);


}


QTEST_GUILESS_MAIN(TestKWaylandConfig)

#include "testkwaylandconfig.moc"
