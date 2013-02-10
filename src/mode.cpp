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

namespace KScreen {

class Mode::Private
{
  public:
    Private():
      rate(0)
    { }

    QString id;
    QString name;
    QSize size;
    float rate;
};

Mode::Mode(QObject *parent)
  : QObject(parent)
  , d(new Private())
{

}

Mode::~Mode()
{
    delete d;
}

const QString Mode::id() const
{
    return d->id;
}

void Mode::setId(const QString& id)
{
    if (d->id == id) {
        return;
    }

    d->id = id;

    Q_EMIT modeChanged();
}

QString Mode::name() const
{
    return d->name;
}

void Mode::setName(const QString& name)
{
    if (d->name == name) {
        return;
    }

    d->name = name;

    Q_EMIT modeChanged();
}


QSize Mode::size() const
{
    return d->size;
}

void Mode::setSize(const QSize& size)
{
    if (d->size == size) {
        return;
    }

    d->size = size;

    Q_EMIT modeChanged();
}

float Mode::refreshRate() const
{
    return d->rate;
}

void Mode::setRefreshRate(float refresh)
{
    if (d->rate == refresh) {
        return;
    }

    d->rate = refresh;

    Q_EMIT modeChanged();
}

} //KScreen namespace

#include "mode.moc"
