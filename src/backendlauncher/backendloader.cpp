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

#include "backendloader.h"
#include "backenddbuswrapper.h"
#include "backendloaderadaptor.h"
#include "kscreen_backendLauncher_debug.h"
#include "src/abstractbackend.h"
#include "src/backendmanager_p.h"

#include <QCoreApplication>
#include <QDBusConnectionInterface>
#include <QDir>
#include <QPluginLoader>
#include <QX11Info>

#include <memory>

#include <QDBusConnection>
#include <QDBusInterface>

void pluginDeleter(QPluginLoader *p)
{
    if (p) {
        qCDebug(KSCREEN_BACKEND_LAUNCHER) << "Unloading" << p->fileName();
        p->unload();
        delete p;
    }
}

BackendLoader::BackendLoader()
    : QObject()
    , QDBusContext()
    , mLoader(nullptr)
    , mBackend(nullptr)
{
}

BackendLoader::~BackendLoader()
{
    delete mBackend;
    pluginDeleter(mLoader);
    qCDebug(KSCREEN_BACKEND_LAUNCHER) << "Backend loader destroyed";
}

bool BackendLoader::init()
{
    QDBusConnection dbus = QDBusConnection::sessionBus();
    new BackendLoaderAdaptor(this);
    if (!dbus.registerObject(QStringLiteral("/"), this, QDBusConnection::ExportAdaptors)) {
        qCWarning(KSCREEN_BACKEND_LAUNCHER) << "Failed to export backend to DBus: another launcher already running?";
        qCWarning(KSCREEN_BACKEND_LAUNCHER) << dbus.lastError().message();
        return false;
    }

    return true;
}

QString BackendLoader::backend() const
{
    if (mBackend) {
        return mBackend->backend()->name();
    }

    return QString();
}

bool BackendLoader::requestBackend(const QString &backendName, const QVariantMap &arguments)
{
    if (mBackend) {
        // If an backend is already loaded, but it's not the same as the one
        // requested, then it's an error
        if (!backendName.isEmpty() && mBackend->backend()->name() != backendName) {
            sendErrorReply(QDBusError::Failed, QStringLiteral("Another backend is already active"));
            return false;
        } else {
            // If caller requested the same one as already loaded, or did not
            // request a specific backend, hapilly reuse the existing one
            return true;
        }
    }

    KScreen::AbstractBackend *backend = loadBackend(backendName, arguments);
    if (!backend) {
        return false;
    }

    mBackend = new BackendDBusWrapper(backend);
    if (!mBackend->init()) {
        delete mBackend;
        mBackend = nullptr;
        pluginDeleter(mLoader);
        mLoader = nullptr;
        return false;
    }
    return true;
}

KScreen::AbstractBackend *BackendLoader::loadBackend(const QString &name, const QVariantMap &arguments)
{
    if (mLoader == nullptr) {
        std::unique_ptr<QPluginLoader, void (*)(QPluginLoader *)> loader(new QPluginLoader(), pluginDeleter);
        mLoader = loader.release();
    }
    return KScreen::BackendManager::loadBackendPlugin(mLoader, name, arguments);
}

void BackendLoader::quit()
{
    qApp->quit();
}
