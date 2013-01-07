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

AbstractBackend* BackendLoader::s_backend = 0;


bool BackendLoader::init()
{
    if (s_backend) {
        return true;
    }

    QStringList paths = QCoreApplication::libraryPaths();
    QString backend(getenv("KSCREEN_BACKEND"));
    if (backend.isEmpty()) {
        qWarning() << "No KScreen backend set!";
        return false;
    }

    QPluginLoader loader;
    QObject *instance;
    Q_FOREACH(const QString& path, paths) {
        loader.setFileName(path + "/kscreen/KSC_" + backend + ".so");
        loader.load();
        instance = loader.instance();
        if (!instance) {
            continue;
        }

        s_backend = qobject_cast< AbstractBackend* >(instance);
        if (s_backend) {
            return true;
        }

    }

    qWarning() << "Backend '" << backend << "' not found";
    return false;
}

AbstractBackend* BackendLoader::backend()
{
    return s_backend;
}
