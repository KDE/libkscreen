/*
 * SPDX-FileCopyrightText: 2014 Daniel Vratil <dvratil@redhat.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 */

#include "setconfigoperation.h"

#include "abstractbackend.h"
#include "backendmanager_p.h"
#include "config.h"
#include "configoperation_p.h"
#include "configserializer_p.h"
#include "kscreen_debug.h"
#include "output.h"

#include <QDBusPendingCall>
#include <QDBusPendingCallWatcher>

using namespace KScreen;

namespace KScreen
{
class SetConfigOperationPrivate : public ConfigOperationPrivate
{
    Q_OBJECT

public:
    explicit SetConfigOperationPrivate(const KScreen::ConfigPtr &config, ConfigOperation *qq);

    void backendReady(org::kde::kscreen::Backend *backend) override;
    void onConfigSet(QDBusPendingCallWatcher *watcher);
    void normalizeOutputPositions();

    KScreen::ConfigPtr config;

private:
    Q_DECLARE_PUBLIC(SetConfigOperation)
};

}

SetConfigOperationPrivate::SetConfigOperationPrivate(const ConfigPtr &config, ConfigOperation *qq)
    : ConfigOperationPrivate(qq)
    , config(config)
{
}

void SetConfigOperationPrivate::backendReady(org::kde::kscreen::Backend *backend)
{
    ConfigOperationPrivate::backendReady(backend);

    Q_Q(SetConfigOperation);

    if (!backend) {
        q->setError(tr("Failed to prepare backend"));
        q->emitResult();
        return;
    }

    const QVariantMap map = ConfigSerializer::serializeConfig(config).toVariantMap();
    if (map.isEmpty()) {
        q->setError(tr("Failed to serialize request"));
        q->emitResult();
        return;
    }

    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(backend->setConfig(map), this);
    connect(watcher, &QDBusPendingCallWatcher::finished, this, &SetConfigOperationPrivate::onConfigSet);
}

void SetConfigOperationPrivate::onConfigSet(QDBusPendingCallWatcher *watcher)
{
    Q_Q(SetConfigOperation);

    QDBusPendingReply<QVariantMap> reply = *watcher;
    watcher->deleteLater();

    if (reply.isError()) {
        q->setError(reply.error().message());
        q->emitResult();
        return;
    }

    config = ConfigSerializer::deserializeConfig(reply.value());
    if (!config) {
        q->setError(tr("Failed to deserialize backend response"));
    }

    q->emitResult();
}

SetConfigOperation::SetConfigOperation(const ConfigPtr &config, QObject *parent)
    : ConfigOperation(new SetConfigOperationPrivate(config, this), parent)
{
}

SetConfigOperation::~SetConfigOperation()
{
}

ConfigPtr SetConfigOperation::config() const
{
    Q_D(const SetConfigOperation);
    return d->config;
}

void SetConfigOperation::start()
{
    Q_D(SetConfigOperation);
    d->normalizeOutputPositions();
    if (BackendManager::instance()->method() == BackendManager::InProcess) {
        auto backend = d->loadBackend();
        backend->setConfig(d->config);
        emitResult();
    } else {
        d->requestBackend();
    }
}

void SetConfigOperationPrivate::normalizeOutputPositions()
{
    if (!config) {
        return;
    }
    int offsetX = INT_MAX;
    int offsetY = INT_MAX;
    const auto outputs = config->outputs();
    for (const KScreen::OutputPtr &output : outputs) {
        if (!output->isPositionable()) {
            continue;
        }
        offsetX = qMin(output->pos().x(), offsetX);
        offsetY = qMin(output->pos().y(), offsetY);
    }

    if (!offsetX && !offsetY) {
        return;
    }
    qCDebug(KSCREEN) << "Correcting output positions by:" << QPoint(offsetX, offsetY);
    for (const KScreen::OutputPtr &output : outputs) {
        if (!output->isConnected() || !output->isEnabled()) {
            continue;
        }
        QPoint newPos = QPoint(output->pos().x() - offsetX, output->pos().y() - offsetY);
        qCDebug(KSCREEN) << "Moved output from" << output->pos() << "to" << newPos;
        output->setPos(newPos);
    }
}

#include "setconfigoperation.moc"
