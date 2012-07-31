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

#include "mode.h"
#include "screen.h"

#include <QtCore/QDebug>
namespace QRandR {

Mode::Mode(Screen* parent, XRRModeInfo* info) : QObject(parent)
, m_parent(parent)
, m_info(info)
, m_rate(-1)
{
}


Mode::~Mode()
{

}

RRMode Mode::id()
{
    return m_info->id;
}

const QString& Mode::name()
{
    if (!m_name.isEmpty()) {
        return m_name;
    }

    m_name = QString::fromUtf8(m_info->name);

    return m_name;
}

const QSize& Mode::size()
{
    if (!m_size.isEmpty()) {
        return m_size;
    }

    m_size.setWidth(m_info->width);
    m_size.setHeight(m_info->height);

    return m_size;
}

float Mode::rate()
{
    if (m_rate != -1) {
        return m_rate;
    }

    m_rate = ((float) m_info->dotClock / ((float) m_info->hTotal * (float) m_info->vTotal));

    return m_rate;
}


}