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

using namespace KScreen;

class testWaylandBackend : public QObject
{
    Q_OBJECT

public:
    explicit testWaylandBackend(QObject *parent = nullptr);

private Q_SLOTS:
    void initTestCase();
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
    WaylandTestServer *m_server;

};

testWaylandBackend::testWaylandBackend(QObject *parent)
    : QObject(parent)
    , m_config(nullptr)
{
    m_server = new WaylandTestServer(this);
    m_server->setConfig(TEST_DATA + QStringLiteral("multipleoutput.json"));
}

void testWaylandBackend::initTestCase()
{
    qputenv("KSCREEN_BACKEND", "kwayland");
    KScreen::BackendManager::instance()->shutdownBackend();
    // This is how KWayland will pick up the right socket,
    // and thus connect to our internal test server.
    setenv("WAYLAND_DISPLAY", s_socketName.toLocal8Bit(), 1);
    m_server->start();

    GetConfigOperation *op = new GetConfigOperation();
    op->exec();
    m_config = op->config();
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
    bool primaryFound = false;
    foreach (const KScreen::OutputPtr op, m_config->outputs()) {
        if (op->isPrimary()) {
            primaryFound = true;
        }
    }
    //qDebug() << "Primary found? " << primaryFound << m_config->outputs();
    QVERIFY(primaryFound);
    QVERIFY(m_config->outputs().count());
    QCOMPARE(m_server->outputCount(), m_config->outputs().count());

    KScreen::OutputPtr primary = m_config->primaryOutput();
    QVERIFY(primary->isEnabled());
    QVERIFY(primary->isConnected());

    QList<int> ids;
    foreach (auto output, m_config->outputs()) {
        QVERIFY(!output->name().isEmpty());
        QVERIFY(output->id() > -1);
        QVERIFY(output->isConnected());
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
    auto output = m_config->output(1);
    auto n_mode = QStringLiteral("800x600@60");
    auto o_mode = output->currentModeId();
    output->setCurrentModeId(n_mode);

    auto setop = new SetConfigOperation(m_config);
    QVERIFY(setop->exec());
}

void testWaylandBackend::cleanupTestCase()
{
    m_config->deleteLater();
    KScreen::BackendManager::instance()->shutdownBackend();
}


QTEST_GUILESS_MAIN(testWaylandBackend)

#include "testkwaylandbackend.moc"
