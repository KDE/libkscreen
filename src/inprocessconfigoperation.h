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

#ifndef KSCREEN_INPROCESSCONFIGOPERATION_H
#define KSCREEN_INPROCESSCONFIGOPERATION_H

#include "configoperation.h"
#include "types.h"
#include "kscreen_export.h"

class QDBusPendingCallWatcher;
class OrgKdeKscreenBackend;

namespace KScreen {

class InProcessConfigOperationPrivate;

class KSCREEN_EXPORT InProcessConfigOperation : public KScreen::ConfigOperation
{
    Q_OBJECT

public:

    explicit InProcessConfigOperation(Options options = NoOptions, QObject* parent = 0);
    ~InProcessConfigOperation();

    KScreen::ConfigPtr config() const override;

protected:
    void start();

private:
    Q_DECLARE_PRIVATE(InProcessConfigOperation)
};
}

#endif // KSCREEN_GETCONFIGOPERATION_H
