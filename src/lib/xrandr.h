/*************************************************************************************
 *  Copyright (C) 2011 by Alex Fiestas <afiestas@kde.org>                            *
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


#ifndef XRANDR_H
#define XRANDR_H

#include "xlibandxrandr.h"

#include <QObject>
#include <QPair>

namespace QRandR {
    class Screen;
}

class XRandR : public QObject
{
    public:
        static XRandR *self();

        QPair<int, int> version();
        QList<QRandR::Screen *> screens();

    public:
        static bool has_1_3;

    private:
        XRandR();
        void setVersion(int major, int minor);
        void setDisplay(Display *display);

    private:
        static XRandR *s_instance;

        Display *m_display;
        QPair<int,int> m_version;
        QList<QRandR::Screen *> m_screens;
};

#endif // XRANDR_H
