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
    void init();
    void cleanup();

    void testConfigs_data();
    void testConfigs();

private:
    void writeConfig();
    void changeConfig();
    void addOutput();
    void changeOutput();

    void removeOutput();

private:

    ConfigPtr m_config;
    WaylandTestServer *m_server;

};

testWaylandWrite::testWaylandWrite(QObject *parent)
    : QObject(parent)
    , m_config(nullptr)
{
}

void testWaylandWrite::init()
{
    setenv("KSCREEN_BACKEND", "kwayland", 1);
    KScreen::BackendManager::instance()->shutdownBackend();

    // This is how KWayland will pick up the right socket,
    // and thus connect to our internal test server.
    setenv("WAYLAND_DISPLAY", s_socketName.toLocal8Bit(), 1);

    m_server = new WaylandTestServer(this);
    //m_server->start();
}

void testWaylandWrite::cleanup()
{
    KScreen::BackendManager::instance()->shutdownBackend();
    delete m_server;
}

void testWaylandWrite::testConfigs_data()
{
    QTest::addColumn<QString>("configfile");

    QTest::newRow("default.json") << "default.json";
    //QTest::newRow("multipleoutput.json") << "multipleoutput.json";
//     QTest::newRow("multipleclone.json") << "multipleclone.json";
//     QTest::newRow("singleoutput.json") << "singleoutput.json";
//     QTest::newRow("singleoutputBroken.json") << "singleoutputBroken.json";
//     QTest::newRow("singleOutputWithoutPreferred.json") << "singleOutputWithoutPreferred.json";
//     QTest::newRow("tooManyOutputs.json") << "tooManyOutputs.json";
}


void testWaylandWrite::testConfigs()
{
    QFETCH(QString, configfile);
    QString cfg = TEST_DATA + configfile;
    //qDebug() << "Config file: " << cfg;

    m_server->setConfig(cfg);
    m_server->start();

    GetConfigOperation *op = new GetConfigOperation();
    op->exec();
    m_config = op->config();

    QVERIFY(m_config);

    QVERIFY(m_config->outputs().count());
    QList<int> ids;
    foreach (auto output, m_config->outputs()) {
        //         qDebug() << " _____________________ Output: " << output;
        //         qDebug() << "   output name: " << output->name();
        //         qDebug() << "   output modes: " << output->modes().count() << output->modes();
        //         qDebug() << "   output enabled: " << output->isEnabled();
        //         qDebug() << "   output connect: " << output->isConnected();
        //         qDebug() << "   output sizeMm : " << output->sizeMm();
        QVERIFY(!output->name().isEmpty());
        QVERIFY(output->id() > -1);
        QVERIFY(output->isConnected());
        if (!output->isEnabled()) {
            continue;
        }
        //QVERIFY(output->isEnabled());
        QVERIFY(output->geometry() != QRectF(1,1,1,1));
        QVERIFY(output->geometry() != QRectF());
        if (configfile.endsWith("default.json")) {
            //qDebug() << "Output MM" << output->name() << output->sizeMm();
            QVERIFY(output->sizeMm() != QSize());
        }
        QVERIFY(output->edid() != 0);
        //QCOMPARE(output->rotation(), Output::None);
        QVERIFY(!ids.contains(output->id()));
        ids << output->id();
    }
    QCOMPARE(m_config->outputs().count(), m_server->outputCount());

}

void testWaylandWrite::writeConfig()
{
    m_server->start();
    GetConfigOperation *op = new GetConfigOperation();
    op->exec();
    m_config = op->config();

    QVERIFY(m_config);

}

void testWaylandWrite::changeConfig()
{
    m_server->start();
    GetConfigOperation *op = new GetConfigOperation();
    op->exec();
    m_config = op->config();

    QVERIFY(m_config);

}

void testWaylandWrite::removeOutput()
{
    m_server->start();
    GetConfigOperation *op = new GetConfigOperation();
    op->exec();
    m_config = op->config();

    QVERIFY(m_config);

}

void testWaylandWrite::addOutput()
{
    m_server->start();
    GetConfigOperation *op = new GetConfigOperation();
    op->exec();
    m_config = op->config();

    QVERIFY(m_config);

}

void testWaylandWrite::changeOutput()
{
    m_server->start();
    GetConfigOperation *op = new GetConfigOperation();
    op->exec();
    m_config = op->config();

    QVERIFY(m_config);

}



QTEST_GUILESS_MAIN(testWaylandWrite)

#include "testwlwrite.moc"
