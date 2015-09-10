/*************************************************************************************
 *  Copyright 2014 Sebastian Kügler <sebas@kde.org>                                  *
 *  Copyright 2013 Martin Gräßlin <mgraesslin@kde.org>                               *
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

#include "waylandconfig.h"
#include "waylandoutput.h"
#include "waylandscreen.h"
#include "waylandbackend.h"

// KWayland
#include <KWayland/Client/connection_thread.h>
#include <KWayland/Client/event_queue.h>
#include <KWayland/Client/registry.h>
#include <KWayland/Client/outputdevice.h>
#include <KWayland/Client/outputmanagement.h>

// Wayland
#include <wayland-client-protocol.h>

#include <configmonitor.h>
#include <mode.h>


using namespace KScreen;


WaylandConfig::WaylandConfig(QObject *parent)
    : QObject(parent)
    , m_screen(new WaylandScreen(this))
    , m_outputManagement(nullptr)
    , m_blockSignals(true)
    , m_registryInitialized(false)
{
    qCDebug(KSCREEN_WAYLAND) << " Config creating.";
    connect(this, &WaylandConfig::initialized, &m_syncLoop, &QEventLoop::quit);
    initConnection();
    m_syncLoop.exec();
//    m_blockSignals = false;
}

WaylandConfig::~WaylandConfig()
{
    Q_FOREACH (auto output, m_outputMap.values()) {
        delete output;
    }
    m_thread->quit();
    m_thread->wait();
}

void WaylandConfig::initConnection()
{

    // setup connection
    m_connection = new KWayland::Client::ConnectionThread;
    //QSignalSpy connectedSpy(m_connection, &KWayland::Client::ConnectionThread::connected);
    //m_connection->setSocketName(s_socketName);

    m_thread = new QThread(this);
    m_connection->moveToThread(m_thread);
    m_thread->start();

    m_connection->initConnection();
    //connectedSpy.wait();
    //m_syncLoop.quit();


    m_queue = new KWayland::Client::EventQueue(this);


    connect(m_connection, &KWayland::Client::ConnectionThread::connected,
            this, &WaylandConfig::setupRegistry, Qt::QueuedConnection);

    connect(m_connection, &KWayland::Client::ConnectionThread::connectionDied,
            this, &WaylandConfig::disconnected, Qt::QueuedConnection);

    connect(m_connection, &KWayland::Client::ConnectionThread::failed, [=] {
        qDebug() << "Failed to connect to Wayland server at socket:" << m_connection->socketName();
        m_syncLoop.quit();
        m_thread->quit();
        m_thread->wait();
    });
    qDebug() << "Done.";
    return;
    // setup connection
    m_connection = new KWayland::Client::ConnectionThread;
    //QSignalSpy connectedSpy(m_connection, SIGNAL(connected()));
    //m_connection->setSocketName(m_so);

    m_thread = new QThread(this);
    m_connection->moveToThread(m_thread);
    m_thread->start();

    m_connection->initConnection();
    //QVERIFY(connectedSpy.wait());

    //m_queue = new KWayland::Client::EventQueue(this);
    //QVERIFY(!m_queue->isValid());
    //m_queue->setup(m_connection);
    //QVERIFY(m_queue->isValid());



    //m_connection = new KWayland::Client::ConnectionThread;
    //m_connection->moveToThread(&m_thread);
    //m_thread.start();

    connect(m_connection, &KWayland::Client::ConnectionThread::connected,
            this, &WaylandConfig::setupRegistry, Qt::QueuedConnection);
    connect(m_connection, &KWayland::Client::ConnectionThread::connectionDied,
            this, &WaylandConfig::disconnected, Qt::QueuedConnection);

    connect(m_connection, &KWayland::Client::ConnectionThread::failed, [=] {
        qDebug() << "Failed to connect to Wayland server at socket:" << m_connection->socketName();
        m_syncLoop.quit();
        m_thread->quit();
        m_thread->wait();
    });
    //m_connection->initConnection();
}

void WaylandConfig::disconnected()
{
    qDebug() << "Wayland disconnected, cleaning up.";
    // Clean up
    m_thread->quit();
    m_thread->wait();

    Q_FOREACH (auto o, m_outputMap.values()) {
        //delete o;
    }
    m_outputMap.clear();
    delete m_screen;
    m_screen = new WaylandScreen(this);

    qDebug() << "WLC Notifying that we're gone";
    //ConfigMonitor::instance()->notifyUpdate();
    Q_EMIT configChanged(toKScreenConfig());
}

void WaylandConfig::setupRegistry()
{
//     m_registry.create(m_connection->display());
//     QVERIFY(m_registry.isValid());
//     m_registry.setEventQueue(m_queue);
//     m_registry.setup();
//     wl_display_flush(m_connection->display());
//
    qDebug() << "setup registry";
    m_queue = new KWayland::Client::EventQueue(this);
    m_queue->setup(m_connection);

    m_registry = new KWayland::Client::Registry(this);
    m_registry->setEventQueue(m_queue);

    connect(m_registry, &KWayland::Client::Registry::outputDeviceAnnounced,
            this, &WaylandConfig::addOutput, Qt::DirectConnection);
    connect(m_registry, &KWayland::Client::Registry::outputDeviceRemoved,
            this, &WaylandConfig::removeOutput, Qt::DirectConnection);

    connect(m_registry, &KWayland::Client::Registry::outputManagementAnnounced, [=](quint32 name, quint32 version) {
        qDebug() << "====> WL new outputManagementAnnounced";
        m_outputManagement = m_registry->createOutputManagement(name, version, m_registry);
        // FIXME: bind outputmanagement
        //m_outputManagement->setup(m_registry->bindOutputManagement(name, version));
        //m_outputManagement.setup(m_registry.bindOutputManagement(m_announcedSpy->first().first().value<quint32>(),
        checkInitialized();
    });

    connect(m_registry, &KWayland::Client::Registry::interfacesAnnounced, [=] {
        qDebug() << "Registry::Sync arrived in Backend!:" << m_outputMap.count();
        m_registryInitialized = true;
        m_blockSignals = false;
        //emit initialized();
        checkInitialized();
        //Q_EMIT configChanged(toKScreenConfig());
    });

    m_registry->create(m_connection);
    m_registry->setup();
    wl_display_flush(m_connection->display());

    //qDebug() << " REG DONE: " << m_registry->outputs().count() << m_registry->outputDevices().count();
}

void WaylandConfig::addOutput(quint32 name, quint32 version)
{
    qDebug() << "WL Adding output" << name;
    if (m_outputMap.keys().contains(name)) {
        qDebug() << "Output already known";
        return;
    }
    if (!m_initializingOutputs.contains(name)) {
        m_initializingOutputs << name;
    }

    auto op = new KWayland::Client::OutputDevice(this);
    WaylandOutput *waylandoutput = new WaylandOutput(this);
    waylandoutput->setId(outputId(waylandoutput)); // Gives us a new id
    waylandoutput->setOutput(m_registry, op, name, version);
    //waylandoutput->setup(m_registry->bindOutput(name, version));

    connect(waylandoutput, &WaylandOutput::complete, [=]{
        m_outputMap[waylandoutput->id()] = waylandoutput;
        qCDebug(KSCREEN_WAYLAND) << "New Output complete" << name;
        m_initializingOutputs.removeAll(name);
        checkInitialized();
        m_screen->setOutputs(m_outputMap.values());

        if (m_blockSignals) {
            checkInitialized();
        } else {
            //KScreen::ConfigMonitor::instance()->notifyUpdate();
            Q_EMIT configChanged(toKScreenConfig());
        }
    });
}

void WaylandConfig::checkInitialized()
{
    qDebug() << "CHECK: " << !m_blockSignals << m_registryInitialized << m_initializingOutputs.isEmpty() << m_outputMap.count() << (m_outputManagement != nullptr);
    if (!m_blockSignals && m_registryInitialized &&
        m_initializingOutputs.isEmpty() && m_outputMap.count() && m_outputManagement != nullptr) {

        //qDebug() << "WaylandConfig is ready!!";
        emit initialized();
    };
}


KScreen::ConfigPtr WaylandConfig::toKScreenConfig() const
{
    KScreen::ConfigPtr config(new Config);
    config->setScreen(m_screen->toKScreenScreen(config));
    updateKScreenConfig(config);
    return config;
}

int WaylandConfig::outputId(WaylandOutput *wlo)
{
    Q_FOREACH (auto output, m_outputMap.values()) {
        if (wlo == output) {
            return output->id();
        }
    }
    m_lastOutputId++;
    return m_lastOutputId;
}

void WaylandConfig::removeOutput(quint32 id)
{
    qCDebug(KSCREEN_WAYLAND) << "output screen Removed!!! .." << id << m_outputMap[id];
    // Find output matching the QScreen object and remove it
    int removedOutputId = -1;
    Q_FOREACH (auto output, m_outputMap.values()) {
        if (output->id() == id) {
            qDebug() << " removing: Found output matching the kscreen-internal output " << output->id();
            removedOutputId = output->id();
            m_outputMap.remove(removedOutputId);
            delete output;
        }
    }
    if (!m_blockSignals) {
        //KScreen::ConfigMonitor::instance()->notifyUpdate();
        Q_EMIT configChanged(toKScreenConfig());
    }
}

void WaylandConfig::updateKScreenConfig(KScreen::ConfigPtr &config) const
{
    //qDebug() << "updateKScreenConfig!";
    config->setValid(m_connection->display());
    KScreen::ScreenPtr screen = config->screen();
    m_screen->updateKScreenScreen(screen);

    //qDebug() << "MAP: " << m_outputMap;
    //Removing removed outputs
    KScreen::OutputList outputs = config->outputs();
    Q_FOREACH (KScreen::OutputPtr output, outputs) {
        if (!m_outputMap.keys().contains(output->id())) {
            config->removeOutput(output->id());
            qWarning() << " outputs removed from config" << m_outputMap.keys();
        }
    }

    // Add KScreen::Outputs that aren't in the list yet, handle primaryOutput
    Q_FOREACH (auto output, m_outputMap.values()) {

        // FIXME: doesn't work
        KScreen::OutputPtr kscreenOutput(config->output(output->id()));

        if (kscreenOutput && m_outputMap.count() == 1) {
            // FIXME: primaryScreen?
            kscreenOutput->setPrimary(true);
        } else if (m_outputMap.count() > 1) {
            //qWarning() << "Multiple outputs, but no way to figure out the primary one. :/";
        }

        if (!kscreenOutput) {
            kscreenOutput = output->toKScreenOutput(config);
            config->addOutput(kscreenOutput);
        }
        output->updateKScreenOutput(kscreenOutput);

    }
}

QMap<int, WaylandOutput*> WaylandConfig::outputMap() const
{
    return m_outputMap;
}

