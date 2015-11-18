/*
 * Copyright (C) 2014  Daniel Vratil <dvratil@redhat.com>
 * Copyright 2015 Sebastian Kügler <sebas@kde.org>
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

#include "abstractbackend.h"
#include "config.h"
#include "configmonitor.h"
#include "backendinterface.h"
#include "debug_p.h"
#include "getconfigoperation.h"
#include "configserializer_p.h"

#include <QDBusConnection>
#include <QDBusPendingCall>
#include <QDBusPendingCallWatcher>
#include <QDBusPendingReply>
#include <QDBusConnectionInterface>
#include <QStandardPaths>
#include <QThread>
#include <QX11Info>

#include <memory>


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
    , mShuttingDown(false)
    , mRequestsCounter(0)
    , mLoader(0)
    , mMethod(OutOfProcess)
{
    if (qgetenv("KSCREEN_BACKEND_INPROCESS") == QByteArray("1")) {
        mMethod = InProcess;
    }
    initMethod(true);
}

void BackendManager::initMethod(bool fromctor)
{
    if (mMethod == OutOfProcess) {
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
}

void BackendManager::setMethod(BackendManager::Method m)
{
    if (mMethod == m) {
        return;
    }
    shutdownBackend();
    mMethod = m;
    initMethod();
}

BackendManager::Method BackendManager::method() const
{
    return mMethod;
}

BackendManager::~BackendManager()
{
    shutdownBackend();
}

KScreen::AbstractBackend *BackendManager::loadBackendPlugin(QPluginLoader *loader, const QString &name,
                                                     const QVariantMap &arguments)
{
    //qCDebug(KSCREEN) << "Requested backend:" << name;
    const QString backendFilter = QString::fromLatin1("KSC_%1*").arg(name);
    const QStringList paths = QCoreApplication::libraryPaths();
    //qCDebug(KSCREEN) << "Lookup paths: " << paths;
    Q_FOREACH (const QString &path, paths) {
        const QDir dir(path + QLatin1String("/kf5/kscreen/"),
                       backendFilter,
                       QDir::SortFlags(QDir::QDir::NoSort),
                       QDir::NoDotAndDotDot | QDir::Files);
        const QFileInfoList finfos = dir.entryInfoList();
        Q_FOREACH (const QFileInfo &finfo, finfos) {
            //qCDebug(KSCREEN) << "path:" << finfo.path();
            // Skip "Fake" backend unless explicitly specified via KSCREEN_BACKEND
            if (name.isEmpty() && (finfo.fileName().contains(QLatin1String("KSC_Fake")) || finfo.fileName().contains(QLatin1String("KSC_FakeUI")))) {
                continue;
            }

            // When on X11, skip the QScreen backend, instead use the XRandR backend,
            // if not specified in KSCREEN_BACKEND
            if (name.isEmpty() &&
                finfo.fileName().contains(QLatin1String("KSC_QScreen")) &&
                QX11Info::isPlatformX11()) {
                continue;
            }
            if (name.isEmpty() &&
                finfo.fileName().contains(QLatin1String("KSC_Wayland"))) {
                continue;
            }

            // When not on X11, skip the XRandR backend, and fall back to QSCreen
            // if not specified in KSCREEN_BACKEND
            if (name.isEmpty() &&
                finfo.fileName().contains(QLatin1String("KSC_XRandR")) &&
                !QX11Info::isPlatformX11()) {
                continue;
            }

            //qCDebug(KSCREEN) << "Trying" << finfo.filePath() << loader->isLoaded();
            loader->setFileName(finfo.filePath());
            QObject *instance = loader->instance();
            if (!instance) {
                qCDebug(KSCREEN) << loader->errorString();
                continue;
            }

            auto backend = qobject_cast<KScreen::AbstractBackend*>(instance);
            if (backend) {
                backend->init(arguments);
                if (!backend->isValid()) {
                    qCDebug(KSCREEN) << "Skipping" << backend->name() << "backend";
                    delete backend;
                    continue;
                }
                qCDebug(KSCREEN) << "Loading" << backend->name() << "backend";
                return backend;
            } else {
                qCDebug(KSCREEN) << finfo.fileName() << "does not provide valid KScreen backend";
            }
        }
    }

    return Q_NULLPTR;
}

KScreen::AbstractBackend *BackendManager::loadBackendInProcess(const QString &name,
                                                      const QVariantMap &arguments)
{
    Q_ASSERT(mMethod == InProcess);
    if (mMethod == OutOfProcess) {
        qCWarning(KSCREEN) << "You are trying to load a backend in process, while the BackendManager is set to use OutOfProcess communication. Use the static version of loadBackend instead.";
        return nullptr;
    }
    if (m_inProcessBackend.first != nullptr && m_inProcessBackend.first->name() == name) {
        auto _backend = m_inProcessBackend.first;
        if (m_inProcessBackend.second != arguments) {
            _backend->init(arguments);
        }
        return _backend;

    }
    if (mLoader == nullptr) {
        mLoader = new QPluginLoader(this);
    }
    auto backend = BackendManager::loadBackendPlugin(mLoader, name, arguments);
    qCDebug(KSCREEN) << "Connecting ConfigMonitor to backend.";
    ConfigMonitor::instance()->connectInProcessBackend(backend);
    m_inProcessBackend = qMakePair<KScreen::AbstractBackend*, QVariantMap>(backend, arguments);
    return backend;
}

void BackendManager::requestBackend()
{
    Q_ASSERT(mMethod == OutOfProcess);
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

    const QByteArray args = qgetenv("KSCREEN_BACKEND_ARGS");
    QVariantMap arguments;
    if (!args.isEmpty()) {
        QList<QByteArray> arglist = args.split(';');
        Q_FOREACH (const QByteArray &arg, arglist) {
            const int pos = arg.indexOf('=');
            if (pos == -1) {
                continue;
            }
            arguments.insert(arg.left(pos), arg.mid(pos + 1));
        }
    }

    startBackend(QString::fromLatin1(qgetenv("KSCREEN_BACKEND")), arguments);
}

void BackendManager::emitBackendReady()
{
    Q_ASSERT(mMethod == OutOfProcess);
    Q_EMIT backendReady(mInterface);
    --mRequestsCounter;
    if (mShutdownLoop.isRunning()) {
        mShutdownLoop.quit();
    }
}

void BackendManager::startBackend(const QString &backend, const QVariantMap &arguments)
{
    qCDebug(KSCREEN) << "starting external backend launcher for" << backend;
    // This will autostart the launcher if it's not running already, calling
    // requestBackend(backend) will:
    //   a) if the launcher is started it will force it to load the correct backend,
    //   b) if the launcher is already running it will make sure it's running with
    //      the same backend as the one we requested and send an error otherwise
    QDBusConnection conn = QDBusConnection::sessionBus();
    QDBusMessage call = QDBusMessage::createMethodCall(QStringLiteral("org.kde.KScreen"),
                                                       QStringLiteral("/"),
                                                       QStringLiteral("org.kde.KScreen"),
                                                       QStringLiteral("requestBackend"));
    call.setArguments({ backend, arguments });
    QDBusPendingCall pending = conn.asyncCall(call);
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(pending);
    connect(watcher, &QDBusPendingCallWatcher::finished,
            this, &BackendManager::onBackendRequestDone);
}

void BackendManager::onBackendRequestDone(QDBusPendingCallWatcher *watcher)
{
    Q_ASSERT(mMethod == OutOfProcess);
    watcher->deleteLater();
    QDBusPendingReply<bool> reply = *watcher;
    // Most probably we requested an explicit backend that is different than the
    // one already loaded in the launcher
    if (reply.isError()) {
        qCWarning(KSCREEN) << "Failed to request backend:" << reply.error().name() << ":" << reply.error().message();
        invalidateInterface();
        emitBackendReady();
        return;
    }

    // Most probably request and explicit backend which is not available or failed
    // to initialize, or the launcher did not find any suitable backend for the
    // current platform.
    if (!reply.value()) {
        qCWarning(KSCREEN) << "Failed to request backend: unknown error";
        invalidateInterface();
        emitBackendReady();
        return;
    }

    // The launcher has successfully loaded the backend we wanted and registered
    // it to DBus (hopefuly), let's try to get an interface for the backend.
    if (mInterface) {
        invalidateInterface();
    }
    mInterface = new org::kde::kscreen::Backend(QStringLiteral("org.kde.KScreen"),
                                                QStringLiteral("/backend"),
                                                QDBusConnection::sessionBus());
    if (!mInterface->isValid()) {
        qCWarning(KSCREEN) << "Backend successfully requested, but we failed to obtain a valid DBus interface for it";
        invalidateInterface();
        emitBackendReady();
        return;
    }

    // The backend is GO, so let's watch for it's possible disappearance, so we
    // can invalidate the interface
    mServiceWatcher.addWatchedService(mBackendService);

    // Immediatelly request config
    connect(new GetConfigOperation(GetConfigOperation::NoEDID), &GetConfigOperation::finished,
            [&](ConfigOperation *op) {
                mConfig = qobject_cast<GetConfigOperation*>(op)->config();
            });
    // And listen for its change.
    connect(mInterface, &org::kde::kscreen::Backend::configChanged,
            [&](const QVariantMap &newConfig) {
                mConfig = KScreen::ConfigSerializer::deserializeConfig(newConfig);
            });

    emitBackendReady();
}

void BackendManager::backendServiceUnregistered(const QString &serviceName)
{
    Q_ASSERT(mMethod == OutOfProcess);
    mServiceWatcher.removeWatchedService(serviceName);

    invalidateInterface();
    requestBackend();
}

void BackendManager::invalidateInterface()
{
    Q_ASSERT(mMethod == OutOfProcess);
    delete mInterface;
    mInterface = 0;
    mBackendService.clear();
}

ConfigPtr BackendManager::config() const
{
    return mConfig;
}

void BackendManager::setConfig(ConfigPtr c)
{
    qCDebug(KSCREEN) << "BackendManager::setConfig, outputs:" << c->outputs().count();
    mConfig = c;
}

void BackendManager::shutdownBackend()
{
    if (mMethod == InProcess) {
        mLoader->deleteLater();
        mLoader = nullptr;
        m_inProcessBackend.second.clear();
        delete m_inProcessBackend.first;
        m_inProcessBackend.first = nullptr;
    } else {

        if (mBackendService.isEmpty() && !mInterface) {
            return;
        }

        // If there are some currently pending requests, then wait for them to
        // finish before quitting
        while (mRequestsCounter > 0) {
            mShutdownLoop.exec();
        }

        mServiceWatcher.removeWatchedService(mBackendService);
        mShuttingDown = true;

        QDBusMessage call = QDBusMessage::createMethodCall(QStringLiteral("org.kde.KScreen"),
                                                        QStringLiteral("/"),
                                                        QStringLiteral("org.kde.KScreen"),
                                                        QStringLiteral("quit"));
        // Call synchronously
        QDBusConnection::sessionBus().call(call);
        invalidateInterface();

        while (QDBusConnection::sessionBus().interface()->isServiceRegistered(QStringLiteral("org.kde.KScreen"))) {
            QThread::msleep(100);
        }
    }
}
