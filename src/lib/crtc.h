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

#ifndef QCRTC_H
#define QCRTC_H

#include "xlibandxrandr.h"

#include <QtCore/QObject>
#include <QRect>

namespace QRandR {

class Screen;

class Crtc : public QObject
{
    Q_OBJECT

    public:
        Crtc (Screen *parent, RRCrtc id);
        virtual ~Crtc();

        QRect rect();
        RRCrtc id() const;

    private:
        XRRCrtcInfo* info();

    private:
        QRect m_rect;
        RRCrtc m_id;
        XRRCrtcInfo *m_info;
        Screen *m_parent;
};

#endif //QCRTC_H

}