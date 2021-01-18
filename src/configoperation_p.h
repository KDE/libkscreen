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

#ifndef CONFIGOPERATIONPRIVATE_H
#define CONFIGOPERATIONPRIVATE_H

#include <QObject>

#include "abstractbackend.h"
#include "backendinterface.h"
#include "configoperation.h"

namespace KScreen
{
class ConfigOperationPrivate : public QObject
{
    Q_OBJECT

public:
    explicit ConfigOperationPrivate(ConfigOperation *qq);
    ~ConfigOperationPrivate() override;

    // For out-of-process
    void requestBackend();
    virtual void backendReady(org::kde::kscreen::Backend *backend);

    // For in-process
    KScreen::AbstractBackend *loadBackend();

public Q_SLOTS:
    void doEmitResult();

private:
    QString error;
    bool isExec;

protected:
    ConfigOperation *const q_ptr;
    Q_DECLARE_PUBLIC(ConfigOperation)
};

}
#endif
