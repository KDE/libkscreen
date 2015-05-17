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

#include "../tests/waylandtestserver.h"

Q_LOGGING_CATEGORY(KSCREEN_QSCREEN, "kscreen.wayland");

//static const QString s_socketName = QStringLiteral("libkscreen-test-wayland-backend-0");

static const QString s_outputConfig = QStringLiteral("waylandconfigtestrc");
using namespace KScreen;

class testWaylandWrite : public QObject
{
    Q_OBJECT

public:
    explicit testWaylandWrite(QObject *parent = nullptr);

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();

    void writeConfig();
    void changeConfig();

private:

    ConfigPtr m_config;
    WaylandTestServer *m_server;

};

testWaylandWrite::testWaylandWrite(QObject *parent)
    : QObject(parent)
    , m_config(nullptr)
{
    m_server = new WaylandTestServer(this);
}

void testWaylandWrite::initTestCase()
{
    setenv("KSCREEN_BACKEND", "wayland", 1);
    KScreen::BackendManager::instance()->shutdownBackend();

    // This is how KWayland will pick up the right socket,
    // and thus connect to our internal test server.
    setenv("WAYLAND_DISPLAY", s_socketName.toLocal8Bit(), 1);

    m_server->start();
}

void testWaylandWrite::cleanupTestCase()
{
    KScreen::BackendManager::instance()->shutdownBackend();
}

void testWaylandWrite::writeConfig()
{
    GetConfigOperation *op = new GetConfigOperation();
    op->exec();
    m_config = op->config();

    QVERIFY(m_config);

    //QVERIFY(WaylandConfigWriter::writeJson(m_config, "waylandconfigfile.json"));
    QVERIFY(WaylandConfigWriter::writeConfig(m_config, s_outputConfig));

    auto cfg = KSharedConfig::openConfig(s_outputConfig, KConfig::SimpleConfig);

    //qDebug() << "groups" << cfg->groupList();
    QVERIFY(cfg->groupList().count() == 2);

    auto o1group = cfg->group("Octopus Graphics-rubyled");
    QCOMPARE(o1group.readEntry("x", -2), 1920);
    QCOMPARE(o1group.readEntry("y", -2), 0);
    QCOMPARE(o1group.readEntry("width", -2), 1680);
    QCOMPARE(o1group.readEntry("height", -2), 1050);
    QCOMPARE(o1group.readEntry("refreshRate", -2), 60);

    auto o2group = cfg->group("Bonsai-marina");
    QCOMPARE(o2group.readEntry("x", -2), 0);
    QCOMPARE(o2group.readEntry("y", -2), 0);
    QCOMPARE(o2group.readEntry("width", -2), 1920);
    QCOMPARE(o2group.readEntry("height", -2), 1080);
    QCOMPARE(o2group.readEntry("refreshRate", -2), 60);
}

void testWaylandWrite::changeConfig()
{
    auto outputs = m_config->outputs();

    /* this is mode 78 from default.json
     *                    "id": "870",
     *                    "name": "1440x900",
     *                    "refreshRate": 74.984428405761719,
     */
    for (auto o: outputs) {
        //qDebug() << "o" << o->id();
        for (auto m: o->modes()) {
            //qDebug() << "   m" << m->id();
        }
    }

    KScreen::OutputPtr o1 = outputs.first();
    QCOMPARE(o1->id(), 5);
    qDebug() << "setCurrentModeId" << o1->currentModeId();
    o1->setCurrentModeId("800x600@0");

    QSignalSpy syncSpy(m_server, SIGNAL(outputsChanged()));
    QVERIFY(WaylandConfigWriter::writeConfig(m_config, s_outputConfig));

    auto cfg = KSharedConfig::openConfig(s_outputConfig, KConfig::SimpleConfig);
    cfg->reparseConfiguration();
    auto o1group = cfg->group("Octopus Graphics-rubyled");
    QCOMPARE(o1group.readEntry("x", -2), 1920);
    QCOMPARE(o1group.readEntry("y", -2), 0);
    QCOMPARE(o1group.readEntry("width", -2), 800);
    QCOMPARE(o1group.readEntry("height", -2), 600);
    QCOMPARE(o1group.readEntry("refreshRate", -2), 75);
    m_server->pickupConfigFile(s_outputConfig);
    syncSpy.wait(10);
    QCOMPARE(syncSpy.count(), 1);

}



QTEST_GUILESS_MAIN(testWaylandWrite)

#include "testwlwrite.moc"
