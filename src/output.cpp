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

Output::Output()
{

}

const QString Output::name() const
{
    return QString("FakeOutput_1");
}

QHash< int, Mode* > Output::modes() const
{
    QHash<int, Mode*> modes;
    modes[1] = new Mode();

    return modes;
}

Mode* Output::currentMode() const
{
    return new Mode();
}

QPoint Output::pos() const
{
    return QPoint(0,0);
}

QSize Output::size() const
{
    return QSize(1280, 800);
}

Output::Rotation Output::rotation() const
{
    return Output::None;
}

bool Output::isConnected() const
{
    return true;
}

bool Output::isEnabled() const
{
    return true;
}

bool Output::isPrimary() const
{
    return true;
}

QHash< int, Output* > Output::clones()
{
    QHash< int, Output* > clones;
    return clones;
}
