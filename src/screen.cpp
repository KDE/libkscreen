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

#include "screen.h"

namespace KScreen {

Screen::Screen(QObject *parent)
 : QObject(parent)
 , m_id(0)
{
}

Screen::~Screen()
{
}

int Screen::id() const
{
    return m_id;
}

void Screen::setId(int id)
{
    m_id = id;
}

QSize Screen::currentSize() const
{
    return m_currentSize;
}

void Screen::setCurrentSize(const QSize& currentSize)
{
    if (m_currentSize == currentSize) {
        return;
    }

    m_currentSize = currentSize;

    Q_EMIT currentSizeChanged();
}

QSize Screen::maxSize() const
{
    return m_maxSize;
}

void Screen::setMaxSize(const QSize& maxSize)
{
    m_maxSize = maxSize;
}

QSize Screen::minSize() const
{
    return m_minSize;
}

void Screen::setMinSize(const QSize& minSize)
{
    m_minSize = minSize;
}

} //KScreen namespace
#include "screen.moc"