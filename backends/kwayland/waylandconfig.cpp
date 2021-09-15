/*
 *  SPDX-FileCopyrightText: 2014-2015 Sebastian Kügler <sebas@kde.org>
 *  SPDX-FileCopyrightText: 2013 Martin Gräßlin <mgraesslin@kde.org>
 *  SPDX-FileCopyrightText: 2021 Méven Car <meven.car@enioka.com>
 *
 *  SPDX-License-Identifier: LGPL-2.1-or-later
 */
#include "waylandconfig.h"

#include "kscreen_kwayland_logging.h"

#include "waylandbackend.h"
#include "waylandoutputdevice.h"
#include "waylandoutputmanagement.h"
#include "waylandscreen.h"

#include "output.h"

#include "tabletmodemanager_interface.h"

#include <QThread>
#include <configmonitor.h>
#include <mode.h>

#include <KWayland/Client/connection_thread.h>
#include <KWayland/Client/event_queue.h>
#include <KWayland/Client/registry.h>

#include <QTimer>

using namespace KScreen;

WaylandConfig::WaylandConfig(QObject *parent)
    : QObject(parent)
    , m_outputManagement(nullptr)
    , m_registryInitialized(false)
    , m_blockSignals(true)
    , m_kscreenConfig(new Config)
    , m_kscreenPendingConfig(nullptr)
    , m_screen(new WaylandScreen(this))
    , m_tabletModeAvailable(false)
    , m_tabletModeEngaged(false)
{
    initKWinTabletMode();

    connect(this, &WaylandConfig::initialized, &m_syncLoop, &QEventLoop::quit);
    QTimer::singleShot(3000, this, [this] {
        if (m_syncLoop.isRunning()) {
            qCWarning(KSCREEN_WAYLAND) << "Connection to Wayland server timed out.";
            m_syncLoop.quit();
        }
    });

    initConnection();
    m_syncLoop.exec();
}

WaylandConfig::~WaylandConfig()
{
    m_syncLoop.quit();
}

void WaylandConfig::initKWinTabletMode()
{
    auto *interface =
        new OrgKdeKWinTabletModeManagerInterface(QStringLiteral("org.kde.KWin"), QStringLiteral("/org/kde/KWin"), QDBusConnection::sessionBus(), this);
    if (!interface->isValid()) {
        m_tabletModeAvailable = false;
        m_tabletModeEngaged = false;
        return;
    }

    m_tabletModeAvailable = interface->tabletModeAvailable();
    m_tabletModeEngaged = interface->tabletMode();

    connect(interface, &OrgKdeKWinTabletModeManagerInterface::tabletModeChanged, this, [this](bool tabletMode) {
        if (m_tabletModeEngaged == tabletMode) {
            return;
        }
        m_tabletModeEngaged = tabletMode;
        if (!m_blockSignals && m_initializingOutputs.empty()) {
            Q_EMIT configChanged();
        }
    });
    connect(interface, &OrgKdeKWinTabletModeManagerInterface::tabletModeAvailableChanged, this, [this](bool available) {
        if (m_tabletModeAvailable == available) {
            return;
        }
        m_tabletModeAvailable = available;
        if (!m_blockSignals && m_initializingOutputs.empty()) {
            Q_EMIT configChanged();
        }
    });
}

void WaylandConfig::initConnection()
{
    m_connection = KWayland::Client::ConnectionThread::fromApplication(this);
    setupRegistry();
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

void WaylandConfig::setupRegistry()
{
    if (!m_connection) {
        return;
    }

    m_registry = new KWayland::Client::Registry(this);

    connect(m_registry, &KWayland::Client::Registry::interfaceAnnounced, this, [this](const QByteArray &interface, quint32 name, quint32 version) {
        if (interface == WaylandOutputDevice::interface()->name) {
            addOutput(name, version);
        }
        if (interface == WaylandOutputManagement::interface()->name) {
            m_outputManagement = new WaylandOutputManagement(m_registry->registry(), name, version);
        }
    });

    connect(m_registry, &KWayland::Client::Registry::interfacesAnnounced, this, [this] {
        m_registryInitialized = true;
        unblockSignals();
        checkInitialized();
    });

    m_registry->create(m_connection);
    m_registry->setup();
}

int s_outputId = 0;

void WaylandConfig::addOutput(quint32 name, quint32 version)
{
    qCDebug(KSCREEN_WAYLAND) << "adding output" << name;

    auto device = new WaylandOutputDevice(++s_outputId);
    m_initializingOutputs << device;

    connect(m_registry, &KWayland::Client::Registry::interfaceRemoved, this, [name, device, this](const quint32 &interfaceName) {
        if (name == interfaceName) {
            removeOutput(device);
        }
    });

    QMetaObject::Connection *const connection = new QMetaObject::Connection;
    *connection = connect(device, &WaylandOutputDevice::done, this, [this, connection, device]() {
        QObject::disconnect(*connection);
        delete connection;

        m_initializingOutputs.removeOne(device);
        m_outputMap.insert(device->id(), device);
        checkInitialized();

        if (!m_blockSignals && m_initializingOutputs.isEmpty()) {
            m_screen->setOutputs(m_outputMap.values());
            Q_EMIT configChanged();
        }

        connect(device, &WaylandOutputDevice::done, this, [this]() {
            // output got update must update current config
            if (!m_blockSignals) {
                Q_EMIT configChanged();
            }
        });
    });

    device->init(*m_registry, name, version);
}

void WaylandConfig::removeOutput(WaylandOutputDevice *output)
{
    qCDebug(KSCREEN_WAYLAND) << "removing output" << output->name();

    if (m_initializingOutputs.removeOne(output)) {
        // output was not yet fully initialized, just remove here and return
        delete output;
        return;
    }

    // remove the output from output mapping
    const auto removedOutput = m_outputMap.take(output->id());
    Q_ASSERT(removedOutput == output);
    Q_UNUSED(removedOutput);
    m_screen->setOutputs(m_outputMap.values());
    delete output;

    if (!m_blockSignals) {
        Q_EMIT configChanged();
    }
}

bool WaylandConfig::isReady() const
{
    // clang-format off
    return !m_blockSignals
            && m_registryInitialized
            && m_initializingOutputs.isEmpty()
            && m_outputMap.count() > 0
            && m_outputManagement != nullptr;
    // clang-format on
}

void WaylandConfig::checkInitialized()
{
    if (!m_initialized && isReady()) {
        m_initialized = true;
        m_screen->setOutputs(m_outputMap.values());
        Q_EMIT initialized();
    }
}

KScreen::ConfigPtr WaylandConfig::currentConfig()
{
    m_kscreenConfig->setScreen(m_screen->toKScreenScreen(m_kscreenConfig));

    const auto features = Config::Feature::Writable | Config::Feature::PerOutputScaling | Config::Feature::AutoRotation | Config::Feature::TabletMode;
    m_kscreenConfig->setSupportedFeatures(features);
    m_kscreenConfig->setValid(m_connection->display());

    KScreen::ScreenPtr screen = m_kscreenConfig->screen();
    m_screen->updateKScreenScreen(screen);

    // Removing removed outputs
    const KScreen::OutputList outputs = m_kscreenConfig->outputs();
    for (const auto &output : outputs) {
        if (!m_outputMap.contains(output->id())) {
            m_kscreenConfig->removeOutput(output->id());
        }
    }

    // Add KScreen::Outputs that aren't in the list yet, handle primaryOutput
    KScreen::OutputList kscreenOutputs = m_kscreenConfig->outputs();
    for (const auto &output : m_outputMap) {
        KScreen::OutputPtr kscreenOutput = kscreenOutputs[output->id()];
        if (!kscreenOutput) {
            kscreenOutput = output->toKScreenOutput();
            kscreenOutputs.insert(kscreenOutput->id(), kscreenOutput);
        }
        if (kscreenOutput && m_outputMap.count() == 1) {
            kscreenOutput->setPrimary(true);
        } else if (m_outputMap.count() > 1) {
            // primaryScreen concept doesn't exist in kwayland, so we don't set one
        }
        output->updateKScreenOutput(kscreenOutput);
    }
    m_kscreenConfig->setOutputs(kscreenOutputs);

    m_kscreenConfig->setTabletModeAvailable(m_tabletModeAvailable);
    m_kscreenConfig->setTabletModeEngaged(m_tabletModeEngaged);

    return m_kscreenConfig;
}

QMap<int, WaylandOutputDevice *> WaylandConfig::outputMap() const
{
    return m_outputMap;
}

void WaylandConfig::tryPendingConfig()
{
    if (!m_kscreenPendingConfig) {
        return;
    }
    applyConfig(m_kscreenPendingConfig);
    m_kscreenPendingConfig = nullptr;
}

void WaylandConfig::applyConfig(const KScreen::ConfigPtr &newConfig)
{
    using namespace KWayland::Client;
    // Create a new configuration object
    auto wlConfig = m_outputManagement->createConfiguration();
    bool changed = false;

    if (m_blockSignals) {
        // Last apply still pending, remember new changes and apply afterwards
        m_kscreenPendingConfig = newConfig;
        return;
    }

    for (const auto &output : newConfig->outputs()) {
        changed |= m_outputMap[output->id()]->setWlConfig(wlConfig, output);
    }

    if (!changed) {
        return;
    }

    // We now block changes in order to compress events while the compositor is doing its thing
    // once it's done or failed, we'll trigger configChanged() only once, and not per individual
    // property change.
    connect(wlConfig, &WaylandOutputConfiguration::applied, this, [this, wlConfig] {
        wlConfig->deleteLater();
        unblockSignals();
        Q_EMIT configChanged();
        tryPendingConfig();
    });
    connect(wlConfig, &WaylandOutputConfiguration::failed, this, [this, wlConfig] {
        wlConfig->deleteLater();
        unblockSignals();
        Q_EMIT configChanged();
        tryPendingConfig();
    });

    // Now block signals and ask the compositor to apply the changes.
    blockSignals();
    wlConfig->apply();
}
