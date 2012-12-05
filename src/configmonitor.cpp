/*
 * Copyright 2012  Dan Vratil <dvratil@redhat.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "configmonitor.h"
#include "backendloader.h"
#include "backends/abstractbackend.h"

#include <QDebug>

KScreen::ConfigMonitor* KScreen::ConfigMonitor::s_instance = 0;

using namespace KScreen;

ConfigMonitor *ConfigMonitor::instance()
{
    if (s_instance == 0) {
        s_instance = new ConfigMonitor();
    }

    return s_instance;
}

ConfigMonitor::ConfigMonitor():
    QObject(),
    m_backend(BackendLoader::backend())
{
}

ConfigMonitor::~ConfigMonitor()
{
}

void ConfigMonitor::addConfig(Config *config)
{
    if (!m_watchedConfigs.contains(QPointer<Config>(config))) {
        qDebug() << "Registered config" << config;
        m_watchedConfigs << QPointer<Config>(config);
    }
}

void ConfigMonitor::removeConfig(Config *config)
{
    if (m_watchedConfigs.contains(QPointer<Config>(config))) {
        m_watchedConfigs.removeAll(QPointer<Config>(config));
    }
}

void ConfigMonitor::notifyUpdate()
{
    updateConfigs();
}


void ConfigMonitor::updateConfigs()
{
    Q_FOREACH(QPointer<Config> config, m_watchedConfigs) {
        if (config) {
            m_backend->updateConfig(config);
        }
    }
}



#include "configmonitor.moc"
