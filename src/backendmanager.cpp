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

#include "backendmanager_p.h"
#include "backendinterface.h"
#include "backendlauncher/backendloader.h"
#include "debug_p.h"
#include "getconfigoperation.h"
#include "configserializer_p.h"

#include <QDBusConnection>
#include <QDBusPendingCall>
#include <QDBusPendingCallWatcher>
#include <QDBusPendingReply>
#include <QDBusConnectionInterface>
#include <QStandardPaths>

#include "config-libkscreen.h"

#include <QProcess>

#ifdef Q_OS_UNIX
#include <sys/wait.h>
#include <signal.h>
#endif

using namespace KScreen;

Q_DECLARE_METATYPE(org::kde::kscreen::Backend*)

const int BackendManager::sMaxCrashCount = 4;

BackendManager *BackendManager::sInstance = 0;

BackendManager *BackendManager::instance()
{
    if (!sInstance) {
        sInstance = new BackendManager();
    }

    return sInstance;
}

BackendManager::BackendManager()
    : QObject()
    , mInterface(0)
    , mCrashCount(0)
    , mLauncher(0)
    , mShuttingDown(false)
    , mRequestsCounter(0)
{
    qRegisterMetaType<org::kde::kscreen::Backend*>("OrgKdeKscreenBackendInterface");

    mServiceWatcher.setConnection(QDBusConnection::sessionBus());
    connect(&mServiceWatcher, &QDBusServiceWatcher::serviceUnregistered,
            this, &BackendManager::backendServiceUnregistered);

    mResetCrashCountTimer.setSingleShot(true);
    mResetCrashCountTimer.setInterval(60000);
    connect(&mResetCrashCountTimer, &QTimer::timeout,
            this, [=]() {
                mCrashCount = 0;
            });
}

BackendManager::~BackendManager()
{
}

void BackendManager::requestBackend()
{
    if (mInterface && mInterface->isValid()) {
        ++mRequestsCounter;
        QMetaObject::invokeMethod(this, "emitBackendReady", Qt::QueuedConnection);
        return;
    }

    // Another request already pending
    if (mRequestsCounter > 0) {
        return;
    }
    ++mRequestsCounter;

    startBackend(QString::fromLatin1(qgetenv("KSCREEN_BACKEND")));
}

void BackendManager::emitBackendReady()
{
    Q_EMIT backendReady(mInterface);
    --mRequestsCounter;
    if (mShutdownLoop.isRunning()) {
        mShutdownLoop.quit();
    }
}

void BackendManager::startBackend(const QString &backend)
{
    if (mLauncher && mLauncher->state() == QProcess::Running) {
        mLauncher->terminate();
    }

    mLauncher = new QProcess(this);
    connect(mLauncher, static_cast<void(QProcess::*)(int,QProcess::ExitStatus)>(&QProcess::finished),
            this, &BackendManager::launcherFinished);
    connect(mLauncher, &QProcess::readyReadStandardOutput,
            this, &BackendManager::launcherDataAvailable);
    connect(mLauncher, &QProcess::readyReadStandardError,
            this, [&] () {
                QString err = QString::fromLocal8Bit(mLauncher->readAllStandardError());
                err.chop(1);
                qCDebug(KSCREEN) << "BACKEND\nBACKEND\t" << err;
            } );

    QString launcher = QString::fromLatin1(CMAKE_INSTALL_FULL_LIBEXECDIR_KF5 "/kscreen_backend_launcher");
    if (!QFile::exists(launcher)) {
        launcher = QStandardPaths::findExecutable("kscreen_backend_launcher");
        if (launcher.isEmpty()) {
            qCWarning(KSCREEN) << "Failed to locate kscreen_backend_launcher, KScreen will be useless";
            invalidateInterface();
            delete mLauncher;
            mLauncher = 0;
            QMetaObject::invokeMethod(this, "emitBackendReady", Qt::QueuedConnection);
            return;
        }
    }

    mLauncher->setProgram(launcher);
    if (!backend.isEmpty()) {
        mLauncher->setArguments(QStringList() << "--backend" << backend);
    }

    mLauncher->start();
    if (!qgetenv("KSCREEN_BACKEND_DEBUG").isEmpty()) {
        pid_t pid = mLauncher->pid();
        qCDebug(KSCREEN) << "==================================";
        qCDebug(KSCREEN) << "KScreen BackendManager: Suspending backend launcher";
        qCDebug(KSCREEN) << "'gdb --pid" << pid << "' to debug";
        qCDebug(KSCREEN) << "'kill -SIGCONT" << pid << "' to continue";
        qCDebug(KSCREEN) << "==================================";
        qCDebug(KSCREEN);
        kill(pid, SIGSTOP);
    }

    mResetCrashCountTimer.start();
}

void BackendManager::launcherFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    qCDebug(KSCREEN) << "Launcher finished with exit code" << exitCode << ", status" << exitStatus;

    // Stop the timer if it's running, otherwise the number would get reset to 0
    // anyway even if we reached the sMaxCrashCount, and then the backend would
    // be restarted again anyway.
    mResetCrashCountTimer.stop();

    if (exitStatus == QProcess::CrashExit) {
        // Backend has crashed: restart it
        invalidateInterface();
        if (!mShuttingDown) {
            if (++mCrashCount <= sMaxCrashCount) {
                requestBackend();
            } else {
                qCWarning(KSCREEN) << "Launcher has crashed too many times: not restarting";
                mLauncher->deleteLater();
                mLauncher = 0;
            }
        }
        mShuttingDown = false;
        return;
    }

    switch (exitCode) {
    case BackendLoader::BackendLoaded:
        // This means that the launcher has terminated successfully after doing
        // what it was asked to do, so delete the interface, but don't emit signals
        invalidateInterface();
        break;

    case BackendLoader::BackendFailedToLoad:
        // Launcher terminated immediatelly because there was no backend, this
        // means that we didn't try before and someone is probably waiting for
        // the signal
        qCWarning(KSCREEN) << "Launcher failed to load any backend: KScreen will be useless";
        invalidateInterface();
        emitBackendReady();
        break;

    case BackendLoader::BackendAlreadyExists:
        // The launcher wrote service name to stdout, so backendReady() was emitted
        // from launcherDataAvailable(), nothing else to do here
        qCDebug(KSCREEN) << "Service for requested backend already running";
        break;

    case BackendLoader::LauncherStopped:
        // The launcher has been stopped on request, probably by someone calling
        // shutdownBackend()
        qCDebug(KSCREEN) << "Backend launcher terminated on requested";
        invalidateInterface();
        break;
    }

    mShuttingDown = false;
    mLauncher->deleteLater();
    mLauncher = 0;
};

void BackendManager::launcherDataAvailable()
{
    mLauncher->setReadChannel(QProcess::StandardOutput);
    const QByteArray service = mLauncher->readLine();
    qCDebug(KSCREEN) << "launcherDataAvailable:" << service;
    mBackendService = QString::fromLatin1(service);

    mInterface = new org::kde::kscreen::Backend(mBackendService,
                                                QLatin1String("/"),
                                                QDBusConnection::sessionBus(),
                                                this);
    if (!mInterface->isValid()) {
        QDBusServiceWatcher *watcher = new QDBusServiceWatcher(mBackendService,
                                                               QDBusConnection::sessionBus());
        connect(watcher, &QDBusServiceWatcher::serviceOwnerChanged,
                [&](const QString &service, const QString &oldOwner, const QString &newOwner) {
                    qDebug() << service << newOwner << oldOwner;
                    if (newOwner == mBackendService) {
                        backendServiceReady();
                    }
                });
        return;
    }
    backendServiceReady();
}

void BackendManager::backendServiceReady()
{
    mServiceWatcher.addWatchedService(mBackendService);

    // Immediatelly request config
    connect(new GetConfigOperation(GetConfigOperation::NoEDID), &GetConfigOperation::finished,
            [&](ConfigOperation *op) {
                mConfig = qobject_cast<GetConfigOperation*>(op)->config();
            });
    connect(mInterface, &org::kde::kscreen::Backend::configChanged,
            [&](const QVariantMap &newConfig) {
                mConfig = KScreen::ConfigSerializer::deserializeConfig(newConfig);
            });

    emitBackendReady();
}

void BackendManager::backendServiceUnregistered(const QString &serviceName)
{
    mServiceWatcher.removeWatchedService(serviceName);

    invalidateInterface();
    requestBackend();
}

void BackendManager::invalidateInterface()
{
    delete mInterface;
    mInterface = 0;
    mBackendService.clear();
}

ConfigPtr BackendManager::config() const
{
    return mConfig;
}

void BackendManager::shutdownBackend()
{
    if (mBackendService.isEmpty() && !mInterface) {
        return;
    }

    while (mRequestsCounter > 0) {
        mShutdownLoop.exec();
    }

    mServiceWatcher.removeWatchedService(mBackendService);
    mShuttingDown = true;
    const QDBusReply<uint> reply = QDBusConnection::sessionBus().interface()->servicePid(mInterface->service());

    // Call synchronously
    mInterface->quit().waitForFinished();
    invalidateInterface();

    if (mLauncher) {
        mLauncher->waitForFinished(5000);
        // This will ensure that launcherFinished() is called, which will take care
        // of deleting the QProcess
    } else {
        // ... ?
    }
}
