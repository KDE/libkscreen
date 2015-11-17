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

#include "setconfigoperation.h"

#include "abstractbackend.h"
#include "backendmanager_p.h"
#include "configoperation_p.h"
#include "config.h"
#include "configserializer_p.h"

#include <QDBusPendingCallWatcher>
#include <QDBusPendingCall>

using namespace KScreen;

namespace KScreen
{

class SetConfigOperationPrivate : public ConfigOperationPrivate
{
    Q_OBJECT

public:
    explicit SetConfigOperationPrivate(const KScreen::ConfigPtr &config, ConfigOperation* qq);

    void backendReady(org::kde::kscreen::Backend* backend);
    void onConfigSet(QDBusPendingCallWatcher *watcher);

    KScreen::ConfigPtr config;

private:
    Q_DECLARE_PUBLIC(SetConfigOperation)
};

}

SetConfigOperationPrivate::SetConfigOperationPrivate(const ConfigPtr &config, ConfigOperation* qq)
    : ConfigOperationPrivate(qq)
    , config(config)
{
    Q_Q(SetConfigOperation);
}

void SetConfigOperationPrivate::backendReady(org::kde::kscreen::Backend* backend)
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
    connect(watcher, &QDBusPendingCallWatcher::finished,
            this, &SetConfigOperationPrivate::onConfigSet);
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


SetConfigOperation::SetConfigOperation(const ConfigPtr &config, QObject* parent)
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
    if (BackendManager::instance()->mode() == BackendManager::InProcess) {
        const QString e = QStringLiteral("SetConfigOperation is not suitable for InProcess backends. Clients should use ConfigOperation::setOperation(config).");
        qWarning() << e;
        setError(e);
        emitResult();
        return;
    }

    d->requestBackend();
}

#include "setconfigoperation.moc"
