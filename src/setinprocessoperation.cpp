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

#include "setinprocessoperation.h"

#include "abstractbackend.h"
#include "backendmanager_p.h"
#include "configoperation_p.h"
#include "config.h"
#include "debug_p.h"


using namespace KScreen;

namespace KScreen
{

class SetInProcessOperationPrivate : public ConfigOperationPrivate
{
    Q_OBJECT

public:
    explicit SetInProcessOperationPrivate(const KScreen::ConfigPtr &config, ConfigOperation* qq);
    void loadBackend();

    KScreen::ConfigPtr config;
    KScreen::AbstractBackend* backend;

private:
    Q_DECLARE_PUBLIC(SetInProcessOperation)
};

}

SetInProcessOperationPrivate::SetInProcessOperationPrivate(const ConfigPtr &config, ConfigOperation* qq)
    : ConfigOperationPrivate(qq)
    , config(config)
    , backend(nullptr)
{
}


SetInProcessOperation::SetInProcessOperation(const ConfigPtr &config, QObject* parent)
    : ConfigOperation(new SetInProcessOperationPrivate(config, this), parent)
{
}

SetInProcessOperation::~SetInProcessOperation()
{
}

ConfigPtr SetInProcessOperation::config() const
{
    Q_D(const SetInProcessOperation);
    return d->config;
}

void SetInProcessOperation::start()
{
    Q_D(SetInProcessOperation);
    d->loadBackend();
}

void SetInProcessOperationPrivate::loadBackend()
{
    Q_Q(SetInProcessOperation);
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
    backend->setConfig(config);

    q->emitResult();
//     connect(backend, &AbstractBackend::configChanged, [this, q](const KScreen::ConfigPtr newconfig) {
//         //qDebug() << "Yay, configChanged: " << config->outputs();
//         q->emitResult();
//     });
//
}

#include "setinprocessoperation.moc"
