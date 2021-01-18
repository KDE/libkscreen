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

#include <QDBusContext>
#include <QObject>

namespace KScreen
{
class AbstractBackend;
}

class QPluginLoader;
class BackendDBusWrapper;

class BackendLoader : public QObject, protected QDBusContext
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.kde.KScreen")

public:
    explicit BackendLoader();
    ~BackendLoader() override;

    bool init();

    Q_INVOKABLE QString backend() const;
    Q_INVOKABLE bool requestBackend(const QString &name, const QVariantMap &arguments);
    Q_INVOKABLE void quit();

private:
    KScreen::AbstractBackend *loadBackend(const QString &name, const QVariantMap &arguments);

private:
    QPluginLoader *mLoader = nullptr;
    BackendDBusWrapper *mBackend = nullptr;
};

#endif // BACKENDLAUNCHER_H
