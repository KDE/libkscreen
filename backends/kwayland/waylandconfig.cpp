/*************************************************************************************
 *  Copyright 2014-2015 Sebastian Kügler <sebas@kde.org>                             *
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
    connect(this, &WaylandConfig::initialized, &m_syncLoop, &QEventLoop::quit);
    QTimer::singleShot(1000, [this] {
        if (m_syncLoop.isRunning()) {
            qCWarning(KSCREEN_WAYLAND) << "Connection to Wayland server at socket:" << m_connection->socketName() << "timed out.";
            m_syncLoop.quit();
            m_thread->quit();
            m_thread->wait();
        }
    });
    initConnection();
    m_syncLoop.exec();
}

WaylandConfig::~WaylandConfig()
{
    m_thread->quit();
    m_thread->wait();
    m_syncLoop.quit();
}

void WaylandConfig::initConnection()
{
    m_thread = new QThread(this);
    //m_queue = new KWayland::Client::EventQueue(this);
    m_connection = new KWayland::Client::ConnectionThread;

    connect(m_connection, &KWayland::Client::ConnectionThread::connected,
            this, &WaylandConfig::setupRegistry, Qt::QueuedConnection);

    connect(m_connection, &KWayland::Client::ConnectionThread::connectionDied,
            this, &WaylandConfig::disconnected, Qt::QueuedConnection);
    connect(m_connection, &KWayland::Client::ConnectionThread::failed, [this] {
        qCWarning(KSCREEN_WAYLAND) << "Failed to connect to Wayland server at socket:" << m_connection->socketName();
        m_syncLoop.quit();
        m_thread->quit();
        m_thread->wait();
    });

    m_thread->start();
    m_connection->moveToThread(m_thread);
    m_connection->initConnection();

}

void WaylandConfig::blockSignals()
{
    Q_ASSERT(m_blockSignals == false);
    m_blockSignals = true;
}

void WaylandConfig::unblockSignals()
{
    Q_ASSERT(m_blockSignals == true);
    m_blockSignals = false;
}

void WaylandConfig::disconnected()
{
    qCWarning(KSCREEN_WAYLAND) << "Wayland disconnected, cleaning up.";
    Q_FOREACH (auto output, m_outputMap.values()) {
        delete output;
    }
    m_outputMap.clear();

    // Clean up
    if (m_queue) {
        delete m_queue;
        m_queue = nullptr;
    }

    m_connection->deleteLater();
    m_connection = nullptr;

    if (m_thread) {
        m_thread->quit();
        if (!m_thread->wait(3000)) {
            m_thread->terminate();
            m_thread->wait();
        }
        delete m_thread;
        m_thread = nullptr;
    }

    Q_EMIT configChanged(toKScreenConfig());
    Q_EMIT gone();
}

void WaylandConfig::setupRegistry()
{
    m_queue = new KWayland::Client::EventQueue(this);
    m_queue->setup(m_connection);

    m_registry = new KWayland::Client::Registry(this);

    connect(m_registry, &KWayland::Client::Registry::outputDeviceAnnounced,
            this, &WaylandConfig::addOutput);
    connect(m_registry, &KWayland::Client::Registry::outputDeviceRemoved,
            this, &WaylandConfig::removeOutput);

    connect(m_registry, &KWayland::Client::Registry::outputManagementAnnounced,
            this, [this](quint32 name, quint32 version) {
                m_outputManagement = m_registry->createOutputManagement(name, version, m_registry);
                checkInitialized();
            }
    );

    connect(m_registry, &KWayland::Client::Registry::interfacesAnnounced,
            this, [this] {
                m_registryInitialized = true;
                unblockSignals();
                checkInitialized();
            }
    );

    m_registry->create(m_connection);
    m_registry->setEventQueue(m_queue);
    m_registry->setup();
}

void WaylandConfig::addOutput(quint32 name, quint32 version)
{
    m_newOutputId++;
    quint32 new_id = m_newOutputId;
    m_outputIds[name] = new_id;
    if (m_outputMap.keys().contains(new_id)) {
        return;
    }
    if (!m_initializingOutputs.contains(name)) {
        m_initializingOutputs << name;
    }

    auto op = new KWayland::Client::OutputDevice(this);
    WaylandOutput *waylandoutput = new WaylandOutput(new_id, this);
    waylandoutput->bindOutputDevice(m_registry, op, name, version);

    // finalize: when the output is done, we put it in the known outputs map,
    // remove if from the list of initializing outputs, and emit configChanged()
    connect(waylandoutput, &WaylandOutput::complete, this, [this, waylandoutput, name]{

        m_outputMap.insert(waylandoutput->id(), waylandoutput);
        m_initializingOutputs.removeAll(name);
        checkInitialized();

        if (!m_blockSignals && m_initializingOutputs.empty()) {
            m_screen->setOutputs(m_outputMap.values());
            Q_EMIT configChanged(toKScreenConfig());
        }
        connect(waylandoutput, &WaylandOutput::changed, this, [this]() {
            if (!m_blockSignals) {
                Q_EMIT configChanged(toKScreenConfig());
            }
        });
    });
}

void WaylandConfig::checkInitialized()
{
    if (!m_blockSignals && m_registryInitialized &&
        m_initializingOutputs.isEmpty() && m_outputMap.count() && m_outputManagement != nullptr) {
        m_screen->setOutputs(m_outputMap.values());
        emit initialized();
    }
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

void WaylandConfig::removeOutput(quint32 name)
{
    const int kscreen_id = m_outputIds[name];
    auto output = m_outputMap[kscreen_id];
    m_outputMap.remove(kscreen_id);
    m_screen->setOutputs(m_outputMap.values());
    delete output;
    if (!m_blockSignals) {
        Q_EMIT configChanged(toKScreenConfig());
    }
}

void WaylandConfig::updateKScreenConfig(KScreen::ConfigPtr &config) const
{
    config->setValid(m_connection->display());
    KScreen::ScreenPtr screen = config->screen();
    m_screen->updateKScreenScreen(screen);

    //Removing removed outputs
    KScreen::OutputList outputs = config->outputs();
    Q_FOREACH (const KScreen::OutputPtr &output, outputs) {
        if (!m_outputMap.keys().contains(output->id())) {
            config->removeOutput(output->id());
        }
    }

    // Add KScreen::Outputs that aren't in the list yet, handle primaryOutput
    KScreen::OutputList kscreenOutputs = config->outputs();
    Q_FOREACH (const auto &output, m_outputMap.values()) {
        KScreen::OutputPtr kscreenOutput = kscreenOutputs[output->id()];
        if (!kscreenOutput) {
            kscreenOutput = output->toKScreenOutput();
            kscreenOutputs.insert(kscreenOutput->id(), kscreenOutput);
        }
        if (kscreenOutput && m_outputMap.count() == 1) {
            kscreenOutput->setPrimary(true);
        } else if (m_outputMap.count() > 1) {
            // primaryScreen concept doesn't exist in kwayland, so we don't set one
            //qCWarning(KSCREEN_WAYLAND) << "Multiple outputs, but no way to figure out the primary one. :/";
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
    auto wlOutputConfiguration = m_outputManagement->createConfiguration();

    Q_FOREACH (auto output, newconfig->outputs()) {
        auto o_old = m_outputMap[output->id()];
        auto device = o_old->outputDevice();
        Q_ASSERT(o_old != nullptr);

        // enabled?
        bool old_enabled = (o_old->outputDevice()->enabled() == OutputDevice::Enablement::Enabled);
        if (old_enabled != output->isEnabled()) {
            auto _enablement = output->isEnabled() ? OutputDevice::Enablement::Enabled : OutputDevice::Enablement::Disabled;
            wlOutputConfiguration->setEnabled(o_old->outputDevice(), _enablement);
        }

        // position
        if (device->globalPosition() != output->pos()) {
            wlOutputConfiguration->setPosition(o_old->outputDevice(), output->pos());
        }

        // rotation
        auto r_current = o_old->toKScreenRotation(device->transform());
        auto r_new = output->rotation();
        if (r_current != r_new) {
            wlOutputConfiguration->setTransform(device, o_old->toKWaylandTransform(r_new));
        }

        // mode
        int w_currentmodeid = device->currentMode().id;
        QString l_newmodeid = output->currentModeId();
        int w_newmodeid = o_old->toKWaylandModeId(l_newmodeid);
        if (w_newmodeid != w_currentmodeid) {
            wlOutputConfiguration->setMode(device, w_newmodeid);
        }

        // FIXME: scale
    }

    // We now block changes in order to compress events while the compositor is doing its thing
    // once it's done or failed, we'll trigger configChanged() only once, and not per individual
    // property change.
    connect(wlOutputConfiguration, &OutputConfiguration::applied, this, [this, wlOutputConfiguration] {
        wlOutputConfiguration->deleteLater();
        unblockSignals();
        Q_EMIT configChanged(toKScreenConfig());
    });
    connect(wlOutputConfiguration, &OutputConfiguration::failed, this, [this, wlOutputConfiguration] {
        wlOutputConfiguration->deleteLater();
        unblockSignals();
        Q_EMIT configChanged(toKScreenConfig());
    });
    blockSignals();
    // Now ask the compositor to apply the changes
    wlOutputConfiguration->apply();
}