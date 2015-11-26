/*************************************************************************************
 *  Copyright 2014 by Sebastian Kügler <sebas@kde.org>                           *
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
#include <QCryptographicHash>
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
#include <KWayland/Server/outputmanagement_interface.h>


#include "../src/backendmanager_p.h"
#include "../src/getconfigoperation.h"
#include "../src/setconfigoperation.h"
#include "../src/config.h"
#include "../src/configmonitor.h"
#include "../src/output.h"
#include "../src/mode.h"
#include "../src/edid.h"

#include "waylandconfigwriter.h"
#include "waylandconfigreader.h"
#include "../tests/waylandtestserver.h"

Q_LOGGING_CATEGORY(KSCREEN_QSCREEN, "kscreen.wayland");

using namespace KScreen;

class testWaylandBackend : public QObject
{
    Q_OBJECT

public:
    explicit testWaylandBackend(QObject *parent = nullptr);

private Q_SLOTS:
    void initTestCase();
    void verifyDisco();
    void loadConfig();

    void verifyConfig();
    void verifyOutputs();
    void verifyModes();
    void verifyScreen();
    void verifyIds();
    void cleanupTestCase();

    void simpleWrite();

private:
    ConfigPtr m_config;
    bool m_startServer;
    WaylandTestServer *m_server;

};

testWaylandBackend::testWaylandBackend(QObject *parent)
    : QObject(parent)
    , m_config(nullptr)
    , m_startServer(true)
{
    m_server = new WaylandTestServer(this);
    m_server->setConfig(TEST_DATA + QStringLiteral("multipleoutput.json"));
    //m_server->setConfig(TEST_DATA + QStringLiteral("singleoutput.json"));
}

void testWaylandBackend::initTestCase()
{
    qputenv("KSCREEN_BACKEND", "kwayland");
    KScreen::BackendManager::instance()->shutdownBackend();
    m_startServer =  qgetenv("KSCREEN_EXTERNAL_WAYLAND_SERVER").isEmpty();

    // This is how KWayland will pick up the right socket,
    // and thus connect to our internal test server.
    setenv("WAYLAND_DISPLAY", s_socketName.toLocal8Bit(), 1);
    if (m_startServer) {
        m_server->start();
    }

    GetConfigOperation *op = new GetConfigOperation();
    op->exec();
    m_config = op->config();
}

void testWaylandBackend::verifyDisco()
{
    //m_config = Config::current();
    //QCOMPARE(m_config->outputs().count(), 0);
    //delete m_config;

}

void testWaylandBackend::loadConfig()
{
    GetConfigOperation *op = new GetConfigOperation();
    op->exec();
    m_config = op->config();
    QVERIFY(m_config->isValid());
    qDebug() << m_config->outputs();
}

void testWaylandBackend::verifyConfig()
{
    QVERIFY(m_config != nullptr);
    if (!m_config) {
        QSKIP("Wayland backend invalid", SkipAll);
    }
}

void testWaylandBackend::verifyScreen()
{
    ScreenPtr screen = m_config->screen();
    //qDebug() << "Screen: " << screen;

    QVERIFY(screen->minSize().width() <= screen->maxSize().width());
    QVERIFY(screen->minSize().height() <= screen->maxSize().height());

    QVERIFY(screen->minSize().width() <= screen->currentSize().width());
    QVERIFY(screen->minSize().height() <= screen->currentSize().height());

    QVERIFY(screen->maxSize().width() >= screen->currentSize().width());
    QVERIFY(screen->maxSize().height() >= screen->currentSize().height());
    QVERIFY(m_config->screen()->maxActiveOutputsCount() > 0);
}

void testWaylandBackend::verifyOutputs()
{
    //qApp->exit(0); // stop dealing signals, results will still be checked

    bool primaryFound = false;
    foreach (const KScreen::OutputPtr op, m_config->outputs()) {
        qDebug() << "CHecking at all";
        if (op->isPrimary()) {
            primaryFound = true;
        }
    }
    qDebug() << "Primary found? " << primaryFound << m_config->outputs();
    //QVERIFY(primaryFound);
    QVERIFY(m_config->outputs().count());
    QCOMPARE(m_server->outputCount(), m_config->outputs().count());

    KScreen::OutputPtr primary = m_config->primaryOutput();
    //QVERIFY(primary->isEnabled());
    QVERIFY(primary->isConnected());
    //qDebug() << " prim modes: " << primary->modes();

    QList<int> ids;
    foreach (auto output, m_config->outputs()) {
        qDebug() << " _____________________ Output: " << output;
        qDebug() << "   output name: " << output->name();
        qDebug() << "   output modes: " << output->currentModeId() << output->modes();
        qDebug() << "   output geometry: " << output->geometry();
        qDebug() << "   output enabled: " << output->isEnabled();
        qDebug() << "   output connect: " << output->isConnected();
        qDebug() << "   output sizeMm : " << output->sizeMm();
        QVERIFY(!output->name().isEmpty());
        QVERIFY(output->id() > -1);
        QVERIFY(output->isConnected());
        //QVERIFY(output->isEnabled());
        QVERIFY(output->geometry() != QRectF(1,1,1,1));
        QVERIFY(output->geometry() != QRectF());
        QVERIFY(output->sizeMm() != QSize());
        QVERIFY(output->edid() != 0);
        QCOMPARE(output->rotation(), Output::None);
        QVERIFY(!ids.contains(output->id()));
        ids << output->id();
    }
}

void testWaylandBackend::verifyModes()
{
    KScreen::OutputPtr primary = m_config->primaryOutput();
    QVERIFY(primary);
    QVERIFY(primary->modes().count() > 0);

    foreach (auto output, m_config->outputs()) {
        foreach (auto mode, output->modes()) {
            QVERIFY(!mode->name().isEmpty());
            QVERIFY(mode->refreshRate() > 0);
            QVERIFY(mode->size() != QSize());
        }
    }
}

void testWaylandBackend::verifyIds()
{
    /*
    qDebug() << " -----> ID: " << QString("ABC").toInt();
    qDebug() << " -----> ID: " << QByteArray("ABC").toInt();
    qDebug() << " -----> ID: " << QByteArray("FF", 16).toInt();

    QString _s = "Samsung-Bla";
    QString _s2 = "FC";
    QString _s3 = "35fc0da6701d42fa0d37c2e300e7005d";
    bool ok = false;
    int i1 = _s.toInt(&ok, 10);
    int i2 = _s.toInt(&ok, 16);
    qDebug() << " -----> ID: " << i1 << ok;
    qDebug() << " -----> ID: " << i2 << ok;
    qDebug() << " -----> ID: " << i1 << ok;
    qDebug() << " -----> ID2: " << _s2.toInt(&ok, 16) << ok;
    qDebug() << " -----> ID3: " << _s3.toLongLong(&ok) << ok;

    auto _h = QCryptographicHash::hash(_s.toUtf8(), QCryptographicHash::Md5).toHex();
    int id = _h.toInt(&ok, 16);
    qDebug() << " -----> ID hash: " << _h << id;
    */

    QList<quint32> ids;
    foreach (auto output, m_config->outputs()) {
        QVERIFY(ids.contains(output->id()) == false);
        QVERIFY(output->id() > 0);
        ids << output->id();
    }
}

void testWaylandBackend::simpleWrite()
{
    KScreen::BackendManager::instance()->shutdownBackend();
    GetConfigOperation *op = new GetConfigOperation();
    op->exec();
    m_config = op->config();
    qDebug() << "Ouptuts: " << m_config->outputs();
    auto output = m_config->output(1);
    auto n_mode = QStringLiteral("800x600@60");
    auto o_mode = output->currentModeId();
    qDebug() << "Current mode: " << o_mode;
    qDebug() << "o->m" << output->modes();
    output->setCurrentModeId(n_mode);

    auto setop = new SetConfigOperation(m_config);
    QVERIFY(setop->exec());

}

void testWaylandBackend::cleanupTestCase()
{
    if (m_startServer) {
        //QCOMPARE(m_config->outputs().count(), 0);
    }
    //delete m_config;
    m_config->deleteLater();
    KScreen::BackendManager::instance()->shutdownBackend();
}


QTEST_GUILESS_MAIN(testWaylandBackend)

#include "testkwaylandbackend.moc"
