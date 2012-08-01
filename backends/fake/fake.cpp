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

#include "fake.h"
#include "parser.h"

#include <config.h>

#include <QtCore/QDebug>
#include <QtCore/QFile>
#include <QtCore/qplugin.h>go

Q_EXPORT_PLUGIN2(Fake, Fake)

Fake::Fake(QObject* parent): QObject(parent)
{

}

Fake::~Fake()
{

}

QString Fake::name() const
{
    return QString("Fake");
}

Config* Fake::config() const
{
    return Parser::fromJson(QString::fromLatin1(getenv("TEST_DATA")));
}

bool Fake::isValid() const
{
    return true;
}

#include "fake.moc"