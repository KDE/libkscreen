/*
 * Copyright (C) 2014  Daniel Vratil <dvratil@redhat.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#include "backenddbuswrapper.h"
#include "backendloader.h"
#include "backendadaptor.h"
#include "src/configserializer_p.h"
#include "src/config.h"
#include "src/abstractbackend.h"

#include <QDBusConnection>
#include <QDBusError>

BackendDBusWrapper::BackendDBusWrapper(KScreen::AbstractBackend* backend)
    : QObject()
    , mBackend(backend)
{
    connect(mBackend, &KScreen::AbstractBackend::configChanged,
            this, &BackendDBusWrapper::backendConfigChanged);

    mChangeCollector.setSingleShot(true);
    mChangeCollector.setInterval(200); // wait for 200 msecs without any change
                                       // before actually emitting configChanged
    connect(&mChangeCollector, &QTimer::timeout,
            this, &BackendDBusWrapper::doEmitConfigChanged);
}

BackendDBusWrapper::~BackendDBusWrapper()
{
}

bool BackendDBusWrapper::init()
{
    QDBusConnection dbus = QDBusConnection::sessionBus();
    if (!dbus.registerService(mBackend->serviceName())) {
        qCWarning(KSCREEN_BACKEND_LAUNCHER) << "Failed to register as DBus service: another launcher already running?";
        qCWarning(KSCREEN_BACKEND_LAUNCHER) << dbus.lastError().message();
        return false;
    }

    new BackendAdaptor(this);
    if (!dbus.registerObject(QLatin1String("/"), this, QDBusConnection::ExportAdaptors)) {
        qCWarning(KSCREEN_BACKEND_LAUNCHER) << "Failed to export backend to DBus: another launcher already running?";
        qCWarning(KSCREEN_BACKEND_LAUNCHER) << dbus.lastError().message();
        return false;
    }

    return true;
}

QVariantMap BackendDBusWrapper::getConfig() const
{
    const KScreen::ConfigPtr config = mBackend->config();
    Q_ASSERT(!config.isNull());
    if (!config) {
        qCWarning(KSCREEN_BACKEND_LAUNCHER) << "Backend provided an empty config!";
        return QVariantMap();
    }

    const QJsonObject obj = KScreen::ConfigSerializer::serializeConfig(mBackend->config());
    Q_ASSERT(!obj.isEmpty());
    return obj.toVariantMap();
}

QVariantMap BackendDBusWrapper::setConfig(const QVariantMap &configMap)
{
    if (configMap.isEmpty()) {
        qCWarning(KSCREEN_BACKEND_LAUNCHER) << "Received an empty config map";
        return QVariantMap();
    }

    const KScreen::ConfigPtr config = KScreen::ConfigSerializer::deserializeConfig(configMap);
    mBackend->setConfig(config);
    // TODO: setConfig should return adjusted config that we should use
    return getConfig();
}

QByteArray BackendDBusWrapper::getEdid(int output) const
{
    const QByteArray edidData =  mBackend->edid(output);
    if (edidData.isEmpty()) {
        return QByteArray();
    }

    return edidData;
}

void BackendDBusWrapper::quit()
{
    qCDebug(KSCREEN_BACKEND_LAUNCHER) << "Launcher termination requested";
    qApp->exit(BackendLoader::LauncherStopped);
}

void BackendDBusWrapper::backendConfigChanged(const KScreen::ConfigPtr &config)
{
    Q_ASSERT(!config.isNull());
    if (!config) {
        qCWarning(KSCREEN_BACKEND_LAUNCHER) << "Backend provided an empty config!";
        return;
    }

    mCurrentConfig = config;
    mChangeCollector.start();
}

void BackendDBusWrapper::doEmitConfigChanged()
{
    Q_ASSERT(!mCurrentConfig.isNull());
    if (mCurrentConfig.isNull()) {
        return;
    }

    const QJsonObject obj = KScreen::ConfigSerializer::serializeConfig(mCurrentConfig);
    Q_EMIT configChanged(obj.toVariantMap());

    mCurrentConfig.clear();
}

