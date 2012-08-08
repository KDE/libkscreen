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

#include "kscreen.h"
#include "config.h"

#include "backends/fake/fake.h"
#include "backends/xrandr/xrandr.h"

#include <QtCore/QDebug>
#include <QtCore/QStringList>
#include <QtCore/QCoreApplication>
#include <QtCore/QPluginLoader>


KScreen *KScreen::s_instance = 0;
KScreen::Error KScreen::s_error = KScreen::None;

KScreen::KScreen()
 : m_backend(0)
{
    QStringList paths = QCoreApplication::libraryPaths();
    QString backend(getenv("KSCREEN_BACKEND"));

    QPluginLoader loader;
    QObject *instance;
    Q_FOREACH(const QString& path, paths) {
        loader.setFileName(path + "/kscreen/KSC_" + backend + ".so");
        loader.load();
        instance = loader.instance();
        if (!instance) {
            continue;
        }

        m_backend = qobject_cast< AbstractBackend* >(instance);
        if (m_backend) {
            return;
        }

    }

    qDebug() << "Failed to load: " << backend;
}

KScreen* KScreen::self()
{
    if (s_instance) {
        return s_instance;
    }

    s_instance = new KScreen();

    if (!s_instance->isValid()) {
        return 0;
    }

    return s_instance;
}

KScreen::Error KScreen::error()
{
    return s_error;
}

const QString KScreen::errorString()
{
    switch(s_error) {
        case None:
            return QString();
            break;
        case NoCompatibleBackend:
            return QString("No compatible backend has been found");
            break;
        default:
            return QString("Unknown error");
    }
}

bool KScreen::isValid()
{
    return m_backend;
}

const QString KScreen::backend()
{
    return m_backend->name();
}

Config* KScreen::config()
{
    return m_backend->config();
}

void KScreen::setConfig(Config* config)
{
    m_backend->setConfig(config);
}