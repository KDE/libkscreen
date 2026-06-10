/*
 * SPDX-FileCopyrightText: 2014 Daniel Vratil <dvratil@redhat.com>
 * SPDX-FileCopyrightText: 2015 Sebastian Kügler <sebas@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 */

#include "configoperation.h"

#include "backendmanager_p.h"
#include "configoperation_p.h"

#include "kscreen_debug.h"

using namespace KScreen;

ConfigOperationPrivate::ConfigOperationPrivate(ConfigOperation *qq)
    : QObject()
    , isExec(false)
    , q_ptr(qq)
{
}

ConfigOperationPrivate::~ConfigOperationPrivate()
{
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

ConfigOperation::ConfigOperation(ConfigOperationPrivate *dd, QObject *parent)
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

void ConfigOperation::setError(const QString &error)
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
    connect(this, &ConfigOperation::finished, this, [&](ConfigOperation *op) {
        Q_UNUSED(op);
        loop.quit();
    });

    d->isExec = true;
    loop.exec(QEventLoop::ExcludeUserInputEvents);

    // Schedule the operation for deletion, see doEmitResult()
    deleteLater();
    return !hasError();
}

KScreen::AbstractBackend *ConfigOperationPrivate::loadBackend()
{
    Q_Q(ConfigOperation);
    auto backend = KScreen::BackendManager::instance()->loadBackendInProcess();
    if (backend == nullptr) {
        const QString &e = QStringLiteral("Plugin does not provide valid KScreen backend");
        qCDebug(KSCREEN) << e;
        q->setError(e);
        q->emitResult();
    }
    return backend;
}

#include "moc_configoperation.cpp"

#include "moc_configoperation_p.cpp"
