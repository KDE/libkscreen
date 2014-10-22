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

#ifndef KSCREEN_SETCONFIGOPERATION_H
#define KSCREEN_SETCONFIGOPERATION_H

#include "configoperation.h"
#include "types.h"
#include "kscreen_export.h"

namespace KScreen {

class SetConfigOperationPrivate;

class KSCREEN_EXPORT SetConfigOperation : public KScreen::ConfigOperation
{
    Q_OBJECT
public:
    explicit SetConfigOperation(const KScreen::ConfigPtr &config, QObject* parent = 0);
    ~SetConfigOperation();

    KScreen::ConfigPtr config() const;

protected:
    void start();

private:
    Q_DECLARE_PRIVATE(SetConfigOperation)
};

}

#endif // KSCREEN_SETCONFIGOPERATION_H
