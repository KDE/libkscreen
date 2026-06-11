/*
 * SPDX-FileCopyrightText: 2014 Daniel Vratil <dvratil@redhat.com>
 * SPDX-FileCopyrightText: 2015 Sebastian Kügler <sebas@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 */

#include "backendmanager_p.h"

#include "../backends/fake/fake.h"
#include "../backends/kwayland/waylandbackend.h"
#include "abstractbackend.h"
#include "configmonitor.h"

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
    : mInProcessBackend(nullptr)
{
}

BackendManager::~BackendManager()
{
    shutdownBackend();
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

KScreen::AbstractBackend *BackendManager::loadBackendInProcess()
{
    if (mInProcessBackend) {
        return mInProcessBackend;
    }

    if (qgetenv("KSCREEN_BACKEND") == QLatin1StringView("Fake")) {
        mInProcessBackend = new Fake;
        mInProcessBackend->init(mBackendArguments);
    } else {
        mInProcessBackend = new WaylandBackend;
        mInProcessBackend->init(mBackendArguments);
    }

    ConfigMonitor::instance()->connectInProcessBackend(mInProcessBackend);
    setConfig(mInProcessBackend->config());
    return mInProcessBackend;
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
    delete mInProcessBackend;
    mInProcessBackend = nullptr;
}

#include "moc_backendmanager_p.cpp"
