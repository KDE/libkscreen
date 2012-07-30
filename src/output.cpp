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

#include "output.h"
#include "mode.h"

Output::Output(int id, QObject* parent)
 : QObject(parent)
 , m_id(id)
 , m_currentMode(0)
{
}

Output::~Output()
{

}

QString Output::name() const
{
    return m_name;
}

void Output::setName(const QString& name)
{
    m_name = name;
}

QString Output::type() const
{
    return m_type;
}

void Output::setType(const QString& type)
{
    m_type = type;
}

QString Output::icon() const
{
    return m_icon;
}

void Output::setIcon(const QString& icon)
{
    m_icon = icon;
}

QHash< int, Mode* > Output::modes() const
{
    return m_modeList;
}

void Output::setModes(ModeList modes)
{
    m_modeList = modes;
}

Mode* Output::currentMode() const
{
    return new Mode(1);
}

void Output::setCurrentMode(Mode* mode)
{
    m_currentMode = mode;
}

QPoint Output::pos() const
{
    return m_pos;
}

void Output::setPos(const QPoint& pos)
{
    m_pos = pos;
}

QSize Output::size() const
{
    return m_size;
}

void Output::setSize(const QSize& size)
{
    m_size = size;
}

Output::Rotation Output::rotation() const
{
    return Output::None;
}

void Output::setRotation(Output::Rotation rotation)
{
    m_rotation = rotation;
}

int Output::id()
{
    return m_id;
}

void Output::setId(int id)
{
    m_id = id;
}

bool Output::isConnected() const
{
    return m_connected;
}

void Output::setConnected(bool connected)
{
    m_connected = connected;
}

bool Output::isEnabled() const
{
    return m_enabled;
}

void Output::setEnabled(bool enabled)
{
    m_enabled = enabled;
}

bool Output::isPrimary() const
{
    return m_primary;
}

void Output::setPrimary(bool primary)
{
    m_primary = primary;
}

QHash< int, Output* > Output::clones()
{
    return m_clones;
}

void Output::setClones(OutputList outputlist)
{
    m_clones = outputlist;
}

#include "output.moc"