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

#include "tabletmodemanager_interface.h"

#include <QGuiApplication>
#include <QThread>
#include <QTimer>
#include <configmonitor.h>
#include <mode.h>
#include <output.h>

#include <wayland-client-protocol.h>

using namespace KScreen;

WaylandConfig::WaylandConfig(QObject *parent)
    : QObject(parent)
    , m_outputManagement(std::make_unique<WaylandOutputManagement>(20))
    , m_outputRegistry(std::make_unique<WaylandOutputDeviceRegistry>())
    , m_blockSignals(false)
    , m_kscreenConfig(new Config)
    , m_kscreenPendingConfig(nullptr)
    , m_screen(new WaylandScreen(this))
    , m_tabletModeAvailable(false)
    , m_tabletModeEngaged(false)
{
    connect(m_outputManagement.get(), &WaylandOutputManagement::activeChanged, this, &WaylandConfig::handleActiveChanged);
    connect(m_outputRegistry.get(), &WaylandOutputDeviceRegistry::outputAdded, this, &WaylandConfig::addOutput);
    connect(m_outputRegistry.get(), &WaylandOutputDeviceRegistry::outputRemoved, this, &WaylandConfig::removeOutput);

    initKWinTabletMode();

    if (auto waylandApp = qGuiApp->nativeInterface<QNativeInterface::QWaylandApplication>()) {
        auto display = waylandApp->display();
        wl_display_roundtrip(display); // list output devices
        wl_display_roundtrip(display); // get output device properties
    }
}

WaylandConfig::~WaylandConfig()
{
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
        if (!m_blockSignals) {
            Q_EMIT configChanged();
        }
    });
    connect(interface, &OrgKdeKWinTabletModeManagerInterface::tabletModeAvailableChanged, this, [this](bool available) {
        if (m_tabletModeAvailable == available) {
            return;
        }
        m_tabletModeAvailable = available;
        if (!m_blockSignals) {
            Q_EMIT configChanged();
        }
    });
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

void WaylandConfig::handleActiveChanged()
{
    if (m_outputManagement->isActive()) {
        return;
    }

    // the compositor went away, clean up all the resources
    m_initializingOutputs.clear();
    m_screen->setOutputs({});

    if (!m_blockSignals) {
        Q_EMIT configChanged();
    }
}

void WaylandConfig::addOutput(WaylandOutputDevice *output)
{
    qCDebug(KSCREEN_WAYLAND) << "adding output" << output;

    m_initializingOutputs << output;
    connect(output, &WaylandOutputDevice::done, this, [this, output]() {
        if (m_initializingOutputs.removeOne(output)) {
            m_outputMap.insert(output->id(), output);
            m_screen->setOutputs(m_outputMap.values());
        }

        if (!m_blockSignals) {
            Q_EMIT configChanged();
        }
    });
}

void WaylandConfig::removeOutput(WaylandOutputDevice *output)
{
    qCDebug(KSCREEN_WAYLAND) << "removing output" << output->name();

    if (m_initializingOutputs.removeOne(output)) {
        return;
    }

    const auto removedOutput = m_outputMap.take(output->id());
    Q_ASSERT(removedOutput == output);
    Q_UNUSED(removedOutput);
    m_screen->setOutputs(m_outputMap.values());

    if (!m_blockSignals) {
        Q_EMIT configChanged();
    }
}

bool WaylandConfig::isValid() const
{
    return m_outputManagement->isActive() && m_outputRegistry->isActive();
}

KScreen::ConfigPtr WaylandConfig::currentConfig()
{
    m_kscreenConfig->setScreen(m_screen->toKScreenScreen(m_kscreenConfig));

    const auto features = Config::Feature::Writable | Config::Feature::PerOutputScaling | Config::Feature::AutoRotation | Config::Feature::TabletMode
        | Config::Feature::PrimaryDisplay | Config::Feature::XwaylandScales | Config::Feature::SynchronousOutputChanges | Config::Feature::OutputReplication;
    m_kscreenConfig->setSupportedFeatures(features);
    m_kscreenConfig->setValid(m_outputManagement->isActive());

    KScreen::ScreenPtr screen = m_kscreenConfig->screen();
    m_screen->updateKScreenScreen(screen);

    // Removing removed outputs
    const KScreen::OutputList outputs = m_kscreenConfig->outputs();
    for (const auto &output : outputs) {
        if (!m_outputMap.contains(output->id())) {
            m_kscreenConfig->removeOutput(output->id());
        }
    }

    // Add KScreen::Outputs that aren't in the list yet
    for (const auto &output : m_outputMap) {
        if (m_kscreenConfig->outputs().contains(output->id())) {
            output->updateKScreenOutput(m_kscreenConfig->outputs()[output->id()], m_outputMap);
        } else {
            m_kscreenConfig->addOutput(output->toKScreenOutput(m_outputMap));
        }
    }

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

bool WaylandConfig::applyConfig(const KScreen::ConfigPtr &newConfig)
{
    for (const auto &output : newConfig->outputs()) {
        if (!m_outputMap.contains(output->id())) {
            qCWarning(KSCREEN_WAYLAND) << "Cannot find output with id" << output->id();
            return false;
        }
    }

    newConfig->adjustPriorities(); // never trust input
    if (m_blockSignals) {
        // Last apply still pending, remember new changes and apply afterwards
        m_kscreenPendingConfig = newConfig;
        return true;
    }

    // Create a new configuration object
    auto wlConfig = m_outputManagement->createConfiguration();
    if (!wlConfig) {
        return false;
    }
    bool changed = false;

    for (const auto &output : newConfig->outputs()) {
        changed |= m_outputMap[output->id()]->setWlConfig(m_outputManagement.get(), wlConfig, output, m_outputMap);
    }

    if (!changed) {
        delete wlConfig;
        return false;
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
    connect(wlConfig, &WaylandOutputConfiguration::failed, this, [this, wlConfig](const QString &errorMessage) {
        wlConfig->deleteLater();
        unblockSignals();
        Q_EMIT configFailed(errorMessage);
        Q_EMIT configChanged();
        tryPendingConfig();
    });

    // Now block signals and ask the compositor to apply the changes.
    blockSignals();
    wlConfig->apply();
    return true;
}

#include "moc_waylandconfig.cpp"
