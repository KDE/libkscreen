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
#include "backendmanager_p.h"
#include "backendinterface.h"
#include "abstractbackend.h"
#include "configserializer_p.h"
#include "getconfigoperation.h"
#include "debug_p.h"

using namespace KScreen;


class ConfigMonitor::Private : public QObject
{
      Q_OBJECT

public:
    Private(ConfigMonitor *q);

    void updateConfigs();
    void onBackendReady(org::kde::kscreen::Backend *backend);
    void backendConfigChanged(const QVariantMap &config);
    void configDestroyed(QObject* removedConfig);
    void getConfigFinished(ConfigOperation *op);
    void updateConfigs(const KScreen::ConfigPtr &newConfig);

    QList<QWeakPointer<KScreen::Config>>  watchedConfigs;

    QPointer<org::kde::kscreen::Backend> mBackend;

private:
    ConfigMonitor *q;
};

ConfigMonitor::Private::Private(ConfigMonitor *q)
    : QObject(q)
    , q(q)
{
}

void ConfigMonitor::Private::onBackendReady(org::kde::kscreen::Backend *backend)
{
    if (backend == mBackend) {
        return;
    }

    if (mBackend) {
        disconnect(mBackend.data(), &org::kde::kscreen::Backend::configChanged,
                   this, &ConfigMonitor::Private::backendConfigChanged);
    }

    mBackend = QPointer<org::kde::kscreen::Backend>(backend);
    connect(mBackend.data(), &org::kde::kscreen::Backend::configChanged,
            this, &ConfigMonitor::Private::backendConfigChanged);

    // If we received a new backend interface, then it's very likely that it is
    // because the backend process has crashed - just to be sure we haven't missed
    // any change, request the current config now and update our watched configs
    if (!watchedConfigs.isEmpty()) {
        connect(new GetConfigOperation(), &GetConfigOperation::finished,
                this, &Private::getConfigFinished);
    }
}

void ConfigMonitor::Private::getConfigFinished(ConfigOperation* op)
{
    if (op->hasError()) {
        qCWarning(KSCREEN) << "Failed to retrieve current config: " << op->errorString();
        return;
    }

    const KScreen::ConfigPtr config = qobject_cast<GetConfigOperation*>(op)->config();
    updateConfigs(config);
}


void ConfigMonitor::Private::backendConfigChanged(const QVariantMap &configMap)
{
    const ConfigPtr newConfig = ConfigSerializer::deserializeConfig(configMap);
    if (!newConfig) {
        qCWarning(KSCREEN) << "Failed to deserialize config from DBus change notification";
        return;
    }

    updateConfigs(newConfig);
}

void ConfigMonitor::Private::updateConfigs(const KScreen::ConfigPtr &newConfig)
{
    QMutableListIterator<QWeakPointer<Config>> iter(watchedConfigs);
    while (iter.hasNext()) {
        KScreen::ConfigPtr config = iter.next().toStrongRef();
        if (!config) {
            iter.remove();
            continue;
        }

        config->apply(newConfig);
        iter.setValue(config.toWeakRef());
    }

    Q_EMIT q->configurationChanged();
}

void ConfigMonitor::Private::configDestroyed(QObject *removedConfig)
{
    for (auto iter = watchedConfigs.begin(); iter != watchedConfigs.end(); ++iter) {
        if (iter->data() == removedConfig) {
            iter = watchedConfigs.erase(iter);
            // Iterate over the entire list in case there are duplicates
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
    connect(BackendManager::instance(), &BackendManager::backendReady,
            d, &ConfigMonitor::Private::onBackendReady);
    BackendManager::instance()->requestBackend();
}

ConfigMonitor::~ConfigMonitor()
{
}

void ConfigMonitor::addConfig(const ConfigPtr &config)
{
    const QWeakPointer<Config> weakConfig = config.toWeakRef();
    if (!d->watchedConfigs.contains(weakConfig)) {
        connect(weakConfig.data(), &QObject::destroyed,
                d, &Private::configDestroyed);
        d->watchedConfigs << weakConfig;
    }
}

void ConfigMonitor::removeConfig(const ConfigPtr &config)
{
    const QWeakPointer<Config> weakConfig = config.toWeakRef();
    if (d->watchedConfigs.contains(config)) {
        disconnect(weakConfig.data(), &QObject::destroyed,
                   d, &Private::configDestroyed);
        d->watchedConfigs.removeAll(config);
    }
}

#include "configmonitor.moc"
