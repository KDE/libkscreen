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
#include "output.h"

#include <QDBusPendingCallWatcher>

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
    void edidReady(QDBusPendingCallWatcher *watcher);

    QList<QWeakPointer<KScreen::Config>>  watchedConfigs;

    QPointer<org::kde::kscreen::Backend> mBackend;

    QSet<int> mPendingEDIDRequests;
    KScreen::ConfigPtr mPendingConfigUpdate;
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
    ConfigPtr newConfig = ConfigSerializer::deserializeConfig(configMap);
    if (!newConfig) {
        qCWarning(KSCREEN) << "Failed to deserialize config from DBus change notification";
        return;
    }

    Q_FOREACH (OutputPtr output, newConfig->connectedOutputs()) {
        if (!output->edid() && output->isConnected()) {
            QDBusPendingReply<QByteArray> reply = mBackend->getEdid(output->id());
            mPendingEDIDRequests.insert(output->id());
            QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(reply);
            watcher->setProperty("outputId", output->id());
            connect(watcher, &QDBusPendingCallWatcher::finished,
                    this, &ConfigMonitor::Private::edidReady);
        }
    }

    if (!mPendingEDIDRequests.isEmpty()) {
        qCDebug(KSCREEN) << "Requesting missing EDID for outputs" << mPendingEDIDRequests;
        mPendingConfigUpdate = newConfig;
    } else {
        updateConfigs(newConfig);
    }
}

void ConfigMonitor::Private::edidReady(QDBusPendingCallWatcher* watcher)
{
    const int outputId = watcher->property("outputId").toInt();
    Q_ASSERT(!mPendingConfigUpdate.isNull());
    Q_ASSERT(mPendingEDIDRequests.contains(outputId));

    watcher->deleteLater();

    mPendingEDIDRequests.remove(watcher->property("outputId").toInt());

    const QDBusPendingReply<QByteArray> reply = *watcher;
    if (reply.isError()) {
        qCWarning(KSCREEN) << "Error when retrieving EDID: " << reply.error().message();
        if (mPendingEDIDRequests.isEmpty()) {
            updateConfigs(mPendingConfigUpdate);
        }
        return;
    }

    const QByteArray edid = reply.argumentAt<0>();
    if (!edid.isEmpty()) {
        qCDebug(KSCREEN) << "Received valid EDID for output" << outputId;
        OutputPtr output = mPendingConfigUpdate->output(outputId);
        output->setEdid(edid);
    }

    if (mPendingEDIDRequests.isEmpty()) {
        const KScreen::ConfigPtr config = mPendingConfigUpdate;
        mPendingConfigUpdate.clear();
        updateConfigs(config);
    }
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
    static ConfigMonitor *s_instance = Q_NULLPTR;

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
    delete d;
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
