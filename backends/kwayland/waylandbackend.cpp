/*
 *  SPDX-FileCopyrightText: 2012 Alejandro Fiestas Olivares <afiestas@kde.org>
 *  SPDX-FileCopyrightText: 2012, 2013 Daniel Vrátil <dvratil@redhat.com>
 *  SPDX-FileCopyrightText: 2014-2015 Sebastian Kügler <sebas@kde.org>
 *  SPDX-FileCopyrightText: 2021 Méven Car <meven.car@enioka.com>
 *
 *  SPDX-License-Identifier: LGPL-2.1-or-later
 */
#include "waylandbackend.h"

#include "waylandconfig.h"
#include "waylandoutputdevice.h"

#include "kscreen_kwayland_logging.h"

#include <configmonitor.h>
#include <mode.h>
#include <output.h>

#include <QProcess>
#include <QSettings>
#include <QStandardPaths>

#include <KConfig>
#include <KConfigGroup>

using namespace KScreen;

WaylandBackend::WaylandBackend()
    : KScreen::AbstractBackend()
    , m_internalConfig(new WaylandConfig(this))
{
    qCDebug(KSCREEN_WAYLAND) << "Loading Wayland backend.";

    connect(m_internalConfig, &WaylandConfig::configChanged, this, [this]() {
        const auto newConfig = m_internalConfig->currentConfig();

        KConfig cfg(QStringLiteral("kdeglobals"));

        KConfigGroup kscreenGroup = cfg.group("KScreen");
        const bool xwaylandClientsScale = kscreenGroup.readEntry("XwaylandClientsScale", true);

        KConfig kwinCfg(QStringLiteral("kwinrc"));
        KConfigGroup xwaylandGroup = kwinCfg.group("Xwayland");
        if (xwaylandClientsScale) {
            qreal scaleFactor = 1;
            const auto outputs = newConfig->outputs();
            for (auto output : outputs) {
                if (output->isEnabled()) {
                    scaleFactor = std::max(scaleFactor, output->scale());
                }
            }

            xwaylandGroup.writeEntry("Scale", scaleFactor, KConfig::Notify);

        } else {
            xwaylandGroup.deleteEntry("Scale", KConfig::Notify);
        }
        // here we rerun the fonts kcm init that does the appropriate xrdb call with the new settings
        QProcess::startDetached("kcminit", {"kcm_fonts", "kcm_style"});

        Q_EMIT configChanged(newConfig);
    });
}

QString WaylandBackend::name() const
{
    return QStringLiteral("kwayland");
}

QString WaylandBackend::serviceName() const
{
    return QStringLiteral("org.kde.KScreen.Backend.KWayland");
}

ConfigPtr WaylandBackend::config() const
{
    // Note: This should ONLY be called from GetConfigOperation!
    return m_internalConfig->currentConfig();
}

void WaylandBackend::setConfig(const KScreen::ConfigPtr &newconfig)
{
    if (!newconfig) {
        return;
    }
    // wait for KWin reply
    QEventLoop loop;

    connect(m_internalConfig, &WaylandConfig::configChanged, &loop, &QEventLoop::quit);
    m_internalConfig->applyConfig(newconfig);

    loop.exec();
}

QByteArray WaylandBackend::edid(int outputId) const
{
    WaylandOutputDevice *output = m_internalConfig->outputMap().value(outputId);
    if (!output) {
        return QByteArray();
    }
    return output->edid();
}

bool WaylandBackend::isValid() const
{
    return m_internalConfig->isReady();
}
