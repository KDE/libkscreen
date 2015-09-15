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
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

#include <KSharedConfig>
#include <KConfigGroup>

// KWayland
#include <KWayland/Client/connection_thread.h>
#include <KWayland/Client/event_queue.h>
#include <KWayland/Client/registry.h>
#include <KWayland/Client/outputdevice.h>

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

class TestWaylandOutputs : public QObject
{
    Q_OBJECT

public:
    explicit TestWaylandOutputs(QObject *parent = nullptr);

private Q_SLOTS:
    void init();
    void cleanup();
    void cleanupTestCase();

    void testConfigs_data();
    void testConfigs();

private:
    void readConfig(const QString &jsonFile);

    QJsonArray jsonOutputs;
    void showJsonOutput(const QVariantMap &o);

    ConfigPtr m_config;
    WaylandTestServer *m_server;

    QMap<KWayland::Client::OutputDevice::Transform, QString> m_transformMap;
    QMap<KScreen::Output::Rotation, QString> m_rotationMap;
};

TestWaylandOutputs::TestWaylandOutputs(QObject *parent)
    : QObject(parent)
    , m_config(nullptr)
{
    m_transformMap[KWayland::Client::OutputDevice::Transform::Normal] = "KScreen::Output::None";
    m_transformMap[KWayland::Client::OutputDevice::Transform::Rotated90] = "KScreen::Output::Right";
    m_transformMap[KWayland::Client::OutputDevice::Transform::Rotated180] = "KScreen::Output::Inverted";
    m_transformMap[KWayland::Client::OutputDevice::Transform::Rotated270] = "KScreen::Output::Left";
    m_transformMap[KWayland::Client::OutputDevice::Transform::Flipped] = "KScreen::Output::None";
    m_transformMap[KWayland::Client::OutputDevice::Transform::Flipped90] = "KScreen::Output::Right";
    m_transformMap[KWayland::Client::OutputDevice::Transform::Flipped180] = "KScreen::Output::Inverted";
    m_transformMap[KWayland::Client::OutputDevice::Transform::Flipped270] = "KScreen::Output::Left";
    m_rotationMap[KScreen::Output::None] = "KScreen::Output::None";
    m_rotationMap[KScreen::Output::Left] = "KScreen::Output::Left";
    m_rotationMap[KScreen::Output::Right] = "KScreen::Output::Right";
    m_rotationMap[KScreen::Output::Inverted] = "KScreen::Output::Inverted";
}

void TestWaylandOutputs::init()
{
    setenv("KSCREEN_BACKEND", "wayland", 1);
    KScreen::BackendManager::instance()->shutdownBackend();

    // This is how KWayland will pick up the right socket,
    // and thus connect to our internal test server.
    setenv("WAYLAND_DISPLAY", s_socketName.toLocal8Bit(), 1);

    m_server = new WaylandTestServer(this);
    //m_server->start();
}

void TestWaylandOutputs::cleanup()
{
    KScreen::BackendManager::instance()->shutdownBackend();
    delete m_server;
}

void TestWaylandOutputs::testConfigs_data()
{
    QTest::addColumn<QString>("configfile");

    QTest::newRow("wayland.json") << "wayland.json";
    //QTest::newRow("default.json") << "default.json";
    //QTest::newRow("multipleoutput.json") << "multipleoutput.json";
}


void TestWaylandOutputs::readConfig(const QString& jsonFile)
{
    QFile file(jsonFile);
    file.open(QIODevice::ReadOnly);

    QJsonDocument jsonDoc = QJsonDocument::fromJson(file.readAll());
    QJsonObject json = jsonDoc.object();

    QJsonArray omap = json["outputs"].toArray();
    Q_FOREACH(const QJsonValue &value, omap) {
        const QVariantMap &output = value.toObject().toVariantMap();
        if (output["connected"].toBool()) {
            jsonOutputs << value;
            //showJsonOutput(output);
        }
    }
    qDebug() << "ID Parsed " << jsonOutputs.count() << "outputs.";

}

void TestWaylandOutputs::showJsonOutput(const QVariantMap& o)
{
    qDebug() << "________________________________________________________";
    qDebug() << "           Name:" << o["name"].toString();
    qDebug() << "          Model:" << o["model"].toString();
    qDebug() << "   Manufacturer:" << o["manufacturer"].toString();
    qDebug() << "        Enabled:" << o["enabled"].toBool();
    qDebug() << "       Position:" << KScreen::WaylandConfigReader::pointFromJson(o["pos"]);
    qDebug() << "     Resolution:" << KScreen::WaylandConfigReader::sizeFromJson(o["size"]);
    qDebug() << "       Rotation:" << o["rotation"].toInt();
    qDebug() << "  Physical size:" << KScreen::WaylandConfigReader::sizeFromJson(o["sizeMM"]);

    auto ee = new Edid(o["edid"].toByteArray(), this); // FIXME: doesn't work
    qDebug() << "      Edid size:" << o["edid"].toString().size();
    qDebug() << " Edid device ID:" << ee->deviceId();
    qDebug() << "    Edid vendor:" << ee->vendor();
    qDebug() << "      Edid size:" << QSize(ee->width(), ee->height());
    qDebug() << "    Edid serial:" << ee->serial();

    QString _ms;
    foreach (auto v, o["modes"].toList()) {
        auto m = v.toMap();
        if (m["id"].toInt() == o["currentModeId"].toInt()) {
            _ms.append("->");
        }
        if (o["preferredModes"].toList().contains(m["id"])) {
            _ms.append("*");
        }
        _ms.append(m["name"].toString());
        _ms.append(" ");
    }
    _ms.chop(1);
    qDebug() << "          Modes:" << _ms;

    qDebug() << "========================================================";
}



void TestWaylandOutputs::testConfigs()
{
    QFETCH(QString, configfile);
    QString cfg = TEST_DATA + configfile;
    //qDebug() << "Config file: " << cfg;
    readConfig(cfg);
    //return;
    m_server->setConfig(cfg);
    m_server->start();

    QCOMPARE(m_server->outputCount(), jsonOutputs.count());

    GetConfigOperation *op = new GetConfigOperation();
    op->exec();
    m_config = op->config();

    QVERIFY(m_config);
    //QCOMPARE(m_config->outputs().count(), jsonOutputs.count());

    QVERIFY(m_config->outputs().count());
    QList<int> ids;
    foreach (auto output, m_config->outputs()) {
        //continue;
        qDebug() << " _____________________ Output: " << output << output->id();
        qDebug() << "   output name: " << output->name();
        qDebug() << "   output modes: " << output->modes().count() << output->modes().keys();
        qDebug() << "   output enabled: " << output->isEnabled();
        qDebug() << "   output connect: " << output->isConnected();
        qDebug() << "   output sizeMm : " << output->sizeMm();
        qDebug() << "   output rotation: " << m_rotationMap[output->rotation()];
        QVERIFY(!output->name().isEmpty());
        QVERIFY(output->id() > -1);
        QVERIFY(!ids.contains(output->id()));
        ids << output->id();
        QVERIFY(output->isConnected());
        QVERIFY(output->edid() != 0);
        if (output->id() == 71) {
            QCOMPARE(output->rotation(), Output::Left);
            QCOMPARE(output->isEnabled(), false);
        } else {
            QCOMPARE(output->rotation(), Output::None);
            QCOMPARE(output->isEnabled(), true);
        }
        QVERIFY(output->geometry() != QRectF(1,1,1,1));
        QVERIFY(output->geometry() != QRectF());
        if (configfile.endsWith("default.json")) {
            //qDebug() << "Output MM" << output->name() << output->sizeMm();
            QVERIFY(output->sizeMm() != QSize());
        }
        if (!output->isEnabled()) {
            continue;
        }
        //QVERIFY(output->isEnabled());
    }
}

void TestWaylandOutputs::cleanupTestCase()
{
    m_config->deleteLater();
    KScreen::BackendManager::instance()->shutdownBackend();
}


QTEST_GUILESS_MAIN(TestWaylandOutputs)

#include "testwloutputs.moc"
