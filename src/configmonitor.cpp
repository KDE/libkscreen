/*
 *  SPDX-FileCopyrightText: 2012-2014 Daniel Vrátil <dvratil@redhat.com>
 *
 *  SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "configmonitor.h"
#include "abstractbackend.h"
#include "backendmanager_p.h"
#include "config.h"
#include "kscreen_debug.h"

using namespace KScreen;

class Q_DECL_HIDDEN ConfigMonitor::Private : public QObject
{
    Q_OBJECT

public:
    Private(ConfigMonitor *q);

    void configDestroyed(QObject *removedConfig);
    void updateConfigs(const KScreen::ConfigPtr &newConfig);

    QList<QWeakPointer<KScreen::Config>> watchedConfigs;

private:
    ConfigMonitor *q;
};

ConfigMonitor::Private::Private(ConfigMonitor *q)
    : QObject(q)
    , q(q)
{
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
    for (auto iter = watchedConfigs.begin(); iter != watchedConfigs.end();) {
        if (iter->toStrongRef() == removedConfig) {
            iter = watchedConfigs.erase(iter);
            // Iterate over the entire list in case there are duplicates
        } else {
            ++iter;
        }
    }
}

ConfigMonitor *ConfigMonitor::instance()
{
    static ConfigMonitor *s_instance = nullptr;

    if (s_instance == nullptr) {
        s_instance = new ConfigMonitor();
    }

    return s_instance;
}

ConfigMonitor::ConfigMonitor()
    : QObject()
    , d(new Private(this))
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
        connect(weakConfig.toStrongRef().data(), &QObject::destroyed, d, &Private::configDestroyed);
        d->watchedConfigs << weakConfig;
    }
}

void ConfigMonitor::removeConfig(const ConfigPtr &config)
{
    const QWeakPointer<Config> weakConfig = config.toWeakRef();
    if (d->watchedConfigs.contains(config)) {
        disconnect(weakConfig.toStrongRef().data(), &QObject::destroyed, d, &Private::configDestroyed);
        d->watchedConfigs.removeAll(config);
    }
}

void ConfigMonitor::connectInProcessBackend(KScreen::AbstractBackend *backend)
{
    connect(backend, &AbstractBackend::configChanged, [this](KScreen::ConfigPtr config) {
        if (config.isNull()) {
            return;
        }
        qCDebug(KSCREEN) << "Backend change!" << config;
        d->updateConfigs(config);
    });
}

#include "configmonitor.moc"

#include "moc_configmonitor.cpp"
