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

#include "fake.h"
#include "parser.h"

#include "config.h"
#include "edid.h"

#include <stdlib.h>

#include <QtCore/QFile>
#include <QtCore/qplugin.h>

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

using namespace KScreen;

Q_LOGGING_CATEGORY(KSCREEN_FAKE, "kscreen.fake")

Fake::Fake(QObject* parent): QObject(parent)
{
    QLoggingCategory::setFilterRules(QStringLiteral("kscreen.fake.debug = true"));
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

    QJsonDocument jsonDoc = QJsonDocument::fromJson(file.readAll());
    QJsonObject json = jsonDoc.object();

    QJsonArray outputs = json["outputs"].toArray();
    Q_FOREACH(const QJsonValue &value, outputs) {
        QVariantMap output = value.toObject().toVariantMap();
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
