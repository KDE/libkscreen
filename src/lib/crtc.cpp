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

#include "crtc.h"
#include "screen.h"
#include "mode.h"

#include <QDebug>
#include <QX11Info>

namespace QRandR {

Crtc::Crtc(Screen* parent, RRCrtc id)
: m_id(id)
, m_info(0)
, m_parent(parent)
, m_mode(0)
{
}

Crtc::~Crtc()
{
}

RRCrtc Crtc::id() const
{
    return m_id;
}

QRect Crtc::rect()
{
    if (m_rect.isEmpty()) {
        XRRCrtcInfo *currentInfo = info();
        m_rect.setRect(currentInfo->x, currentInfo->y, currentInfo->width, currentInfo->height);
    }

    return m_rect;
}

Mode* Crtc::mode()
{
    if (!m_mode) {
        m_mode = m_parent->mode(info()->mode);
    }

    return m_mode;
}

XRRCrtcInfo* Crtc::info()
{
    if (!m_info) {
        m_info = XRRGetCrtcInfo(QX11Info::display(), m_parent->resources(), m_id);
    }

    return m_info;
}

}