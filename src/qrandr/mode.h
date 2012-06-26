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

#ifndef QMODE_H
#define QMODE_H

#include "xlibandxrandr.h"

#include <QtCore/QSize>
#include <QtCore/QObject>

namespace QRandR {

class Screen;
class Output;

class Mode : public QObject
{
    Q_OBJECT

    public:
        Mode (Screen *parent, XRRModeInfo *info);
        virtual ~Mode();

        RRMode id();
        const QString& name();
        const QSize& size();
        float rate();

    private:
        float m_rate;
        QSize m_size;
        QString m_name;
        Screen *m_parent;
        XRRModeInfo *m_info;
};

#endif //QMODE_H

}