/***************************************************************************
 *   Copyright (C) 2012 by Daniel Nicoletti <dantti12@gmail.com>           *
 *             (C) 2012 by Dan Vrátil <dvratil@redhat.com>                 *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; see the file COPYING. If not, write to       *
 *   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,  *
 *   Boston, MA 02110-1301, USA.                                           *
 ***************************************************************************/

#ifndef KSCREEN_EDID_H
#define KSCREEN_EDID_H

#include "kscreen_export.h"

#include <QtCore/QObject>
#include <QtCore/QtGlobal>
#include <QtGui/QQuaternion>

namespace KScreen
{

class KSCREEN_EXPORT Edid: public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString deviceId READ deviceId CONSTANT)
    Q_PROPERTY(QString name READ name CONSTANT)
    Q_PROPERTY(QString vendor READ vendor CONSTANT)
    Q_PROPERTY(QString serial READ serial CONSTANT)
    Q_PROPERTY(QString eisaId READ eisaId CONSTANT)
    Q_PROPERTY(QString hash READ hash CONSTANT)
    Q_PROPERTY(uint width READ width CONSTANT)
    Q_PROPERTY(uint height READ height CONSTANT)
    Q_PROPERTY(qreal gamma READ gamma CONSTANT)
    Q_PROPERTY(QQuaternion red READ red CONSTANT)
    Q_PROPERTY(QQuaternion green READ green CONSTANT)
    Q_PROPERTY(QQuaternion blue READ blue CONSTANT)
    Q_PROPERTY(QQuaternion white READ white CONSTANT)

public:
    explicit Edid();
    explicit Edid(const quint8 *data, size_t length, QObject *parent = 0);
    virtual ~Edid();

    bool isValid() const;

    QString deviceId(const QString &fallbackName = QString()) const;
    QString name() const;
    QString vendor() const;
    QString serial() const;
    QString eisaId() const;
    QString hash() const;
    QString pnpId() const;
    uint width() const;
    uint height() const;
    qreal gamma() const;
    QQuaternion red() const;
    QQuaternion green() const;
    QQuaternion blue() const;
    QQuaternion white() const;

private:
    class Private;
    Private * const d;
};

}

Q_DECLARE_METATYPE(KScreen::Edid*)

#endif // EDID_H
