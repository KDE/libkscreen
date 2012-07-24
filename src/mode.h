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

#ifndef MODE_H
#define MODE_H

#include "kscreen_export.h"

#include <QtCore/QSize>
#include <QtCore/QHash>
#include <QtCore/QObject>
#include <QtCore/QMetaType>

class KSCREEN_EXPORT Mode : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QSize size READ size)
    Q_PROPERTY(float refreshrate READ refreshRate)

    public:
        explicit Mode(QObject *parent = 0);
        virtual ~Mode();
        QSize size() const;
        float refreshRate() const;

    private:
        QSize m_size;
        float m_rate;
};

typedef QHash<int, Mode*> ModeList;
Q_DECLARE_METATYPE(ModeList);

#endif //MODE_H