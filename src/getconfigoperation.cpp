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
#include "output.h"
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
    GetConfigOperationPrivate(GetConfigOperation::Options options, GetConfigOperation *qq);

    virtual void backendReady(org::kde::kscreen::Backend* backend);
    void onConfigReceived(QDBusPendingCallWatcher *watcher);
    void onEDIDReceived(QDBusPendingCallWatcher *watcher);

public:
    GetConfigOperation::Options options;
    ConfigPtr config;
    int pendingEDIDs;

private:
    Q_DECLARE_PUBLIC(GetConfigOperation)
};

}

GetConfigOperationPrivate::GetConfigOperationPrivate(GetConfigOperation::Options options, GetConfigOperation* qq)
    : ConfigOperationPrivate(qq)
    , options(options)
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

    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(backend->getConfig(), this);
    watcher->setProperty("backend", QVariant::fromValue(backend));
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

    if (options & GetConfigOperation::NoEDID) {
        q->emitResult();
        return;
    }

    pendingEDIDs = 0;
    org::kde::kscreen::Backend *backend = watcher->property("backend").value<org::kde::kscreen::Backend*>();
    Q_FOREACH (const OutputPtr &output, config->outputs()) {
        QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(backend->getEdid(output->id()), this);
        watcher->setProperty("outputId", output->id());
        connect(watcher, &QDBusPendingCallWatcher::finished,
                this, &GetConfigOperationPrivate::onEDIDReceived);
        ++pendingEDIDs;
    }
}

void GetConfigOperationPrivate::onEDIDReceived(QDBusPendingCallWatcher* watcher)
{
    Q_Q(GetConfigOperation);

    QDBusPendingReply<QByteArray> reply = *watcher;
    watcher->deleteLater();
    if (reply.isError()) {
        q->setError(reply.error().message());
        q->emitResult();
        return;
    }

    const QByteArray edidData = reply.value();
    const int outputId = watcher->property("outputId").toInt();

    OutputList outputs = config->outputs();
    outputs[outputId]->setEdid(edidData);
    config->setOutputs(outputs);

    if (--pendingEDIDs == 0) {
        q->emitResult();
    }
}



GetConfigOperation::GetConfigOperation(Options options, QObject* parent)
    : ConfigOperation(new GetConfigOperationPrivate(options, this), parent)
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
