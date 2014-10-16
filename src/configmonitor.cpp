/*************************************************************************************
 *  Copyright 2012, 2013  Daniel Vrátil <dvratil@redhat.com>                         *
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

#include "configmonitor.h"
#include "backendloader.h"
#include "backends/abstractbackend.h"

using namespace KScreen;


class ConfigMonitor::Private
{
  public:
    Private(ConfigMonitor *q)
     : backend(BackendLoader::backend())
     , m_q(q)
    { }

    void updateConfigs();
    void _k_configurationDestroyed(QObject* removedConfig);

    QList< KScreen::Config* >  watchedConfigs;
    AbstractBackend* backend;
    ConfigMonitor *m_q;
};

void ConfigMonitor::Private::updateConfigs()
{
    Q_FOREACH( Config *config, watchedConfigs) {
        if (config) {
            backend->updateConfig(config);
        }
    }
}

void ConfigMonitor::Private::_k_configurationDestroyed(QObject *removedConfig)
{
    m_q->removeConfig(static_cast<Config*>(removedConfig));
}

ConfigMonitor *ConfigMonitor::instance()
{
    static ConfigMonitor *s_instance;

    if (s_instance == 0) {
        s_instance = new ConfigMonitor();
    }

    return s_instance;
}

ConfigMonitor::ConfigMonitor():
    QObject(),
    d(new Private(this))
{
}

ConfigMonitor::~ConfigMonitor()
{
    delete d;
}

void ConfigMonitor::addConfig(Config *config)
{
    if (!d->watchedConfigs.contains(config)) {
        connect(config, SIGNAL(destroyed(QObject*)), SLOT(_k_configurationDestroyed(QObject*)));
        d->watchedConfigs << config;
    }
}

void ConfigMonitor::removeConfig(Config *config)
{
    if (d->watchedConfigs.contains(config)) {
        disconnect(config, SIGNAL(destroyed(QObject*)), this, SLOT(_k_configurationDestroyed(QObject*)));
        d->watchedConfigs.removeAll(config);
    }
}

void ConfigMonitor::notifyUpdate()
{
    d->updateConfigs();
    qDebug() << "ConfigMonitor::configurationChanged()!";

    Q_EMIT configurationChanged();
}


#include "moc_configmonitor.cpp"
