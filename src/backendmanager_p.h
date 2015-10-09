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

/**
 * WARNING: This header is *not* part of public API and is subject to change.
 * There are not guarantees or API or ABI stability or compatibility between
 * releases
 */

#ifndef KSCREEN_BACKENDMANAGER_H
#define KSCREEN_BACKENDMANAGER_H

#include <QObject>
#include <QProcess>
#include <QDBusServiceWatcher>
#include <QTimer>
#include <QEventLoop>

#include "types.h"
#include "kscreen_export.h"

class QDBusPendingCallWatcher;
class OrgKdeKscreenBackendInterface;

namespace KScreen {

class KSCREEN_EXPORT BackendManager : public QObject
{
    Q_OBJECT

public:
    static BackendManager *instance();
    ~BackendManager();

    void requestBackend();
    void shutdownBackend();

    KScreen::ConfigPtr config() const;

Q_SIGNALS:
    void backendReady(OrgKdeKscreenBackendInterface *backend);

private Q_SLOTS:
    void emitBackendReady();

    void startBackend(const QString &backend = QString(),
                      const QVariantMap &arguments = QVariantMap());
    void onBackendRequestDone(QDBusPendingCallWatcher *watcher);

    void backendServiceUnregistered(const QString &serviceName);

private:
    void invalidateInterface();
    void backendServiceReady();


    explicit BackendManager();
    static BackendManager *sInstance;
    static const int sMaxCrashCount;

    OrgKdeKscreenBackendInterface *mInterface;
    int mCrashCount;

    QString mBackendService;
    QDBusServiceWatcher mServiceWatcher;
    KScreen::ConfigPtr mConfig;
    QTimer mResetCrashCountTimer;
    bool mShuttingDown;

    int mRequestsCounter;
    QEventLoop mShutdownLoop;

};

}

#endif // KSCREEN_BACKENDMANAGER_H
