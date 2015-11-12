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

#include "inprocessconfigoperation.h"

#include "abstractbackend.h"
#include "configoperation_p.h"
#include "config.h"
#include "debug_p.h"
#include "output.h"
#include "backendmanager_p.h"
#include "configserializer_p.h"
#include "backendinterface.h"

#include <QX11Info>

#include <memory>

using namespace KScreen;

namespace KScreen
{

class InProcessConfigOperationPrivate : public ConfigOperationPrivate
{
    Q_OBJECT

public:
    InProcessConfigOperationPrivate(ConfigOperation::Options options, InProcessConfigOperation *qq);

    void loadBackend();
    void loadEdid();

public:
    InProcessConfigOperation::Options options;
    ConfigPtr config;
    KScreen::AbstractBackend* backend;
    QPluginLoader *mLoader;

private:
    Q_DECLARE_PUBLIC(InProcessConfigOperation)
};

}

InProcessConfigOperationPrivate::InProcessConfigOperationPrivate(ConfigOperation::Options options, InProcessConfigOperation* qq)
    : ConfigOperationPrivate(qq)
    , options(options)
    , backend(nullptr)
    , mLoader(nullptr)
{
}

void InProcessConfigOperation::start()
{
    Q_D(InProcessConfigOperation);
    d->loadBackend();
}

InProcessConfigOperation::InProcessConfigOperation(Options options, QObject* parent)
    : ConfigOperation(new InProcessConfigOperationPrivate(options, this), parent)
{
}

InProcessConfigOperation::~InProcessConfigOperation()
{
    qDebug() << "Deleting backend, resetting config";
    Q_D(InProcessConfigOperation);
    KScreen::ConfigPtr cfg;
    BackendManager::instance()->setConfig(cfg);
    delete d->backend;
}

KScreen::ConfigPtr InProcessConfigOperation::config() const
{
    Q_D(const InProcessConfigOperation);
    return d->config;
}

void InProcessConfigOperationPrivate::loadBackend()
{
    Q_Q(InProcessConfigOperation);
    QVariantMap arguments;
    const QString &name = qgetenv("KSCREEN_BACKEND").constData();
    auto beargs = QString::fromLocal8Bit(qgetenv("KSCREEN_BACKEND_ARGS"));
    if (beargs.startsWith("TEST_DATA=")) {
        //"TEST_DATA=" = "multipleclone.json");
        arguments["TEST_DATA"] = beargs.remove("TEST_DATA=");
    }

    backend = KScreen::BackendManager::instance()->loadBackend(name, arguments);
    if (backend == nullptr) {
        qCDebug(KSCREEN) << "plugin does not provide valid KScreen backend";
        //q->setError(finfo.fileName() + "does not provide valid KScreen backend");
        q->setError("Plugin does not provide valid KScreen backend");
        q->emitResult();
        return;
    }
    config = backend->config();
    KScreen::BackendManager::instance()->mInProcessBackend = backend;
    KScreen::BackendManager::instance()->setConfig(config);
    loadEdid();
    q->emitResult();
    return;
}

void InProcessConfigOperationPrivate::loadEdid()
{
    Q_Q(InProcessConfigOperation);
    if (options & KScreen::ConfigOperation::NoEDID) {
        return;

    }
    Q_FOREACH (auto output, config->outputs()) {
        const QByteArray edidData = backend->edid(output->id());
        output->setEdid(edidData);
    }
}


#include "inprocessconfigoperation.moc"
