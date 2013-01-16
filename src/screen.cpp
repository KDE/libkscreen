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

class Screen::Private
{
  public:
    Private():
      id(0)
    { }

    int id;
    QSize currentSize;
    QSize minSize;
    QSize maxSize;
};

Screen::Screen(QObject *parent)
 : QObject(parent)
 , d(new Private())
{
}

Screen::~Screen()
{
    delete d;
}

int Screen::id() const
{
    return d->id;
}

void Screen::setId(int id)
{
    d->id = id;
}

QSize Screen::currentSize() const
{
    return d->currentSize;
}

void Screen::setCurrentSize(const QSize& currentSize)
{
    if (d->currentSize == currentSize) {
        return;
    }

    d->currentSize = currentSize;

    Q_EMIT currentSizeChanged();
}

QSize Screen::maxSize() const
{
    return d->maxSize;
}

void Screen::setMaxSize(const QSize& maxSize)
{
    d->maxSize = maxSize;
}

QSize Screen::minSize() const
{
    return d->minSize;
}

void Screen::setMinSize(const QSize& minSize)
{
    d->minSize = minSize;
}

} //KScreen namespace

#include "screen.moc"
