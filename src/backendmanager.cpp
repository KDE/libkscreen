/*
 * SPDX-FileCopyrightText: 2014 Daniel Vratil <dvratil@redhat.com>
 * SPDX-FileCopyrightText: 2015 Sebastian Kügler <sebas@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 */

#include "backendmanager_p.h"

#include "abstractbackend.h"
#include "backendinterface.h"
#include "configmonitor.h"
#include "getconfigoperation.h"
#include "kscreen_debug.h"

#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDBusPendingCall>
#include <QDBusPendingCallWatcher>
#include <QDBusPendingReply>
#include <QGuiApplication>
#include <QStandardPaths>
#include <QThread>
#include <QtGui/private/qtx11extras_p.h>

#include <memory>

using namespace KScreen;

BackendManager *BackendManager::sInstance = nullptr;

BackendManager *BackendManager::instance()
{
    if (!sInstance) {
        sInstance = new BackendManager();
    }

    return sInstance;
}

BackendManager::BackendManager()
    : mLoader(nullptr)
    , mInProcessBackend(nullptr)
{
}

BackendManager::~BackendManager()
{
    shutdownBackend();
}

QFileInfo BackendManager::preferredBackend(const QString &backend)
{
    QString backendFilter;
    const auto env_kscreen_backend = QString::fromUtf8(qgetenv("KSCREEN_BACKEND"));
    if (!env_kscreen_backend.isEmpty()) {
        backendFilter = env_kscreen_backend;
    } else {
        backendFilter = QStringLiteral("KWayland");
    }
    QFileInfo fallback;
    const auto backends = listBackends();
    for (const QFileInfo &f : backends) {
        // Here's the part where we do the match case-insensitive
        if (f.baseName().toLower() == QStringLiteral("ksc_%1").arg(backendFilter.toLower())) {
            return f;
        }
    }
    //     qCWarning(KSCREEN) << "No preferred backend found. KSCREEN_BACKEND is set to " << env_kscreen_backend;
    //     qCWarning(KSCREEN) << "falling back to " << fallback.fileName();
    return fallback;
}

QFileInfoList BackendManager::listBackends()
{
    // Compile a list of installed backends first
    const QString backendFilter = QStringLiteral("KSC_*");
    const QStringList paths = QCoreApplication::libraryPaths();
    QFileInfoList finfos;
    for (const QString &path : paths) {
        const QDir dir(path + QStringLiteral("/kf6/kscreen/"), backendFilter, QDir::SortFlags(QDir::QDir::Name), QDir::NoDotAndDotDot | QDir::Files);
        finfos.append(dir.entryInfoList());
    }
    return finfos;
}

void BackendManager::setBackendArgs(const QVariantMap &arguments)
{
    if (mBackendArguments != arguments) {
        mBackendArguments = arguments;
    }
}

QVariantMap BackendManager::getBackendArgs()
{
    return mBackendArguments;
}

KScreen::AbstractBackend *BackendManager::loadBackendPlugin(QPluginLoader *loader, const QString &name, const QVariantMap &arguments)
{
    const auto finfo = preferredBackend(name);
    loader->setFileName(finfo.filePath());
    QObject *instance = loader->instance();
    if (!instance) {
        qCDebug(KSCREEN) << loader->errorString();
        return nullptr;
    }

    auto backend = qobject_cast<KScreen::AbstractBackend *>(instance);
    if (backend) {
        backend->init(arguments);
        if (!backend->isValid()) {
            qCDebug(KSCREEN) << "Skipping" << backend->name() << "backend";
            delete backend;
            return nullptr;
        }
        // qCDebug(KSCREEN) << "Loaded" << backend->name() << "backend";
        return backend;
    } else {
        qCDebug(KSCREEN) << finfo.fileName() << "does not provide valid KScreen backend";
    }

    return nullptr;
}

KScreen::AbstractBackend *BackendManager::loadBackendInProcess(const QString &name)
{
    if (mInProcessBackend != nullptr && (name.isEmpty() || mInProcessBackend->name() == name)) {
        return mInProcessBackend;
    } else if (mInProcessBackend != nullptr && mInProcessBackend->name() != name) {
        shutdownBackend();
    }

    if (mLoader == nullptr) {
        mLoader = new QPluginLoader(this);
    }

    auto backend = BackendManager::loadBackendPlugin(mLoader, name, mBackendArguments);
    if (!backend) {
        return nullptr;
    }
    // qCDebug(KSCREEN) << "Connecting ConfigMonitor to backend.";
    ConfigMonitor::instance()->connectInProcessBackend(backend);
    mInProcessBackend = backend;
    setConfig(backend->config());
    return backend;
}

ConfigPtr BackendManager::config() const
{
    return mConfig;
}

void BackendManager::setConfig(ConfigPtr c)
{
    // qCDebug(KSCREEN) << "BackendManager::setConfig, outputs:" << c->outputs().count();
    mConfig = c;
}

void BackendManager::shutdownBackend()
{
    delete mLoader;
    mLoader = nullptr;
    delete mInProcessBackend;
    mInProcessBackend = nullptr;
}

#include "moc_backendmanager_p.cpp"
