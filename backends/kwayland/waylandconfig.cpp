/*
 *  SPDX-FileCopyrightText: 2014-2015 Sebastian Kügler <sebas@kde.org>
 *  SPDX-FileCopyrightText: 2013 Martin Gräßlin <mgraesslin@kde.org>
 *  SPDX-FileCopyrightText: 2021 Méven Car <meven.car@enioka.com>
 *
 *  SPDX-License-Identifier: LGPL-2.1-or-later
 */
#include "waylandconfig.h"
#include "config.h"
#include "kscreen_kwayland_logging.h"
#include "tabletmodemanager_interface.h"
#include "waylandbackend.h"
#include "waylandoutputdevice.h"
#include "waylandoutputmanagement.h"

#include <QGuiApplication>

#include <wayland-client-protocol.h>

using namespace KScreen;

WaylandConfig::WaylandConfig(QObject *parent)
    : QObject(parent)
    , m_outputManagement(std::make_unique<WaylandOutputManagement>(19))
    , m_config(new Config)
{
    connect(m_outputManagement.get(), &WaylandOutputManagement::activeChanged, this, &WaylandConfig::handleActiveChanged);
    initKWinTabletMode();
    setupRegistry();
}

WaylandConfig::~WaylandConfig()
{
    if (m_registry) {
        destroyRegistry();
    }
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
        if (!m_currentOperation) {
            Q_EMIT configChanged();
        }
    });
    connect(interface, &OrgKdeKWinTabletModeManagerInterface::tabletModeAvailableChanged, this, [this](bool available) {
        if (m_tabletModeAvailable == available) {
            return;
        }
        m_tabletModeAvailable = available;
        if (!m_currentOperation) {
            Q_EMIT configChanged();
        }
    });
}

void WaylandConfig::setupRegistry()
{
    auto waylandApp = qGuiApp->nativeInterface<QNativeInterface::QWaylandApplication>();
    if (!waylandApp) {
        return;
    }

    auto display = waylandApp->display();
    m_registry = wl_display_get_registry(display);

    auto globalAdded = [](void *data, wl_registry *registry, uint32_t name, const char *interface, uint32_t version) {
        Q_UNUSED(registry)
        auto self = static_cast<WaylandConfig *>(data);
        if (qstrcmp(interface, kde_output_device_v2_interface.name) == 0) {
            self->addOutput(name, std::min(20u, version));
        } else if (qstrcmp(interface, wl_fixes_interface.name) == 0) {
            self->m_fixes = static_cast<wl_fixes *>(wl_registry_bind(registry, name, &wl_fixes_interface, 1));
        }
    };

    auto globalRemoved = [](void *data, wl_registry *registry, uint32_t name) {
        Q_UNUSED(registry)
        auto self = static_cast<WaylandConfig *>(data);
        self->removeOutput(name);
    };

    static const wl_registry_listener registryListener{globalAdded, globalRemoved};
    wl_registry_add_listener(m_registry, &registryListener, this);

    wl_display_roundtrip(display); // list output devices
    wl_display_roundtrip(display); // get output device properties
}

void WaylandConfig::destroyRegistry()
{
    qDeleteAll(m_initializingOutputs);
    m_initializingOutputs.clear();

    qDeleteAll(m_outputMap);
    m_outputMap.clear();

    if (m_registry) {
        if (m_fixes) {
            wl_fixes_destroy_registry(m_fixes, m_registry);
        }
        wl_registry_destroy(m_registry);
        m_registry = nullptr;
    }

    if (m_fixes) {
        wl_fixes_destroy(m_fixes);
        m_fixes = nullptr;
    }
}

void WaylandConfig::handleActiveChanged()
{
    if (m_outputManagement->isActive()) {
        if (!m_registry) {
            setupRegistry();
        }
        return;
    }
    // the compositor went away, clean up all the Wayland resources
    if (!m_registry) {
        return;
    }

    destroyRegistry();

    if (!m_currentOperation) {
        Q_EMIT configChanged();
    }
}

void WaylandConfig::addOutput(quint32 name, quint32 version)
{
    qCDebug(KSCREEN_WAYLAND) << "adding output" << name;

    auto device = new WaylandOutputDevice(m_registry, name, version);
    m_initializingOutputs << device;

    // The done signal will be sent later after returning to the event loop.
    connect(device, &WaylandOutputDevice::done, this, [this, device]() {
        if (m_initializingOutputs.removeOne(device)) {
            m_outputMap.insert(device->id(), device);
        }

        if (!m_currentOperation) {
            Q_EMIT configChanged();
        }
    });
}

void WaylandConfig::removeOutput(quint32 name)
{
    for (qsizetype i = 0; i < m_initializingOutputs.size(); ++i) {
        WaylandOutputDevice *outputDevice = m_initializingOutputs[i];
        if (outputDevice->globalId() == name) {
            qCDebug(KSCREEN_WAYLAND) << "removing output" << name;
            m_initializingOutputs.removeAt(i);
            delete outputDevice;
            return;
        }
    }

    for (auto it = m_outputMap.begin(); it != m_outputMap.end(); ++it) {
        WaylandOutputDevice *outputDevice = *it;
        if (outputDevice->globalId() == name) {
            qCDebug(KSCREEN_WAYLAND) << "removing output" << name;
            m_outputMap.erase(it);
            delete outputDevice;

            if (!m_currentOperation) {
                Q_EMIT configChanged();
            }

            return;
        }
    }
}

bool WaylandConfig::isValid() const
{
    return m_outputManagement->isActive();
}

static QRect boundingRect(const QMap<int, WaylandOutputDevice *> &outputs)
{
    QRect r;
    for (const auto *out : outputs) {
        if (out->enabled()) {
            r |= QRect(out->globalPosition(), out->pixelSize() / out->scale());
        }
    }
    return r;
}

KScreen::ConfigPtr WaylandConfig::currentConfig()
{
    const auto features = Config::Feature::Writable | Config::Feature::PerOutputScaling | Config::Feature::AutoRotation | Config::Feature::TabletMode
        | Config::Feature::PrimaryDisplay | Config::Feature::XwaylandScales | Config::Feature::SynchronousOutputChanges | Config::Feature::OutputReplication;
    m_config->setSupportedFeatures(features);
    m_config->setValid(m_outputManagement->isActive());

    KScreen::ScreenPtr kscreenScreen(new KScreen::Screen);
    kscreenScreen->setMaxSize(QSize(64000, 64000)); // 64000^2 should be enough for everyone.
    kscreenScreen->setMinSize(QSize(0, 0));
    kscreenScreen->setCurrentSize(boundingRect(m_outputMap).size());
    kscreenScreen->setMaxActiveOutputsCount(m_outputMap.size());
    m_config->setScreen(kscreenScreen);

    // Removing removed outputs
    const KScreen::OutputList outputs = m_config->outputs();
    for (const auto &output : outputs) {
        if (!m_outputMap.contains(output->id())) {
            m_config->removeOutput(output->id());
        }
    }

    // Add KScreen::Outputs that aren't in the list yet
    QMap<OutputPtr, uint32_t> priorities;
    for (const auto &output : m_outputMap) {
        KScreen::OutputPtr kscreenOutput;
        if (m_config->outputs().contains(output->id())) {
            kscreenOutput = m_config->outputs()[output->id()];
            output->updateKScreenOutput(kscreenOutput, m_outputMap);
        } else {
            kscreenOutput = output->toKScreenOutput(m_outputMap);
            m_config->addOutput(kscreenOutput);
        }
        priorities[kscreenOutput] = output->index();
    }
    m_config->setOutputPriorities(priorities);

    m_config->setTabletModeAvailable(m_tabletModeAvailable);
    m_config->setTabletModeEngaged(m_tabletModeEngaged);

    return m_config;
}

QMap<int, WaylandOutputDevice *> WaylandConfig::outputMap() const
{
    return m_outputMap;
}

void WaylandConfig::tryPendingConfig()
{
    if (m_pendingOperation) {
        apply(std::move(m_pendingOperation));
    }
}

WaylandOutputDevice *WaylandConfig::findOutputDevice(struct ::kde_output_device_v2 *outputdevice) const
{
    for (WaylandOutputDevice *device : m_outputMap) {
        if (device->object() == outputdevice) {
            return device;
        }
    }
    return nullptr;
}

QFuture<SetConfigResult> WaylandConfig::applyConfig(const KScreen::ConfigPtr &newConfig)
{
    auto operation = std::make_unique<WaylandConfigApplyOperation>();
    operation->config = newConfig;
    operation->promise.start();

    return apply(std::move(operation));
}

QFuture<SetConfigResult> WaylandConfig::apply(std::unique_ptr<WaylandConfigApplyOperation> &&operation)
{
    for (const auto &output : operation->config->outputs()) {
        if (!m_outputMap.contains(output->id())) {
            qCWarning(KSCREEN_WAYLAND) << "Cannot find output with id" << output->id();
            return QtFuture::makeReadyFuture<SetConfigResult>(std::unexpected(QStringLiteral("Unknown output id: %1").arg(output->id())));
        }
    }

    operation->config->adjustPriorities(); // never trust input

    if (m_currentOperation) {
        // Last apply still pending, remember new changes and apply afterwards
        m_pendingOperation = std::move(operation);
        return m_pendingOperation->promise.future();
    }

    // Create a new configuration object
    operation->request = m_outputManagement->createConfiguration();
    if (!operation->request) {
        return QtFuture::makeReadyFuture<SetConfigResult>(std::unexpected(QStringLiteral("Failed to create kde_output_configuration_v2")));
    }

    bool changed = false;
    for (const auto &output : operation->config->outputs()) {
        changed |= m_outputMap[output->id()]->setWlConfig(m_outputManagement.get(), operation->request.get(), output, m_outputMap);
    }

    if (!changed) {
        return QtFuture::makeReadyFuture(SetConfigResult());
    }

    // We now block changes in order to compress events while the compositor is doing its thing
    // once it's done or failed, we'll trigger configChanged() only once, and not per individual
    // property change.
    connect(operation->request.get(), &WaylandOutputConfiguration::applied, this, [this] {
        m_currentOperation->promise.addResult(SetConfigResult());
        m_currentOperation->promise.finish();
        m_currentOperation.reset();
        Q_EMIT configChanged();
        tryPendingConfig();
    });
    connect(operation->request.get(), &WaylandOutputConfiguration::failed, this, [this](const QString &errorMessage) {
        m_currentOperation->promise.addResult(std::unexpected(errorMessage));
        m_currentOperation->promise.finish();
        m_currentOperation.reset();
        Q_EMIT configChanged();
        tryPendingConfig();
    });

    m_currentOperation = std::move(operation);
    m_currentOperation->request->apply();
    return m_currentOperation->promise.future();
}

#include "moc_waylandconfig.cpp"
