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
#include "backendloaderadaptor.h"
#include "backenddbuswrapper.h"
#include "debug_p.h"
#include "src/abstractbackend.h"

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
    , mLoader(Q_NULLPTR)
    , mBackend(Q_NULLPTR)
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
    if (!dbus.registerObject(QLatin1String("/"), this, QDBusConnection::ExportAdaptors)) {
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

bool BackendLoader::requestBackend(const QString &backendName)
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

    KScreen::AbstractBackend *backend = loadBackend(backendName);
    if (!backend) {
        return false;
    }

    mBackend = new BackendDBusWrapper(backend);
    if (!mBackend->init()) {
        delete mBackend;
        mBackend = Q_NULLPTR;
        pluginDeleter(mLoader);
        mLoader = Q_NULLPTR;
        return false;
    }
    return true;
}

KScreen::AbstractBackend *BackendLoader::loadBackend(const QString &name)
{
    qCDebug(KSCREEN_BACKEND_LAUNCHER) << "Requested backend:" << name;
    const QString backendFilter = QString::fromLatin1("KSC_%1*").arg(name);
    const QStringList paths = QCoreApplication::libraryPaths();
    qCDebug(KSCREEN_BACKEND_LAUNCHER) << "Lookup paths: " << paths;
    Q_FOREACH (const QString &path, paths) {
        const QDir dir(path + QLatin1String("/kf5/kscreen/"),
                       backendFilter,
                       QDir::SortFlags(QDir::QDir::NoSort),
                       QDir::NoDotAndDotDot | QDir::Files);
        const QFileInfoList finfos = dir.entryInfoList();
        Q_FOREACH (const QFileInfo &finfo, finfos) {
            // Skip "Fake" backend unless explicitly specified via KSCREEN_BACKEND
            if (name.isEmpty() && (finfo.fileName().contains(QLatin1String("KSC_Fake")) || finfo.fileName().contains(QLatin1String("KSC_FakeUI")))) {
                continue;
            }

            // When on X11, skip the QScreen backend, instead use the XRandR backend,
            // if not specified in KSCREEN_BACKEND
            if (name.isEmpty() &&
                    finfo.fileName().contains(QLatin1String("KSC_QScreen")) &&
                    QX11Info::isPlatformX11()) {
                continue;
            }

            // When not on X11, skip the XRandR backend, and fall back to QSCreen
            // if not specified in KSCREEN_BACKEND
            if (name.isEmpty() &&
                    finfo.fileName().contains(QLatin1String("KSC_XRandR")) &&
                    !QX11Info::isPlatformX11()) {
                continue;
            }

            qCDebug(KSCREEN_BACKEND_LAUNCHER) << "Trying" << finfo.filePath();
            // Make sure we unload() and delete the loader whenever it goes out of scope here
            std::unique_ptr<QPluginLoader, void(*)(QPluginLoader *)> loader(new QPluginLoader(finfo.filePath()), pluginDeleter);
            QObject *instance = loader->instance();
            if (!instance) {
                qCDebug(KSCREEN_BACKEND_LAUNCHER) << loader->errorString();
                continue;
            }

            auto backend = qobject_cast<KScreen::AbstractBackend*>(instance);
            if (backend) {
                if (!backend->isValid()) {
                    qCDebug(KSCREEN_BACKEND_LAUNCHER) << "Skipping" << backend->name() << "backend";
                    delete backend;
                    continue;
                }

                // This is the only case we don't want to unload() and delete the loader, instead
                // we store it and unload it when the backendloader terminates.
                mLoader = loader.release();
                qCDebug(KSCREEN_BACKEND_LAUNCHER) << "Loading" << backend->name() << "backend";
                return backend;
            } else {
                qCDebug(KSCREEN_BACKEND_LAUNCHER) << finfo.fileName() << "does not provide valid KScreen backend";
            }
        }
    }

    return Q_NULLPTR;
}

void BackendLoader::quit()
{
    qApp->quit();
}
