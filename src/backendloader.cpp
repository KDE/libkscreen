/*************************************************************************************
 *  Copyright (C) 2012 by Alejandro Fiestas Olivares <afiestas@kde.org>              *
 *                                                                                   *
 *  This program is free software; you can redistribute it and/or                    *
 *  modify it under the terms of the GNU General Public License                      *
 *  as published by the Free Software Foundation; either version 2                   *
 *  of the License, or (at your option) any later version.                           *
 *                                                                                   *
 *  This program is distributed in the hope that it will be useful,                  *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of                   *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                    *
 *  GNU General Public License for more details.                                     *
 *                                                                                   *
 *  You should have received a copy of the GNU General Public License                *
 *  along with this program; if not, write to the Free Software                      *
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA   *
 *************************************************************************************/

#include "backendloader.h"
#include "backends/abstractbackend.h"

#include <QtCore/QStringList>
#include <QtCore/QCoreApplication>
#include <QtCore/QPluginLoader>
#include <QDir>
#include <kdebug.h>

AbstractBackend* BackendLoader::s_backend = 0;


bool BackendLoader::init()
{
    if (s_backend) {
        return true;
    }

    KDebug::Block block("Loading backend");

    const QString backend = qgetenv("KSCREEN_BACKEND").constData();
    const QString backendFilter = QString::fromLatin1("KSC_%1*").arg(backend);

    const QStringList paths = QCoreApplication::libraryPaths();
    Q_FOREACH (const QString &path, paths) {
        const QDir dir(path + QDir::separator() + QLatin1String("kscreen"),
                       backendFilter,
                       QDir::SortFlags(QDir::QDir::NoSort),
                       QDir::NoDotAndDotDot | QDir::Files);
        const QFileInfoList finfos = dir.entryInfoList();
        Q_FOREACH (const QFileInfo &finfo, finfos) {
            // Skip "Fake" backend unless explicitly specified via KSCREEN_BACKEND
            if (backend.isEmpty() && finfo.fileName().contains(QLatin1String("KSC_Fake"))) {
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
                    kDebug() << "Skipping" << s_backend->name() << "backend";
                    delete s_backend;
                    s_backend = 0;
                    loader.unload();
                    continue;
                }
                kDebug() << "Loading" << s_backend->name() << "backend";
                return true;
            }
        }
    }

    kDebug() << "No backend found!";
    return false;
}

AbstractBackend* BackendLoader::backend()
{
    return s_backend;
}
