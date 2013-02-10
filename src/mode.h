/*************************************************************************************
 *  Copyright (C) 2012 by Alejandro Fiestas Olivares <afiestas@kde.org>              *
 *                                                                                   *
 *  This program is free software; you can redistribute it and/or                    *
 *  modify it under the terms of the GNU General Public License                      *
 *  as published by the Free Software Foundation; either version 2                   *
 *  of the License, or (at your option) any later version.                           *
 *                                                                                   *
 *  This program is distributed in the hope that it will be useful,                  *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of                   *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                    *
 *  GNU General Public License for more details.                                     *
 *                                                                                   *
 *  You should have received a copy of the GNU General Public License                *
 *  along with this program; if not, write to the Free Software                      *
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA   *
 *************************************************************************************/

#ifndef MODE_CONFIG_H
#define MODE_CONFIG_H

#include "kscreen_export.h"

#include <QtCore/QSize>
#include <QtCore/QHash>
#include <QtCore/QObject>
#include <QtCore/QMetaType>

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
};

typedef QHash<QString, Mode*> ModeList;

} //KSCreen namespace

Q_DECLARE_METATYPE(KScreen::ModeList)

#endif //MODE_H
