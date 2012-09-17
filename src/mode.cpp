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

#include "mode.h"

Mode::Mode()
 : QObject()
 , m_id(0)
 , m_rate(0)
{

}

Mode::Mode(int id, QObject* parent)
 : QObject(parent)
 , m_id(id)
 , m_rate(0)
{

}

Mode::~Mode()
{

}

int Mode::id()
{
    return m_id;
}

void Mode::setId(int id)
{
    m_id = id;

   Q_EMIT modeChanged();
}


QString Mode::name() const
{
    return m_name;
}

void Mode::setName(const QString& name)
{
    m_name = name;

    Q_EMIT modeChanged();
}


QSize Mode::size() const
{
    return m_size;
}

void Mode::setSize(const QSize& size)
{
    m_size = size;

    Q_EMIT modeChanged();
}

float Mode::refreshRate() const
{
    return m_rate;
}

void Mode::setRefreshRate(float refresh)
{
    m_rate = refresh;

    Q_EMIT modeChanged();
}

#include "mode.moc"
