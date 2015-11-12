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
#include <QPluginLoader>
#include <QProcess>
#include <QDBusServiceWatcher>
#include <QTimer>
#include <QEventLoop>

#include "types.h"
#include "kscreen_export.h"

class QDBusPendingCallWatcher;
class OrgKdeKscreenBackendInterface;

namespace KScreen {

class AbstractBackend;

class KSCREEN_EXPORT BackendManager : public QObject
{
    Q_OBJECT

public:
    enum Mode {
        InProcess,
        OutOfProcess
    };

    static BackendManager *instance();
    ~BackendManager();

    void requestBackend();
    void shutdownBackend();


    KScreen::ConfigPtr config() const;
    void setConfig(KScreen::ConfigPtr c);

    static KScreen::AbstractBackend *loadBackend(QPluginLoader *loader,
                                                 const QString &name,
                                                 const QVariantMap &arguments);

    KScreen::AbstractBackend *loadBackend(const QString &name,
                                          const QVariantMap &arguments);


Q_SIGNALS:
    void backendReady(OrgKdeKscreenBackendInterface *backend);

private Q_SLOTS:
    void emitBackendReady();

    void startBackend(const QString &backend = QString(),
                      const QVariantMap &arguments = QVariantMap());
    void onBackendRequestDone(QDBusPendingCallWatcher *watcher);

    void backendServiceUnregistered(const QString &serviceName);

private:
    friend class SetInProcessOperation;
    friend class SetConfigOperation;
    //friend class SetInProcessOperation;
    friend class InProcessConfigOperationPrivate;
    friend class SetConfigOperationPrivate;

    explicit BackendManager();
    static BackendManager *sInstance;

    // For out-of-process operation
    void invalidateInterface();
    void backendServiceReady();

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

    // For in-process operation
    QPluginLoader *mLoader;
    KScreen::AbstractBackend *mInProcessBackend;
    void setConfigInProcess(ConfigPtr config);

};

}

#endif // KSCREEN_BACKENDMANAGER_H
