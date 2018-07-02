/*
 * <one line to give the library's name and an idea of what it does.>
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

#ifndef KSCREEN_CONFIGOPERATION_H
#define KSCREEN_CONFIGOPERATION_H

#include <QObject>

#include "kscreen_export.h"
#include "types.h"

namespace KScreen {

class ConfigOperationPrivate;

class KSCREEN_EXPORT ConfigOperation : public QObject
{
    Q_OBJECT

public:
    enum Option {
        NoOptions,
        NoEDID
    };
    Q_DECLARE_FLAGS(Options, Option)

    ~ConfigOperation() override;

    bool hasError() const;
    QString errorString() const;

    virtual KScreen::ConfigPtr config() const = 0;

    bool exec();

Q_SIGNALS:
    void finished(ConfigOperation *operation);

protected:
    explicit ConfigOperation(ConfigOperationPrivate *dd, QObject *parent = nullptr);

    void setError(const QString &error);
    void emitResult();

protected Q_SLOTS:
    virtual void start() = 0;

protected:
    ConfigOperationPrivate * const d_ptr;
    Q_DECLARE_PRIVATE(ConfigOperation)
};
}

#endif // KSCREEN_CONFIGOPERATION_H
