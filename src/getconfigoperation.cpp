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

#include "getconfigoperation.h"
#include "configoperation_p.h"
#include "config.h"
#include "backendmanager_p.h"
#include "configserializer_p.h"
#include "backendinterface.h"

using namespace KScreen;

namespace KScreen
{

class GetConfigOperationPrivate : public ConfigOperationPrivate
{
    Q_OBJECT

public:
    GetConfigOperationPrivate(GetConfigOperation *qq);

    virtual void backendReady(org::kde::kscreen::Backend* backend);
    void onConfigReceived(QDBusPendingCallWatcher *watcher);

public:
    ConfigPtr config;

private:
    Q_DECLARE_PUBLIC(GetConfigOperation)
};

}

GetConfigOperationPrivate::GetConfigOperationPrivate(GetConfigOperation* qq)
    : ConfigOperationPrivate(qq)
{
}

void GetConfigOperationPrivate::backendReady(org::kde::kscreen::Backend* backend)
{
    ConfigOperationPrivate::backendReady(backend);

    Q_Q(GetConfigOperation);

    if (!backend) {
        q->setError(tr("Failed to prepare backend"));
        q->emitResult();
        return;
    }

    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(backend->getConfig(), q);
    connect(watcher, &QDBusPendingCallWatcher::finished,
            this, &GetConfigOperationPrivate::onConfigReceived);
}

void GetConfigOperationPrivate::onConfigReceived(QDBusPendingCallWatcher *watcher)
{
    Q_Q(GetConfigOperation);

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


GetConfigOperation::GetConfigOperation(QObject* parent)
    : ConfigOperation(new GetConfigOperationPrivate(this), parent)
{
}

GetConfigOperation::~GetConfigOperation()
{
}

KScreen::ConfigPtr GetConfigOperation::config() const
{
    Q_D(const GetConfigOperation);
    return d->config;
}

void GetConfigOperation::start()
{
    Q_D(GetConfigOperation);
    d->requestBackend();
}


#include "getconfigoperation.moc"
