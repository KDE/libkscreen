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

#ifndef SCREEN_CONFIG_H
#define SCREEN_CONFIG_H

#include "kscreen_export.h"

#include <QtCore/QSize>
#include <QtCore/QObject>

class KSCREEN_EXPORT CScreen : public QObject
{
    Q_OBJECT

    public:
        Q_PROPERTY(int id READ id CONSTANT);
        Q_PROPERTY(QSize currentSize READ currentSize CONSTANT);
        Q_PROPERTY(QSize minSize READ minSize CONSTANT);
        Q_PROPERTY(QSize maxSize READ maxSize CONSTANT);

        CScreen(int screenId, QObject *parent = 0);
        virtual ~CScreen();

        int id() const;

        QSize currentSize() const;
        void setCurrentSize(const QSize& currentSize);

        QSize minSize() const;
        void setMinSize(const QSize& minSize);

        QSize maxSize() const;
        void setMaxSize(const QSize& maxSize);

    private:
        int m_id;
        QSize m_currentSize;
        QSize m_minSize;
        QSize m_maxSize;
};

#endif //SCREEN_H
