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

// KWayland
#include <KWayland/Client/connection_thread.h>
#include <KWayland/Client/event_queue.h>
#include <KWayland/Client/registry.h>

#include <KWayland/Server/compositor_interface.h>
#include <KWayland/Server/display.h>
#include <KWayland/Server/output_interface.h>
#include <KWayland/Server/seat_interface.h>
#include <KWayland/Server/shell_interface.h>

#include "waylandconfigwriter.h"
#include "waylandconfigreader.h"

#include "../src/backendmanager_p.h"
#include "../src/getconfigoperation.h"
#include "../src/config.h"
#include "../src/configmonitor.h"
#include "../src/output.h"
#include "../src/mode.h"
#include "../src/edid.h"

Q_LOGGING_CATEGORY(KSCREEN_QSCREEN, "kscreen.wayland");

static const QString s_socketName = QStringLiteral("libkscreen-test-wayland-backend-0");

using namespace KScreen;

class testWaylandSetup : public QObject
{
    Q_OBJECT

public:
    explicit testWaylandSetup(QObject *parent = nullptr);

private Q_SLOTS:
    void initTestCase();

    void testConfigs_data();
    void testConfigs();

    void cleanupTestCase();

private:
    void loadConfig();
    void writeConfig();
    void startWaylandServer(const QString& configfile = QString());
    void stopWaylandServer();
    QProcess m_process;
    ConfigPtr m_config;
    QString m_backend;

    bool m_startServer;
    KWayland::Server::Display *m_display;
    KWayland::Server::CompositorInterface *m_compositor;
    QList<KWayland::Server::OutputInterface*> m_outputs;
    KWayland::Server::SeatInterface *m_seat;
    KWayland::Server::ShellInterface *m_shell;
};

testWaylandSetup::testWaylandSetup(QObject *parent)
    : QObject(parent)
    , m_config(nullptr)
    , m_startServer(true)
    , m_display(nullptr)
    , m_compositor(nullptr)
    , m_seat(nullptr)
    , m_shell(nullptr)
{
}

void testWaylandSetup::initTestCase()
{
    setenv("KSCREEN_BACKEND", "wayland", 1);
    KScreen::BackendManager::instance()->shutdownBackend();
    m_backend = qgetenv("KSCREEN_BACKEND").constData();
    m_startServer = QString::fromLocal8Bit(qgetenv("KSCREEN_EXTERNAL_WAYLAND_SERVER").constData()).isEmpty();

    // This is how KWayland will pick up the right socket,
    // and thus connect to our internal test server.
    setenv("WAYLAND_DISPLAY", s_socketName.toLocal8Bit(), 1);

    startWaylandServer();

    GetConfigOperation *op = new GetConfigOperation();
    op->exec();
    m_config = op->config();
}


void testWaylandSetup::startWaylandServer(const QString &configfile)
{
    if (!m_startServer) {
        return;
    }
    using namespace KWayland::Server;
    m_display = new KWayland::Server::Display(this);
    m_display->setSocketName(s_socketName);
    m_display->start();

    // Enable once we actually use these things...
    //m_display->createShm();
    //m_compositor = m_display->createCompositor();
    //m_compositor->create();
    //m_seat = m_display->createSeat();
    //m_seat->create();
    //m_shell = m_display->createShell();
    //m_shell->create();
    QString cfg = configfile;
    if (configfile.isEmpty()) {
        cfg = TEST_DATA + QStringLiteral("default.json");
    }
    qDebug() << "CONFIG: " << cfg;

    //configfile.append("/multipleoutput.json");
    m_outputs = KScreen::WaylandConfigReader::outputsFromConfig(cfg, m_display);

    qDebug() << "Wayland server running. Outputs: " << m_outputs.count();
    QTimer t;
    t.setSingleShot(true);
    t.setInterval(500);

    QSignalSpy s(&t, SIGNAL(timeout()));
    t.start();
    s.wait(3000);
    //QVERIFY(m_display->isRunning());

}

void testWaylandSetup::loadConfig()
{
    return;
    GetConfigOperation *op = new GetConfigOperation();
    op->exec();
    m_config = op->config();
    QVERIFY(m_config->isValid());
}

void testWaylandSetup::stopWaylandServer()
{
    if (!m_startServer) {
        return;
    }

    // actually stop the Wayland server
    delete m_display;
    m_display = nullptr;

    QTimer t;
    t.setSingleShot(true);
    t.setInterval(500);

    QSignalSpy s(&t, SIGNAL(timeout()));
    t.start();
    s.wait(3000);

}

void testWaylandSetup::cleanupTestCase()
{
    if (m_startServer) {
        //QCOMPARE(m_config->outputs().count(), 0);
    }
    KScreen::BackendManager::instance()->shutdownBackend();
    stopWaylandServer();
}

void testWaylandSetup::writeConfig()
{
    QVERIFY(WaylandConfigWriter::write(m_config, "waylandconfigfile.json"));

}

void testWaylandSetup::testConfigs_data()
{
    QTest::addColumn<QString>("configfile");

    QTest::newRow("default") << "default.json";
    QTest::newRow("single") << "singleoutput.json";
    QTest::newRow("multiple") << "multipleoutput.json";
}

void testWaylandSetup::testConfigs()
{
    QFETCH(QString, configfile);
    QString cfg = TEST_DATA + configfile;
    qDebug() << "Config file: " << cfg;
    startWaylandServer(cfg);
    //     return;


    if (m_config) {
        //m_config->deleteLater();
//         m_config(nullptr);
    }

    GetConfigOperation *op = new GetConfigOperation();
    op->exec();
    m_config = op->config();

    QVERIFY(m_config);
    // Has outputs?
    qDebug() << "Outputs for " << configfile << " " << m_config->outputs().count();
    QVERIFY(m_config->outputs().count());
    stopWaylandServer();
}


QTEST_GUILESS_MAIN(testWaylandSetup)

#include "testwlsetup.moc"
