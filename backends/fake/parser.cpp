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

#include "parser.h"

#include <config.h>

#include <QtCore/QFile>

#include <qjson/parser.h>
#include <qjson/qobjecthelper.h>

Config* Parser::fromJson(const QByteArray& data)
{
    Config *config =  new Config();
    QJson::Parser parser;

    QVariantMap json = parser.parse(data).toMap();

    CScreen* screen = Parser::screenFromJson(json["screen"].toMap());

    QList <QVariant> outputs = json["outputs"].toList();
    if (outputs.isEmpty()) {
        return config;
    }

    Output *output;
    OutputList outputList;
    Q_FOREACH(const QVariant &value, outputs) {
        output = Parser::outputFromJson(value);
        outputList.insert(output->id(), output);
    }

    config->setScreen(screen);
    config->setOutputs(outputList);
    return config;
}

Config* Parser::fromJson(const QString& path)
{
    QFile file(path);
    file.open(QIODevice::ReadOnly);

    return Parser::fromJson(file.readAll());
}

CScreen* Parser::screenFromJson(const QMap< QString, QVariant >& data)
{
    CScreen* screen = new CScreen(data["id"].toInt());
    screen->setMinSize(Parser::sizeFromJson(data["minSize"].toMap()));
    screen->setMaxSize(Parser::sizeFromJson(data["maxSize"].toMap()));
    screen->setCurrentSize(Parser::sizeFromJson(data["currentSize"].toMap()));

    return screen;
}

Output* Parser::outputFromJson(const QVariant& data)
{
    QVariantMap map = data.toMap();
    Output *output = new Output(map["id"].toInt());
    QJson::QObjectHelper::qvariant2qobject(map, output);

    Mode *mode;
    ModeList modelist;
    QList <QVariant> modes = map["modes"].toList();
    Q_FOREACH(const QVariant &modeValue, modes) {
        mode = Parser::modeFromJson(modeValue);
        modelist.insert(mode->id(), mode);
    }
    output->setModes(modelist);

    if(map.contains("clones")) {
        QList<int> clones;
        Q_FOREACH(const QVariant id, map["clones"].toList()) {
            clones.append(id.toInt());
        }

        output->setClones(clones);
    }
    return output;
}

Mode* Parser::modeFromJson(const QVariant& data)
{
    QVariantMap map = data.toMap();
    Mode *mode = new Mode(map["id"].toInt());
    QJson::QObjectHelper::qvariant2qobject(map, mode);

    mode->setSize(Parser::sizeFromJson(map["size"].toMap()));

    return mode;
}

QSize Parser::sizeFromJson(const QVariant& data)
{
    QVariantMap map = data.toMap();

    QSize size;
    size.setWidth(map["width"].toInt());
    size.setHeight(map["height"].toInt());

    return size;
}

QPoint Parser::pointFromJson(const QVariant& data)
{
    QVariantMap map = data.toMap();

    QPoint point;
    point.setX(map["x"].toInt());
    point.setY(map["y"].toInt());

    return point;
}

QRect Parser::rectFromJson(const QVariant& data)
{
    QRect rect;
    rect.setSize(Parser::sizeFromJson(data));
    rect.setBottomLeft(Parser::pointFromJson(data));

    return rect;
}

bool Parser::validate(const QByteArray& data)
{
    Q_UNUSED(data);
    return true;
}

bool Parser::validate(const QString& data)
{
    Q_UNUSED(data);
    return true;
}
