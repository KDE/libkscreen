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

#ifndef BACKENDLAUNCHER_H
#define BACKENDLAUNCHER_H

#include <QObject>
#include <QLoggingCategory>

class QPluginLoader;

namespace KScreen
{
class AbstractBackend;
}

class BackendLoader : public QObject
{
    Q_OBJECT

public:
    enum State {
        BackendLoaded = 0,
        BackendAlreadyExists = 1,
        BackendFailedToLoad = 2,
        LauncherStopped = 3
    };

    explicit BackendLoader();
    ~BackendLoader();

    bool loadBackend(const QString &backendName = QString());
    bool checkIsAlreadyRunning();

    KScreen::AbstractBackend* backend() const;

private:
    QPluginLoader *mLoader;
    KScreen::AbstractBackend* mBackend;
};

Q_DECLARE_LOGGING_CATEGORY(KSCREEN_BACKEND_LAUNCHER)

#endif // BACKENDLAUNCHER_H
