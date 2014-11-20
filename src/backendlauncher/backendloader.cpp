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
#include <QDir>
#include <QPluginLoader>
#include <QX11Info>

#include <QDBusConnection>
#include <QDBusInterface>

Q_LOGGING_CATEGORY(KSCREEN_BACKEND_LAUNCHER, "kscreen.backendLauncher")

BackendLoader::BackendLoader()
    : QObject()
{
}

BackendLoader::~BackendLoader()
{
}

bool BackendLoader::loadBackend(const QString& backend)
{
    qCDebug(KSCREEN_BACKEND_LAUNCHER) << "Requested backend:" << backend;
    const QString backendFilter = QString::fromLatin1("KSC_%1*").arg(backend);
    const QStringList paths = QCoreApplication::libraryPaths();
    qCDebug(KSCREEN_BACKEND_LAUNCHER) << "Lookup paths: " << paths;
    Q_FOREACH (const QString &path, paths) {
        const QDir dir(path + QDir::separator() + QLatin1String("/kf5/kscreen/"),
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
            QPluginLoader loader(finfo.filePath());
            loader.load();
            QObject *instance = loader.instance();
            if (!instance) {
                qCDebug(KSCREEN_BACKEND_LAUNCHER) << loader.errorString();
                loader.unload();
                continue;
            }

            mBackend = qobject_cast<KScreen::AbstractBackend*>(instance);
            if (mBackend) {
                if (!mBackend->isValid()) {
                    qCDebug(KSCREEN_BACKEND_LAUNCHER) << "Skipping" << mBackend->name() << "backend";
                    delete mBackend;
                    mBackend = 0;
                    loader.unload();
                    continue;
                }
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
    QDBusInterface *iface = new QDBusInterface(mBackend->serviceName(),
                                               QLatin1String("/"),
                                               QLatin1String("org.kde.KScreen.Backend"),
                                               QDBusConnection::sessionBus(),
                                               this);
    const bool valid = iface->isValid();
    delete iface;
    return valid;
}
