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

    void cleanupTestCase();

private:
    void startWaylandServer();
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

testWaylandBackend::testWaylandBackend(QObject *parent)
    : QObject(parent)
    , m_config(nullptr)
    , m_startServer(true)
    , m_display(nullptr)
    , m_compositor(nullptr)
    , m_seat(nullptr)
    , m_shell(nullptr)
{
}

void testWaylandBackend::initTestCase()
{
    setenv("KSCREEN_BACKEND", "wayland", 1);
    KScreen::BackendManager::instance()->shutdownBackend();
    m_backend = qgetenv("KSCREEN_BACKEND").constData();
    m_startServer = QString::fromLocal8Bit(qgetenv("KSCREEN_EXTERNAL_WAYLAND_SERVER").constData()).isEmpty();

    // This is how KWayland will pick up the right socket,
    // and thus connect to our internal test server.
    setenv("WAYLAND_DISPLAY", s_socketName.toLocal8Bit(), 1);

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

void testWaylandBackend::startWaylandServer()
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

    {
        OutputInterface *output = m_display->createOutput(this);
        output->addMode(QSize(800, 600), OutputInterface::ModeFlags(OutputInterface::ModeFlag::Preferred));
        output->addMode(QSize(1024, 768));
        output->addMode(QSize(1280, 1024), OutputInterface::ModeFlags(), 90000);
        output->setCurrentMode(QSize(1024, 768));
        output->setGlobalPosition(QPoint(0, 0));
        output->setPhysicalSize(QSize(400, 300)); // FIXME mm?
        output->setManufacturer("Darknet Industries");
        output->setModel("Small old monitor");
        output->create();
        m_outputs << output;
    }
    {
        auto output = m_display->createOutput(this);
        output->addMode(QSize(1600, 1200), OutputInterface::ModeFlags(OutputInterface::ModeFlag::Preferred));
        output->addMode(QSize(1920, 1080));
        output->addMode(QSize(2840, 2160), OutputInterface::ModeFlags(), 100000);
        output->setCurrentMode(QSize(1920, 1080));
        output->setGlobalPosition(QPoint(1024, 0));
        output->setPhysicalSize(QSize(1600, 900)); // FIXME mm?
        output->setManufacturer("Shiny Electrics");
        output->setModel("XXL Television");
        output->create();
        m_outputs << output;
    }

    QVERIFY(m_display->isRunning());
    qDebug() << "Wayland server running.";
}

void testWaylandBackend::loadConfig()
{
    GetConfigOperation *op = new GetConfigOperation();
    op->exec();
    m_config = op->config();
    QVERIFY(m_config->isValid());
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

    //qDebug() << "Outputs: " << m_config->outputs();
    bool primaryFound = false;
    foreach (const KScreen::OutputPtr op, m_config->outputs()) {
        //qDebug() << "CHecking at all";
        if (op->isPrimary()) {
            primaryFound = true;
        }
    }
    //qDebug() << "Primary found? " << primaryFound << m_config->outputs();
    QVERIFY(primaryFound);
    QVERIFY(m_config->outputs().count());

    KScreen::OutputPtr primary = m_config->primaryOutput();
    QVERIFY(primary->isEnabled());
    QVERIFY(primary->isConnected());
    //qDebug() << " prim modes: " << primary->modes();

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
        QVERIFY(output->isEnabled());
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

void testWaylandBackend::stopWaylandServer()
{
    if (!m_startServer) {
        return;
    }
    KScreen::ConfigMonitor::instance()->addConfig(m_config);
    QSignalSpy connectedSpy(ConfigMonitor::instance(), SIGNAL(configurationChanged()));
    QVERIFY(connectedSpy.isValid());

    // actually stop the Wayland server
    delete m_display;
    m_display = nullptr;

    QVERIFY(connectedSpy.wait(1000));
    QCOMPARE(connectedSpy.count(), 1);
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

#include "testwaylandbackend.moc"
