/*************************************************************************************
 *  Copyright (C) 2012 by Alejandro Fiestas Olivares <afiestas@kde.org>              *
 *                                                                                   *
 *  This library is free software; you can redistribute it and/or                    *
 *  modify it under the terms of the GNU Lesser General Public                       *
 *  License as published by the Free Software Foundation; either                     *
 *  version 2.1 of the License, or (at your option) any later version.               *
 *                                                                                   *
 *  This library is distributed in the hope that it will be useful,                  *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of                   *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU                *
 *  Lesser General Public License for more details.                                  *
 *                                                                                   *
 *  You should have received a copy of the GNU Lesser General Public                 *
 *  License along with this library; if not, write to the Free Software              *
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA       *
 *************************************************************************************/

#ifndef MODE_CONFIG_H
#define MODE_CONFIG_H

#include "kscreen_export.h"

#include <QtCore/QSize>
#include <QtCore/QHash>
#include <QtCore/QObject>
#include <QtCore/QMetaType>
#include <QtCore/QDebug>

namespace KScreen {

class KSCREEN_EXPORT Mode : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString id READ id NOTIFY modeChanged)
    Q_PROPERTY(QString name READ name WRITE setName NOTIFY modeChanged)
    Q_PROPERTY(QSize size READ size WRITE setSize NOTIFY modeChanged)
    Q_PROPERTY(float refreshRate READ refreshRate WRITE setRefreshRate NOTIFY modeChanged)

    public:
        explicit Mode(QObject *parent = 0);
        virtual ~Mode();

        Mode* clone() const;

        const QString id() const;
        void setId(const QString &id);

        QString name() const;
        void setName(const QString& name);

        QSize size() const;
        void setSize(const QSize& size);

        float refreshRate() const;
        void setRefreshRate(float refresh);

    Q_SIGNALS:
        void modeChanged();

    private:
        class Private;
        Private * const d;

        Mode(Private *dd);
};

typedef QHash<QString, Mode*> ModeList;

} //KSCreen namespace

KSCREEN_EXPORT QDebug operator<<(QDebug dbg, const KScreen::Mode *mode);

Q_DECLARE_METATYPE(KScreen::ModeList)

#endif //MODE_H
