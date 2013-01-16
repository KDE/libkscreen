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

#include "config.h"
#include "edid.h"

#include <stdlib.h>

#include <QtCore/QFile>
#include <QtCore/qplugin.h>

#include <qjson/parser.h>

Q_EXPORT_PLUGIN2(Fake, Fake)

using namespace KScreen;

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
    return Parser::fromJson(QString(qgetenv("TEST_DATA")));
}

void Fake::setConfig(Config* config) const
{
    Q_UNUSED(config)
}

bool Fake::isValid() const
{
    return true;
}

Edid *Fake::edid(int outputId) const
{
    Q_UNUSED(outputId);
    QFile file(QString(qgetenv("TEST_DATA")));
    file.open(QIODevice::ReadOnly);

    QJson::Parser parser;
    QVariantMap json = parser.parse(file.readAll()).toMap();

    QList <QVariant> outputs = json["outputs"].toList();
    Q_FOREACH(const QVariant &value, outputs) {
        QMap <QString, QVariant > output = value.toMap();
        if (output["id"].toInt() != outputId) {
            continue;
        }

        QByteArray data = QByteArray::fromBase64(output["edid"].toByteArray());
        return new Edid((quint8*)data.data(), data.length());
    }
    return 0;
}

void Fake::updateConfig(Config *config) const
{
    Q_UNUSED(config);
}

#include "fake.moc"
