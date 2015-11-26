/*************************************************************************************
 *  Copyright (C) 2012 by Alejandro Fiestas Olivares <afiestas@kde.org>              *
 *  Copyright (C) 2012, 2013 by Daniel Vrátil <dvratil@redhat.com>                   *
 *  Copyright 2014 Sebastian Kügler <sebas@kde.org>                                  *
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

#include "waylandbackend.h"
#include "waylandconfig.h"
#include "waylandconfigwriter.h"
#include "waylandoutput.h"

#include <configmonitor.h>
#include <mode.h>

#include <QSettings>
#include <QStandardPaths>

using namespace KScreen;

Q_LOGGING_CATEGORY(KSCREEN_WAYLAND, "kscreen.wayland");

WaylandConfig* WaylandBackend::s_internalConfig = 0;

WaylandBackend::WaylandBackend()
    : KScreen::AbstractBackend()
    , m_isValid(true)
    , m_config(nullptr)
{
    QLoggingCategory::setFilterRules(QLatin1Literal("kscreen.wayland.debug = true"));

    if (s_internalConfig == 0) {
        qCDebug(KSCREEN_WAYLAND) << "Loading Wayland backend.";
        s_internalConfig = new WaylandConfig();
        m_config = internalConfig()->toKScreenConfig();
        connect(s_internalConfig, &WaylandConfig::configChanged,
                this, &WaylandBackend::emitConfigChanged);
    }
}

WaylandBackend::~WaylandBackend()
{
}

WaylandConfig* WaylandBackend::internalConfig()
{
    return s_internalConfig;
}

QString WaylandBackend::name() const
{
    return QString("kwayland");
}

QString WaylandBackend::serviceName() const
{
    return QLatin1Literal("org.kde.KScreen.Backend.Wayland");
}

ConfigPtr WaylandBackend::config() const
{
    // Note: This should ONLY be called from GetConfigOperation!
    return internalConfig()->toKScreenConfig();
}

void WaylandBackend::setConfig(const KScreen::ConfigPtr &newconfig)
{
    if (!newconfig) {
        return;
    }
    internalConfig()->applyConfig(newconfig);
}

void WaylandBackend::emitConfigChanged(const KScreen::ConfigPtr cfg)
{
    Q_EMIT configChanged(m_config);
}


// Edid *WaylandBackend::edid(int outputId) const
// {
//     WaylandOutput *output = internalConfig()->outputMap().value(outputId);
//     if (!output) {
//         return 0;
//     }
//     return output->edid();
//
//     return 0;
// }

bool WaylandBackend::isValid() const
{
    return m_isValid;
}

void WaylandBackend::updateConfig(ConfigPtr &config)
{
    qDebug() << "Update config";
    Q_ASSERT(config != 0);
    internalConfig()->updateKScreenConfig(config);
}


