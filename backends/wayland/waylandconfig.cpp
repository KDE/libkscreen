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
#include <KWayland/Client/screen_management.h>

#include <configmonitor.h>
#include <mode.h>


using namespace KScreen;


WaylandConfig::WaylandConfig(QObject *parent)
    : QObject(parent)
    , m_screen(new WaylandScreen(this))
    , m_screen_management(nullptr)
    , m_blockSignals(true)
    , m_registryInitialized(false)
    , m_disabledOutputsDone(false)

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
    m_thread.quit();
    m_thread.wait();
}

void WaylandConfig::initConnection()
{
    m_connection = new KWayland::Client::ConnectionThread;
    m_connection->moveToThread(&m_thread);
    m_thread.start();

    connect(m_connection, &KWayland::Client::ConnectionThread::connected,
            this, &WaylandConfig::setupRegistry, Qt::QueuedConnection);
    connect(m_connection, &KWayland::Client::ConnectionThread::connectionDied,
            this, &WaylandConfig::disconnected, Qt::QueuedConnection);

    connect(m_connection, &KWayland::Client::ConnectionThread::failed, [=] {
        qDebug() << "Failed to connect to Wayland server at socket:" << m_connection->socketName();
        m_syncLoop.quit();
        m_thread.quit();
        m_thread.wait();
    });
    m_connection->initConnection();
}

void WaylandConfig::disconnected()
{
    qDebug() << "Wayland disconnected, cleaning up.";
    // Clean up
    m_thread.quit();
    m_thread.wait();

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
    m_queue = new KWayland::Client::EventQueue(this);
    m_queue->setup(m_connection);

    m_registry = new KWayland::Client::Registry(this);
    m_registry->setEventQueue(m_queue);

    connect(m_registry, &KWayland::Client::Registry::outputAnnounced,
            this, &WaylandConfig::addOutput, Qt::DirectConnection);
    connect(m_registry, &KWayland::Client::Registry::outputRemoved,
            this, &WaylandConfig::removeOutput, Qt::DirectConnection);

    connect(m_registry, &KWayland::Client::Registry::interfacesAnnounced, [=] {
        qDebug() << "Registry::Sync arrived in Backend!:";
        m_registryInitialized = true;
        m_blockSignals = false;
        emit initialized();
        checkInitialized();
        //Q_EMIT configChanged(toKScreenConfig());
    });

    connect(m_registry, &KWayland::Client::Registry::screenManagementAnnounced, [=](quint32 name, quint32 version) {
        qDebug() << "WL new screenManagementAnnounced";
        m_screen_management = m_registry->createScreenManagement(name, version, m_registry);

        connect(m_screen_management, &KWayland::Client::ScreenManagement::done, [=] {
            qDebug() << "WL disabled outputs: " << m_screen_management->disabledOutputs().count();
            m_disabledOutputsDone = true;
            foreach (auto dop, m_screen_management->disabledOutputs()) {
                addDisabledOutput(dop);
            }
        });
    });
    m_registry->create(m_connection);
    m_registry->setup();
}

void WaylandConfig::addDisabledOutput(KWayland::Client::DisabledOutput* op)
{
    return;
    WaylandOutput *waylandoutput = new WaylandOutput(this);
    waylandoutput->setId(outputId(waylandoutput));
    waylandoutput->setDisabledOutput(op);
//     //waylandoutput->setName(op->name());
//     // ... more properties
//
    m_outputMap[waylandoutput->id()] = waylandoutput;
    connect(waylandoutput, &WaylandOutput::complete, [=]{
        m_outputMap[waylandoutput->id()] = waylandoutput;
        qCDebug(KSCREEN_WAYLAND) << "New Output complete";
        m_screen->setOutputs(m_outputMap.values());

        Q_EMIT configChanged(toKScreenConfig());
//         if (m_blockSignals) {
//             //m_initializingOutputs.removeAll(name);
//             checkInitialized();
//         } else {
//             //KScreen::ConfigMonitor::instance()->notifyUpdate();
//             Q_EMIT configChanged(toKScreenConfig());
//         }
    });
}

void WaylandConfig::addOutput(quint32 name, quint32 version)
{
    qDebug() << "WL Adding output" << name;
    if (!m_blockSignals) {
        m_initializingOutputs << name;
    }
    if (m_outputMap.keys().contains(name)) {
        qDebug() << "Output already known";
        return;
    }

    auto op = new KWayland::Client::Output(this);
    WaylandOutput *waylandoutput = new WaylandOutput(this);
    waylandoutput->setId(outputId(waylandoutput)); // Gives us a new id
    waylandoutput->setOutput(m_registry, op, name, version);
    //waylandoutput->setup(m_registry->bindOutput(name, version));

    connect(waylandoutput, &WaylandOutput::complete, [=]{
        m_outputMap[waylandoutput->id()] = waylandoutput;
        qCDebug(KSCREEN_WAYLAND) << "New Output complete" << name;
        m_screen->setOutputs(m_outputMap.values());

        if (m_blockSignals) {
            m_initializingOutputs.removeAll(name);
            checkInitialized();
        } else {
            //KScreen::ConfigMonitor::instance()->notifyUpdate();
            Q_EMIT configChanged(toKScreenConfig());
        }
    });
}

void WaylandConfig::checkInitialized()
{
    if (!m_blockSignals && m_registryInitialized && m_disabledOutputsDone &&
        m_initializingOutputs.isEmpty() && m_outputMap.count()) {

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

    qDebug() << "MAP: " << m_outputMap;
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
            qWarning() << "Multiple outputs, but no way to figure out the primary one. :/";
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

