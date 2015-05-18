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
#include "src/abstractbackend.h"

#include <QCoreApplication>
#include <QDBusConnectionInterface>
#include <QDir>
#include <QPluginLoader>
#include <QX11Info>

#include <memory>

#include <QDBusConnection>
#include <QDBusInterface>

Q_LOGGING_CATEGORY(KSCREEN_BACKEND_LAUNCHER, "kscreen.backendLauncher")

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
    , mLoader(0)
    , mBackend(0)
{
}

BackendLoader::~BackendLoader()
{
    delete mBackend;
    pluginDeleter(mLoader);
    qCDebug(KSCREEN_BACKEND_LAUNCHER) << "Backend loader destroyed";
}

bool BackendLoader::loadBackend(const QString& backend)
{
    qCDebug(KSCREEN_BACKEND_LAUNCHER) << "Requested backend:" << backend;
    const QString backendFilter = QString::fromLatin1("KSC_%1*").arg(backend);
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
            if (backend.isEmpty() && (finfo.fileName().contains(QLatin1String("KSC_Fake")) || finfo.fileName().contains(QLatin1String("KSC_FakeUI")))) {
                continue;
            }

            // When on X11, skip the QScreen backend, instead use the XRandR backend,
            // if not specified in KSCREEN_BACKEND
            if (backend.isEmpty() &&
                    finfo.fileName().contains(QLatin1String("KSC_QScreen")) &&
                    QX11Info::isPlatformX11()) {
                continue;
            }

            // When not on X11, skip the XRandR backend, and fall back to QSCreen
            // if not specified in KSCREEN_BACKEND
            if (backend.isEmpty() &&
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

            mBackend = qobject_cast<KScreen::AbstractBackend*>(instance);
            if (mBackend) {
                if (!mBackend->isValid()) {
                    qCDebug(KSCREEN_BACKEND_LAUNCHER) << "Skipping" << mBackend->name() << "backend";
                    delete mBackend;
                    mBackend = 0;
                    continue;
                }

                // This is the only case we don't want to unload() and delete the loader, instead
                // we store it and unload it when the backendloader terminates.
                mLoader = loader.release();
                qCDebug(KSCREEN_BACKEND_LAUNCHER) << "Loading" << mBackend->name() << "backend";
                return true;
            } else {
                qCDebug(KSCREEN_BACKEND_LAUNCHER) << finfo.fileName() << "does not provide valid KScreen backend";
            }
        }
    }

    return false;
}

KScreen::AbstractBackend* BackendLoader::backend() const
{
    return mBackend;
}

bool BackendLoader::checkIsAlreadyRunning()
{
    return QDBusConnection::sessionBus().interface()->isServiceRegistered(mBackend->serviceName());
}
