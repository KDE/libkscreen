/*************************************************************************************
 *  Copyright (C) 2012 by Alejandro Fiestas Olivares <afiestas@kde.org>              *
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

#include "backendloader.h"
#include "debug_p.h"
#include "backends/abstractbackend.h"

#include <QtCore/QStringList>
#include <QtCore/QCoreApplication>
#include <QtCore/QPluginLoader>
#include <QX11Info>
#include <QDir>

AbstractBackend* BackendLoader::s_backend = 0;


bool BackendLoader::init()
{
    if (s_backend) {
        return true;
    }

    qCDebug(KSCREEN) << "Loading backend";

    const QString backend = qgetenv("KSCREEN_BACKEND").constData();
    const QString backendFilter = QString::fromLatin1("KSC_%1*").arg(backend);

    const QStringList paths = QCoreApplication::libraryPaths();
    Q_FOREACH (const QString &path, paths) {
        const QDir dir(path + QDir::separator() + QLatin1String("/kf5/kscreen/"),
                       backendFilter,
                       QDir::SortFlags(QDir::QDir::NoSort),
                       QDir::NoDotAndDotDot | QDir::Files);
        const QFileInfoList finfos = dir.entryInfoList();
        Q_FOREACH (const QFileInfo &finfo, finfos) {
            // Skip "Fake" backend unless explicitly specified via KSCREEN_BACKEND
            if (backend.isEmpty() && finfo.fileName().contains(QLatin1String("KSC_Fake"))) {
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

            // When not on X11, skip the Wayland backend, and fall back to xrandr
            // if not specified in KSCREEN_BACKEND
            if (backend.isEmpty() &&
                    finfo.fileName().contains(QLatin1String("KSC_Wayland")) &&
                    QX11Info::isPlatformX11()) {
                continue;
            }

            QPluginLoader loader(finfo.filePath());
            loader.load();
            QObject *instance = loader.instance();
            if (!instance) {
                loader.unload();
                continue;
            }

            s_backend = qobject_cast< AbstractBackend* >(instance);
            if (s_backend) {
                if (!s_backend->isValid()) {
                    qCDebug(KSCREEN) << "Skipping" << s_backend->name() << "backend";
                    delete s_backend;
                    s_backend = 0;
                    loader.unload();
                    continue;
                }
                qCDebug(KSCREEN) << "Loading" << s_backend->name() << "backend";
                return true;
            }
        }
    }

    qCDebug(KSCREEN) << "No backend found!";
    return false;
}

AbstractBackend* BackendLoader::backend()
{
    return s_backend;
}
