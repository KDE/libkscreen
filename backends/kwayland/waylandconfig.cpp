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
#include <KWayland/Client/outputconfiguration.h>
#include <KWayland/Client/outputdevice.h>
#include <KWayland/Client/outputmanagement.h>

// Wayland
#include <wayland-client-protocol.h>


// Qt
#include <QTimer>

#include <configmonitor.h>
#include <mode.h>


using namespace KScreen;


WaylandConfig::WaylandConfig(QObject *parent)
    : QObject(parent)
    , m_screen(new WaylandScreen(this))
    , m_outputManagement(nullptr)
    , m_blockSignals(true)
    , m_registryInitialized(false)
    , m_newOutputId(0)
    , m_kscreenConfig(nullptr)
{
    qDebug() << " Config creating.";
    connect(this, &WaylandConfig::initialized, &m_syncLoop, &QEventLoop::quit);
    QTimer::singleShot(1000, [=] {
        if (m_syncLoop.isRunning()) {
            qWarning() << "Connection to Wayland server at socket:" << m_connection->socketName() << "timed out.";
            m_syncLoop.quit();
            m_thread->quit();
            m_thread->wait();
        }
    });
    initConnection();
    m_syncLoop.exec();
    m_blockSignals = false;
    qDebug() << "Syncloop done, config initialized" << m_outputMap.count();

}

WaylandConfig::~WaylandConfig()
{
    qDebug() << "bye bye";
    m_thread->quit();
    m_thread->wait();
    m_syncLoop.quit();
}

void WaylandConfig::initConnection()
{
    m_thread = new QThread(this);
    //m_queue = new KWayland::Client::EventQueue(this);
    m_connection = new KWayland::Client::ConnectionThread;

    m_connection->moveToThread(m_thread);
    m_thread->start();
    m_connection->initConnection();

    connect(m_connection, &KWayland::Client::ConnectionThread::connected,
            this, &WaylandConfig::setupRegistry, Qt::QueuedConnection);

    connect(m_connection, &KWayland::Client::ConnectionThread::connectionDied,
            this, &WaylandConfig::disconnected, Qt::QueuedConnection);
    qDebug() << "init...";
    connect(m_connection, &KWayland::Client::ConnectionThread::failed, [=] {
        qDebug() << "Failed to connect to Wayland server at socket:" << m_connection->socketName();
        m_syncLoop.quit();
        m_thread->quit();
        m_thread->wait();
    });
}

void WaylandConfig::disconnected()
{
    //m_syncLoop.quit();

    qDebug() << "Wayland disconnected, cleaning up.";
    // Clean up
    if (m_queue) {
        delete m_queue;
        m_queue = nullptr;
    }
    if (m_thread) {
        m_thread->quit();
        m_thread->wait();
        delete m_thread;
        m_thread = nullptr;
    }
    delete m_connection;
    m_connection = nullptr;


    Q_FOREACH (auto output, m_outputMap.values()) {
        delete output;
    }
    m_outputMap.clear();

//     delete m_screen;
//     m_screen = new WaylandScreen(this);

    qDebug() << "WLC Notifying that we're gone";
    //ConfigMonitor::instance()->notifyUpdate();
    Q_EMIT configChanged(toKScreenConfig());
    Q_EMIT gone();
}

void WaylandConfig::setupRegistry()
{
    m_queue = new KWayland::Client::EventQueue(this);
    m_queue->setup(m_connection);

    m_registry = new KWayland::Client::Registry(this);
    //m_registry->setEventQueue(m_queue);

    connect(m_registry, &KWayland::Client::Registry::outputDeviceAnnounced,
            this, &WaylandConfig::addOutput, Qt::DirectConnection);
    connect(m_registry, &KWayland::Client::Registry::outputDeviceRemoved,
            this, &WaylandConfig::removeOutput, Qt::DirectConnection);

    connect(m_registry, &KWayland::Client::Registry::outputManagementAnnounced, [=](quint32 name, quint32 version) {
        m_outputManagement = m_registry->createOutputManagement(name, version, m_registry);
        checkInitialized();
    });

    connect(m_registry, &KWayland::Client::Registry::interfacesAnnounced, [=] {
        //qDebug() << "Registry::Sync arrived in Backend!:" << m_outputMap.count();
        m_registryInitialized = true;
        m_blockSignals = false;
        checkInitialized();
    });

    m_registry->create(m_connection);
    m_registry->setEventQueue(m_queue);
    m_registry->setup();
    wl_display_flush(m_connection->display());

    //qDebug() << " REG DONE: " << m_registry->outputDevices().count() << m_registry->outputDevices().count();
}

void WaylandConfig::addOutput(quint32 name, quint32 version)
{
    m_newOutputId++;
    quint32 new_id = m_newOutputId;
    if (m_outputMap.keys().contains(new_id)) {
        return;
    }
    if (!m_initializingOutputs.contains(name)) {
        m_initializingOutputs << name;
    }

    auto op = new KWayland::Client::OutputDevice(this);
    WaylandOutput *waylandoutput = new WaylandOutput(new_id, this);
    waylandoutput->bindOutputDevice(m_registry, op, name, version);

    connect(waylandoutput, &WaylandOutput::complete, [=]{
        m_outputMap[waylandoutput->id()] = waylandoutput;
        //qCDebug(KSCREEN_WAYLAND) << "\\n+++++++++++++++++++++++++++++ New Output complete, ID:" << waylandoutput->id() << waylandoutput->output()->model() << m_outputMap.count();
        m_initializingOutputs.removeAll(name);
        checkInitialized();

        if (!m_blockSignals && m_initializingOutputs.empty()) {
            Q_EMIT configChanged(toKScreenConfig());
        }
        connect(waylandoutput, &WaylandOutput::changed, [this]() {
            Q_EMIT configChanged(toKScreenConfig());
        });
    });
}

void WaylandConfig::checkInitialized()
{
    //qDebug() << "CHECK: " << !m_blockSignals << m_registryInitialized << m_initializingOutputs.isEmpty() << m_outputMap.count() << (m_outputManagement != nullptr);
    if (!m_blockSignals && m_registryInitialized &&
        m_initializingOutputs.isEmpty() && m_outputMap.count() && m_outputManagement != nullptr) {
        //qDebug() << "\n ===================== config initialized." << m_outputMap.count();
        m_screen->setOutputs(m_outputMap.values());
        emit initialized();
    };
}

KScreen::ConfigPtr WaylandConfig::toKScreenConfig()
{
    if (m_kscreenConfig == nullptr) {
        m_kscreenConfig = KScreen::ConfigPtr(new Config);
    }
    m_kscreenConfig->setScreen(m_screen->toKScreenScreen(m_kscreenConfig));

    updateKScreenConfig(m_kscreenConfig);
    return m_kscreenConfig;
}

int WaylandConfig::outputId(KWayland::Client::OutputDevice *wlo)
{
    if (m_outputIds.keys().contains(wlo)) {
        return m_outputIds.value(wlo);
    }
    int _id = qHash(wlo->uuid());
    m_outputIds[wlo] = _id;
    //qDebug() << "!!!!!!!!!!!!!!!!!!!!!!!!!! New id: " << _id;
    return _id;
    /*
    Q_FOREACH (auto output, m_outputMap.values()) {
        if (wlo == output) {
            return output->id();
        }
    }
    m_lastOutputId++;
    return m_lastOutputId;
    */
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
        Q_EMIT configChanged(toKScreenConfig());
    }
}

void WaylandConfig::updateKScreenConfig(KScreen::ConfigPtr &config) const
{
    //qDebug() << "updateKScreenConfig!";
    config->setValid(m_connection->display());
    KScreen::ScreenPtr screen = config->screen();
    m_screen->updateKScreenScreen(screen);

    //Removing removed outputs
    KScreen::OutputList outputs = config->outputs();
    Q_FOREACH (KScreen::OutputPtr output, outputs) {
        if (!m_outputMap.keys().contains(output->id())) {
            config->removeOutput(output->id());
            qWarning() << " outputs removed from config" << m_outputMap.keys();
        }
    }

    // Add KScreen::Outputs that aren't in the list yet, handle primaryOutput
    KScreen::OutputList kscreenOutputs = config->outputs();
    Q_FOREACH (auto output, m_outputMap.values()) {

        // FIXME: doesn't work
        //KScreen::OutputPtr kscreenOutput(config->output(output->id()));
        KScreen::OutputPtr kscreenOutput = kscreenOutputs[output->id()];
        if (!kscreenOutput) {
            kscreenOutput = output->toKScreenOutput();
            kscreenOutputs.insert(kscreenOutput->id(), kscreenOutput);
        }
        if (kscreenOutput && m_outputMap.count() == 1) {
            // FIXME: primaryScreen?
            kscreenOutput->setPrimary(true);
        } else if (m_outputMap.count() > 1) {
            //qWarning() << "Multiple outputs, but no way to figure out the primary one. :/";
        }
        output->updateKScreenOutput(kscreenOutput);
    }
    config->setOutputs(kscreenOutputs);
}

QMap<int, WaylandOutput*> WaylandConfig::outputMap() const
{
    return m_outputMap;
}

void WaylandConfig::applyConfig(const KScreen::ConfigPtr &newconfig)
{
    using namespace KWayland::Client;
    // Create a new configuration object
    auto config = m_outputManagement->createConfiguration();

//     // handle applied and failed signals
//     connect(config, &OutputConfiguration::applied, []() {
//         qDebug() << "Configuration applied!";
//     });
//     connect(config, &OutputConfiguration::failed, []() {
//         qDebug() << "Configuration failed!";
//     });
//
    foreach (auto o_new, newconfig->outputs()) {
        auto o_old = m_outputMap[o_new->id()];
        auto device = o_old->outputDevice();
        Q_ASSERT(o_old != nullptr);

        // enabled?
        bool old_enabled = (o_old->outputDevice()->enabled() == OutputDevice::Enablement::Enabled);
        qDebug() << "output:" << o_new->id() << " enabled? " << o_new->isEnabled() << "was" << old_enabled << o_new->pos() << o_old->outputDevice()->globalPosition();

        if (old_enabled != o_new->isEnabled()) {
            auto _enablement = o_new->isEnabled() ? OutputDevice::Enablement::Enabled : OutputDevice::Enablement::Disabled;
            config->setEnabled(o_old->outputDevice(), _enablement);
        }

        // position
        if (device->globalPosition() != o_new->pos()) {
            qDebug() << "setting new position: " << o_new->pos();
            config->setPosition(o_old->outputDevice(), o_new->pos());
        }

        // rotation
        auto r_current = o_old->toKScreenRotation(device->transform());
        auto r_new = o_new->rotation();
        if (r_current != r_new) {
            config->setTransform(device, o_old->toKWaylandTransform(r_new));
        }
    }
    /*
     *    auto device =
    // Change settings
    config->setMode(output, m_clientOutputs.first()->modes().last().id);
    config->setTransform(output, OutputDevice::Transform::Normal);
    config->setPosition(output, QPoint(0, 1920));
    config->setScale(output, 2);
*/
    // Now ask the compositor to apply the changes
    config->apply();

}