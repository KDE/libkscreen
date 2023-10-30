/*
 * SPDX-FileCopyrightText: 2014 Daniel Vratil <dvratil@redhat.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 */

#pragma once

#include "configoperation.h"
#include "kscreen_export.h"
#include "types.h"

namespace KScreen
{
class SetConfigOperationPrivate;

class KSCREEN_EXPORT SetConfigOperation : public KScreen::ConfigOperation
{
    Q_OBJECT
public:
    explicit SetConfigOperation(const KScreen::ConfigPtr &config, QObject *parent = nullptr);
    ~SetConfigOperation() override;

    KScreen::ConfigPtr config() const override;

protected:
    void start() override;

private:
    Q_DECLARE_PRIVATE(SetConfigOperation)
};

}
