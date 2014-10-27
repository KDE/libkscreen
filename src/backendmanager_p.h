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

#ifndef KSCREEN_BACKENDMANAGER_H
#define KSCREEN_BACKENDMANAGER_H

#include <QObject>
#include <QProcess>

#include "types.h"
#include "backendinterface.h"
#include "kscreen_export.h"

class QDBusPendingCallWatcher;

namespace KScreen {

class KSCREEN_EXPORT BackendManager : public QObject
{
    Q_OBJECT

public:
    static BackendManager *instance();
    ~BackendManager();

    void requestBackend();

    KScreen::ConfigPtr config() const;

    void shutdownBackend();

Q_SIGNALS:
    void backendReady(org::kde::kscreen::Backend *backend);


private Q_SLOTS:
    void emitBackendReady();

    void startBackend(const QString &backend = QString());

    void launcherFinished(int existCode, QProcess::ExitStatus exitStatus);
    void launcherDataAvailable();

    void backendServiceUnregistered(const QString &serviceName);

private:
    void findBestBackend();
    void invalidateInterface();
    void backendServiceReady();


    explicit BackendManager();
    static BackendManager *sInstance;
    static const int sMaxCrashCount;

    org::kde::kscreen::Backend *mInterface;
    int mCrashCount;

    QProcess *mLauncher;
    QString mBackendService;
    QDBusServiceWatcher mServiceWatcher;
    KScreen::ConfigPtr mConfig;
    QTimer mRestCrashCountTimer;
    bool mShuttingDown;

    int mRequestsCounter;
    QEventLoop mShutdownLoop;

};

}

#endif // KSCREEN_BACKENDMANAGER_H
