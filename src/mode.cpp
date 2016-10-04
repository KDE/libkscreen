/*************************************************************************************
 *  Copyright (C) 2012 by Alejandro Fiestas Olivares <afiestas@kde.org>              *
 *  Copyright (C) 2014 by Daniel Vrátil <dvratil@redhat.com>                         *
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

#include "mode.h"

using namespace KScreen;
class Mode::Private
{
  public:
    Private():
      rate(0)
    { }

    Private(const Private &other):
        id(other.id),
        name(other.name),
        size(other.size),
        rate(other.rate)
    {
    }

    QString id;
    QString name;
    QSize size;
    float rate;
};

Mode::Mode()
  : QObject(0)
  , d(new Private())
{

}

Mode::Mode(Mode::Private *dd)
  : QObject()
  , d(dd)
{
}

Mode::~Mode()
{
    delete d;
}

ModePtr Mode::clone() const
{
    return ModePtr(new Mode(new Private(*d)));
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

QDebug operator<<(QDebug dbg, const KScreen::ModePtr &mode)
{
    if (mode) {
        dbg << "KScreen::Mode(Id:" << mode->id() << ", Size:" << mode->size() << "@" << mode->refreshRate() << ")";
    }  else {
        dbg << "KScreen::Mode(NULL)";
    }
    return dbg;
}
