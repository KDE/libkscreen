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

#include "output.h"
#include "screen.h"
#include "output.h"
#include "mode.h"
#include "crtc.h"

#include <QtCore/QDebug>
#include <QtGui/QX11Info>

namespace QRandR {

Output::Output(Screen* parent, RROutput id)
: m_id(id)
, m_info(0)
, m_parent(parent)
, m_primary(false)
, m_crtc(0)
{
}

Output::~Output()
{
}

RROutput Output::id() const
{
    return m_id;
}

const QString& Output::name()
{
    if (m_name.isEmpty()) {
        m_name = info()->name;
    }

    return m_name;
}

bool Output::isConnected()
{
    return info()->connection == RR_Connected;
}

bool Output::isEnabled()
{
    return info()->crtc != None;
}

QHash<RRMode,  Mode* > Output::modes()
{
    if (!m_modes.isEmpty()) {
        return m_modes;
    }

    RRMode id;
    XRROutputInfo* outputInfo = info();
    for (int i = 0; i < outputInfo->nmode; ++i)
    {
        id = outputInfo->modes[i];
        m_modes[id] = m_parent->mode(id);
    }

    return m_modes;
}

Mode* Output::mode()
{
    return crtc()->mode();
}

bool Output::isPrimary()
{
    return m_primary;
}

void Output::setPrimary(bool primary)
{
    m_primary = primary;
}

Crtc* Output::crtc()
{
    if (!m_crtc) {
        m_crtc = m_parent->crtc(info()->crtc);
    }

    return m_crtc;
}

XRROutputInfo* Output::info()
{
    if (!m_info) {
        m_info = XRRGetOutputInfo(QX11Info::display(), m_parent->resources(), m_id);
    }

    return m_info;
}

}