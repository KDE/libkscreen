/*
 * SPDX-FileCopyrightText: 2014 Daniel Vratil <dvratil@redhat.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 */

#include "getconfigoperation.h"
#include "backendinterface.h"
#include "backendmanager_p.h"
#include "config.h"
#include "configoperation_p.h"
#include "output.h"

using namespace KScreen;

namespace KScreen
{
class GetConfigOperationPrivate : public ConfigOperationPrivate
{
    Q_OBJECT

public:
    GetConfigOperationPrivate(GetConfigOperation::Options options, GetConfigOperation *qq);

public:
    GetConfigOperation::Options options;
    ConfigPtr config;
    // For in-process
    void loadEdid(KScreen::AbstractBackend *backend);

    // For out-of-process
    int pendingEDIDs;
    QPointer<org::kde::kscreen::Backend> mBackend;

private:
    Q_DECLARE_PUBLIC(GetConfigOperation)
};

}

GetConfigOperationPrivate::GetConfigOperationPrivate(GetConfigOperation::Options options, GetConfigOperation *qq)
    : ConfigOperationPrivate(qq)
    , options(options)
{
}

GetConfigOperation::GetConfigOperation(Options options, QObject *parent)
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
    auto backend = d->loadBackend();
    if (!backend) {
        return; // loadBackend() already set error and called emitResult() for us
    }
        d->config = backend->config()->clone();
        d->loadEdid(backend);
        emitResult();
}

void GetConfigOperationPrivate::loadEdid(KScreen::AbstractBackend *backend)
{
    if (options & KScreen::ConfigOperation::NoEDID) {
        return;
    }
    if (!config) {
        return;
    }
    auto outputs = config->outputs();
    for (auto it = outputs.begin(); it != outputs.end(); ++it) {
        auto output = *it;
        if (output->edid() == nullptr) {
            const QByteArray edidData = backend->edid(output->id());
            output->setEdid(edidData);
        }
    }
}

#include "getconfigoperation.moc"

#include "moc_getconfigoperation.cpp"
