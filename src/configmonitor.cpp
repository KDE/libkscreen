/*************************************************************************************
 *  Copyright 2012 - 2014  Daniel Vrátil <dvratil@redhat.com>                        *
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

    QList<QWeakPointer<KScreen::Config>>  watchedConfigs;
    AbstractBackend* backend;
    ConfigMonitor *m_q;
};

void ConfigMonitor::Private::updateConfigs()
{
    QMutableListIterator<QWeakPointer<Config>> iter(watchedConfigs);
    while (iter.hasNext()) {
        KScreen::ConfigPtr config = iter.next().toStrongRef();
        backend->updateConfig(config);
        iter.setValue(config.toWeakRef());
    }
}

void ConfigMonitor::Private::_k_configurationDestroyed(QObject *removedConfig)
{
    for (auto iter = watchedConfigs.begin(); iter != watchedConfigs.end(); ++iter) {
        if (iter->data() == removedConfig) {
            watchedConfigs.erase(iter);
            break;
        }
    }
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

void ConfigMonitor::addConfig(const ConfigPtr &config)
{
    const QWeakPointer<Config> weakConfig = config.toWeakRef();
    if (!d->watchedConfigs.contains(weakConfig)) {
        connect(weakConfig.data(), SIGNAL(destroyed(QObject*)), SLOT(_k_configurationDestroyed(QObject*)));
        d->watchedConfigs << weakConfig;
    }
}

void ConfigMonitor::removeConfig(const ConfigPtr &config)
{
    const QWeakPointer<Config> weakConfig = config.toWeakRef();
    if (d->watchedConfigs.contains(config)) {
        disconnect(config.data(), SIGNAL(destroyed(QObject*)), this, SLOT(_k_configurationDestroyed(QObject*)));
        d->watchedConfigs.removeAll(config);
    }
}

void ConfigMonitor::notifyUpdate()
{
    d->updateConfigs();

    Q_EMIT configurationChanged();
}

#include "moc_configmonitor.cpp"
