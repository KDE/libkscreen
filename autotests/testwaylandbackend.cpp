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

#define QT_GUI_LIB

#include <QCoreApplication>
#include <QtTest/QtTest>
#include <QtCore/QObject>

// KWayland
#include <KWayland/Client/connection_thread.h>
#include <KWayland/Client/event_queue.h>
#include <KWayland/Client/registry.h>

#include <KWayland/Server/compositor_interface.h>
#include <KWayland/Server/display.h>
#include <KWayland/Server/output_interface.h>
#include <KWayland/Server/seat_interface.h>
#include <KWayland/Server/shell_interface.h>


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
    void startWaylandServer();

    void loadConfig();
    void verifyConfig();
    void verifyOutputs();
    void verifyModes();
    void verifyScreen();

    void stopWaylandServer();
    void cleanupTestCase();

private:
    void verifyAsync();
    QProcess m_process;
    Config *m_config;
    QString m_backend;

    KWayland::Server::Display *m_display;
    KWayland::Server::CompositorInterface *m_compositor;
    KWayland::Server::OutputInterface *m_output;
    KWayland::Server::SeatInterface *m_seat;
    KWayland::Server::ShellInterface *m_shell;
};

testWaylandBackend::testWaylandBackend(QObject *parent)
    : QObject(parent)
    , m_config(nullptr)
    , m_display(nullptr)
    , m_compositor(nullptr)
    , m_output(nullptr)
    , m_seat(nullptr)
    , m_shell(nullptr)
{
}

void testWaylandBackend::initTestCase()
{
    setenv("KSCREEN_BACKEND", "wayland", 1);
    m_backend = qgetenv("KSCREEN_BACKEND").constData();

    // This is how KWayland will pick up the right socket,
    // and thus connect to our internal test server.
    setenv("WAYLAND_DISPLAY", s_socketName.toLocal8Bit(), 1);
}

void testWaylandBackend::verifyDisco()
{
    //m_config = Config::current();
    //QCOMPARE(m_config->outputs().count(), 0);
    //delete m_config;

}

void testWaylandBackend::startWaylandServer()
{
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

    m_output = m_display->createOutput(this);
    m_output->addMode(QSize(800, 600), OutputInterface::ModeFlags(OutputInterface::ModeFlag::Preferred));
    m_output->addMode(QSize(1024, 768));
    m_output->addMode(QSize(1280, 1024), OutputInterface::ModeFlags(), 90000);
    m_output->setCurrentMode(QSize(1024, 768));
    m_output->create();

    QVERIFY(m_display->isRunning());
    qDebug() << "Wayland server running.";
}

void testWaylandBackend::loadConfig()
{
    m_config = Config::current();
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
    Screen *screen = m_config->screen();
    qDebug() << "Screen: " << screen;

    QVERIFY(screen->minSize().width() <= screen->maxSize().width());
    QVERIFY(screen->minSize().height() <= screen->maxSize().height());

    QVERIFY(screen->minSize().width() <= screen->currentSize().width());
    QVERIFY(screen->minSize().height() <= screen->currentSize().height());

    QVERIFY(screen->maxSize().width() >= screen->currentSize().width());
    QVERIFY(screen->maxSize().height() >= screen->currentSize().height());
    QVERIFY(m_config->screen()->maxActiveOutputsCount() > 0);
}


void testWaylandBackend::verifyAsync()
{
    KScreen::ConfigMonitor::instance()->addConfig(m_config);
    connect(KScreen::ConfigMonitor::instance(), &KScreen::ConfigMonitor::configurationChanged,
            this, &testWaylandBackend::verifyOutputs);
    qApp->exec();
}

void testWaylandBackend::verifyOutputs()
{
    qApp->exit(0); // stop dealing signals, results will still be checked

    //qDebug() << "Outputs: " << m_config->outputs();
    bool primaryFound = false;
    foreach (const KScreen::Output* op, m_config->outputs()) {
        qDebug() << "CHecking at all";
        if (op->isPrimary()) {
            primaryFound = true;
        }
    }
    //qDebug() << "Primary found? " << primaryFound << m_config->outputs();
    QVERIFY(primaryFound);
    QVERIFY(m_config->outputs().count());

    KScreen::Output *primary = m_config->primaryOutput();
    QVERIFY(primary->isEnabled());
    QVERIFY(primary->isConnected());
    qDebug() << " prim modes: " << primary->modes();

    QList<int> ids;
    foreach (auto output, m_config->outputs()) {
        qDebug() << " _____________________ Output: " << output;
        qDebug() << "   output name: " << output->name();
        qDebug() << "   output modes: " << output->modes().count() << output->modes();
        qDebug() << "   output enabled: " << output->isEnabled();
        qDebug() << "   output connect: " << output->isConnected();
        qDebug() << "   output sizeMm : " << output->sizeMm();
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
    KScreen::Output *primary = m_config->primaryOutput();
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
    delete m_display;
    m_display = nullptr;
}

void testWaylandBackend::cleanupTestCase()
{
    //stopWaylandServer();
    delete m_config;

    m_config = Config::current();
    QCOMPARE(m_config->outputs().count(), 0);
    delete m_config;

    qApp->exit(0);
}


QTEST_MAIN(testWaylandBackend)

#include "testwaylandbackend.moc"
