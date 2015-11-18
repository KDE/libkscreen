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

#include "configoperation.h"
#include "configoperation_p.h"
#include "backendmanager_p.h"

#include "getconfigoperation.h"
#include "inprocessconfigoperation.h"
#include "setconfigoperation.h"
#include "debug_p.h"

using namespace KScreen;

ConfigOperationPrivate::ConfigOperationPrivate(ConfigOperation* qq)
    : QObject()
    , isExec(false)
    , q_ptr(qq)
    , backend(nullptr)
{
}

ConfigOperationPrivate::~ConfigOperationPrivate()
{
}

ConfigOperation* ConfigOperation::create(Options options)
{
    return new GetConfigOperation(options);
}

ConfigOperation* ConfigOperation::setOperation(KScreen::ConfigPtr newconfig)
{
    return new SetConfigOperation(newconfig);
}

void ConfigOperationPrivate::requestBackend()
{
    connect(BackendManager::instance(), &BackendManager::backendReady,
            this, &ConfigOperationPrivate::backendReady);
    BackendManager::instance()->requestBackend();
}

void ConfigOperationPrivate::backendReady(org::kde::kscreen::Backend *backend)
{
    Q_UNUSED(backend);

    disconnect(BackendManager::instance(), &BackendManager::backendReady,
               this, &ConfigOperationPrivate::backendReady);
}

void ConfigOperationPrivate::doEmitResult()
{
    Q_Q(ConfigOperation);

    Q_EMIT q->finished(q);

    // Don't call deleteLater() when this operation is running from exec()
    // because then the operation will be deleted when we return control to
    // the nested QEventLoop in exec() (i.e. before loop.exec() returns)
    // and subsequent hasError() call references deleted "this". Instead we
    // shedule the operation for deletion manually in exec(), so that it will
    // be deleted when control returns to parent event loop (or QApplication).
    if (!isExec) {
        q->deleteLater();
    }
}

ConfigOperation::ConfigOperation(ConfigOperationPrivate* dd, QObject* parent)
    : QObject(parent)
    , d_ptr(dd)
{
    const bool ok = QMetaObject::invokeMethod(this, "start", Qt::QueuedConnection);
    Q_ASSERT(ok);
    Q_UNUSED(ok);
}

ConfigOperation::~ConfigOperation()
{
    delete d_ptr;
}

bool ConfigOperation::hasError() const
{
    Q_D(const ConfigOperation);
    return !d->error.isEmpty();
}

QString ConfigOperation::errorString() const
{
    Q_D(const ConfigOperation);
    return d->error;
}

void ConfigOperation::setError(const QString& error)
{
    Q_D(ConfigOperation);
    d->error = error;
}

void ConfigOperation::emitResult()
{
    Q_D(ConfigOperation);
    const bool ok = QMetaObject::invokeMethod(d, "doEmitResult", Qt::QueuedConnection);
    Q_ASSERT(ok);
    Q_UNUSED(ok);
}

bool ConfigOperation::exec()
{
    Q_D(ConfigOperation);

    QEventLoop loop;
    connect(this, &ConfigOperation::finished,
            [&](ConfigOperation *op) {
                Q_UNUSED(op);
                loop.quit();
            });

    d->isExec = true;
    loop.exec(QEventLoop::ExcludeUserInputEvents);

    // Schedule the operation for deletion, see doEmitResult()
    deleteLater();
    return !hasError();
}

void ConfigOperationPrivate::loadBackend()
{
    Q_Q(ConfigOperation);
    QVariantMap arguments;
    const QString &name = qgetenv("KSCREEN_BACKEND").constData();
    auto beargs = QString::fromLocal8Bit(qgetenv("KSCREEN_BACKEND_ARGS"));
    if (beargs.startsWith("TEST_DATA=")) {
        arguments["TEST_DATA"] = beargs.remove("TEST_DATA=");
    }
    backend = KScreen::BackendManager::instance()->loadBackendInProcess(name, arguments);
    if (backend == nullptr) {
        qCDebug(KSCREEN) << "plugin does not provide valid KScreen backend";
        q->setError("Plugin does not provide valid KScreen backend");
        q->emitResult();
        return;
    }
    qDebug() << "Calling Backend::setConfig().";
    //     connect(backend, &AbstractBackend::configChanged, [this, q](const KScreen::ConfigPtr newconfig) {
    //         //qDebug() << "Yay, configChanged: " << config->outputs();
    //         q->emitResult();
    //     });
    //
}
