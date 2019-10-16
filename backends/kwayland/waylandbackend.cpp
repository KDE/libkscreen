/*************************************************************************************
 *  Copyright (C) 2012 by Alejandro Fiestas Olivares <afiestas@kde.org>              *
 *  Copyright (C) 2012, 2013 by Daniel Vrátil <dvratil@redhat.com>                   *
 *  Copyright 2014-2015 Sebastian Kügler <sebas@kde.org>                             *
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
#include "waylandoutput.h"

#include <configmonitor.h>
#include <mode.h>

#include <QSettings>
#include <QStandardPaths>

using namespace KScreen;

Q_LOGGING_CATEGORY(KSCREEN_WAYLAND, "kscreen.kwayland")


WaylandBackend::WaylandBackend()
    : KScreen::AbstractBackend()
    , m_isValid(true)
    , m_internalConfig(new WaylandConfig(this))
{
    qCDebug(KSCREEN_WAYLAND) << "Loading Wayland backend.";

    connect(m_internalConfig, &WaylandConfig::configChanged,
            this, [this]() {
        Q_EMIT configChanged(m_internalConfig->currentConfig());
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
    m_internalConfig->applyConfig(newconfig);
}

QByteArray WaylandBackend::edid(int outputId) const
{
    WaylandOutput *output = m_internalConfig->outputMap().value(outputId);
    if (!output) {
        return QByteArray();
    }
    return output->outputDevice()->edid();
}

bool WaylandBackend::isValid() const
{
    return m_isValid;
}
