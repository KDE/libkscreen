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
#include "kscreen_debug.h"
#include "output.h"

#include <QDir>
#include <QFutureWatcher>
#include <ranges>

using namespace KScreen;

namespace KScreen
{
class SetConfigOperationPrivate : public ConfigOperationPrivate
{
    Q_OBJECT

public:
    explicit SetConfigOperationPrivate(const KScreen::ConfigPtr &config, ConfigOperation *qq);

    void fixPriorities();

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
    d->fixPriorities();
    auto backend = d->loadBackend();
    QFutureWatcher<SetConfigResult> *watcher = new QFutureWatcher<SetConfigResult>(this);
    connect(watcher, &QFutureWatcher<SetConfigResult>::finished, this, [this, watcher]() {
        watcher->deleteLater();
        const SetConfigResult result = watcher->result();
        if (!result.has_value()) {
            setError(result.error());
        }
        emitResult();
    });

    QFuture<SetConfigResult> pendingResult = backend->setConfig(d->config);
    watcher->setFuture(pendingResult);
}

void SetConfigOperationPrivate::fixPriorities()
{
    if (!config || !(config->supportedFeatures() & Config::Feature::PrimaryDisplay)) {
        return;
    }
    // Here we make sure that among enabled outputs, each
    // priority value is unique
    const auto outputs = config->outputs();
    auto enabled = outputs | std::views::filter([](const auto &output) {
                       return output->isEnabled();
                   })
        | std::ranges::to<QList>();
    if (enabled.isEmpty()) {
        return;
    }
    std::ranges::sort(enabled, [](const auto &left, const auto &right) {
        return left->priority() < right->priority();
    });
    uint32_t priority = enabled.front()->priority();
    for (const auto &output : enabled | std::views::drop(1)) {
        if (output->priority() <= priority) {
            output->setPriority(priority + 1);
        }
        priority++;
    }
}

#include "setconfigoperation.moc"

#include "moc_setconfigoperation.cpp"
