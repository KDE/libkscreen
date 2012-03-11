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

#ifndef QOUTPUT_H
#define QOUTPUT_H

#include "xlibandxrandr.h"

#include <QtCore/QObject>
#include <QtCore/QRect>

namespace QRandR {

class Mode;
class Screen;

class Output : public QObject
{
    Q_OBJECT

    public:
        Output (Screen *parent, RROutput id);
        virtual ~Output();

        const QString& name();
        QList<Mode*> modes();
        bool isConnected();
        bool isEnabled();
        RROutput id() const;

    private:
        XRROutputInfo* info();

    private:
        QString m_name;
        QRect m_rect;
        RROutput m_id;
        XRROutputInfo *m_info;
        Screen *m_parent;

        QList<Mode *> m_modes;
};

}

#endif //QOUTPUT_H