/*************************************************************************************
 *  Copyright (C) 2012 by Alejandro Fiestas Olivares <afiestas@kde.org>              *
 *                                                                                   *
 *  This library is free software; you can redistribute it and/or                    *
 *  modify it under the terms of the GNU Lesser General Public                       *
 *  License as published by the Free Software Foundation; either                     *
 *  version 2.1 of the License, or (at your option) any later version.               *
 *                                                                                   *
 *  This library is distributed in the hope that it will be useful,                  *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of                   *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU                *
 *  Lesser General Public License for more details.                                  *
 *                                                                                   *
 *  You should have received a copy of the GNU Lesser General Public                 *
 *  License along with this library; if not, write to the Free Software              *
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA       *
 *************************************************************************************/

#include "screen.h"

namespace KScreen {

class Screen::Private
{
  public:
    Private():
      id(0),
      maxActiveOutputsCount(0)
    { }

    Private(const Private &other):
        id(other.id),
        maxActiveOutputsCount(other.maxActiveOutputsCount),
        currentSize(other.currentSize),
        minSize(other.minSize),
        maxSize(other.maxSize)
    {
    }

    int id;
    int maxActiveOutputsCount;
    QSize currentSize;
    QSize minSize;
    QSize maxSize;
};

Screen::Screen(QObject *parent)
 : QObject(parent)
 , d(new Private())
{
}

Screen::Screen(Screen::Private *dd)
 : QObject()
 , d(dd)
{
}


Screen::~Screen()
{
    delete d;
}

Screen *Screen::clone() const
{
    return new Screen(new Private(*d));
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

int Screen::maxActiveOutputsCount() const
{
    return d->maxActiveOutputsCount;
}

void Screen::setMaxActiveOutputsCount(int maxActiveOutputsCount)
{
    d->maxActiveOutputsCount = maxActiveOutputsCount;
}

} //KScreen namespace

#include "screen.moc"
