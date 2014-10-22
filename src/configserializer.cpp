/*
 * Copyright (C) 2014  Daniel Vratil <dvratil@redhat.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#include "configserializer_p.h"

#include "config.h"
#include "mode.h"
#include "output.h"
#include "screen.h"
#include "edid.h"

#include <QDateTime>

using namespace KScreen;

QJsonObject ConfigSerializer::serializePoint(const QPoint &point)
{
    QJsonObject obj;
    obj[QLatin1String("x")] = point.x();
    obj[QLatin1String("y")] = point.y();
    return obj;
}

QJsonObject ConfigSerializer::serializeSize(const QSize &size)
{
    QJsonObject obj;
    obj[QLatin1String("width")] = size.width();
    obj[QLatin1String("height")] = size.height();
    return obj;
}

QJsonObject ConfigSerializer::serializeConfig(const ConfigPtr &config)
{
    QJsonObject obj;

    QJsonArray outputs;
    Q_FOREACH (const OutputPtr &output, config->outputs()) {
        outputs.append(serializeOutput(output));
    }
    obj[QLatin1String("outputs")] = outputs;
    obj[QLatin1String("screen")] = serializeScreen(config->screen());

    return obj;
}

QJsonObject ConfigSerializer::serializeOutput(const OutputPtr &output)
{
    QJsonObject obj;

    obj[QLatin1String("id")] = output->id();
    obj[QLatin1String("name")] = output->name();
    obj[QLatin1String("type")] = static_cast<int>(output->type());
    obj[QLatin1String("icon")] = output->icon();
    obj[QLatin1String("pos")] = serializePoint(output->pos());
    obj[QLatin1String("rotation")] = static_cast<int>(output->rotation());
    obj[QLatin1String("currentModeId")] = output->currentModeId();
    obj[QLatin1String("preferredModes")] = serializeList(output->preferredModes());
    obj[QLatin1String("connected")] = output->isConnected();
    obj[QLatin1String("enabled")] = output->isEnabled();
    obj[QLatin1String("primary")] = output->isPrimary();
    obj[QLatin1String("clones")] = serializeList(output->clones());
    //obj[QLatin1String("edid")] = output->edid()->raw();
    obj[QLatin1String("sizeMM")] = serializeSize(output->sizeMm());

    QJsonArray modes;
    Q_FOREACH (const ModePtr &mode, output->modes()) {
        modes.append(serializeMode(mode));
    }
    obj[QLatin1String("modes")] = modes;

    return obj;
}

QJsonObject ConfigSerializer::serializeMode(const ModePtr &mode)
{
    QJsonObject obj;

    obj[QLatin1String("id")] = mode->id();
    obj[QLatin1String("name")] = mode->name();
    obj[QLatin1String("size")] = serializeSize(mode->size());
    obj[QLatin1String("refreshRate")] = (double) mode->refreshRate();

    return obj;
}

QJsonObject ConfigSerializer::serializeScreen(const ScreenPtr &screen)
{
    QJsonObject obj;

    obj[QLatin1String("id")] = screen->id();
    obj[QLatin1String("currentSize")] = serializeSize(screen->currentSize());
    obj[QLatin1String("maxSize")] = serializeSize(screen->maxSize());
    obj[QLatin1String("minSize")] = serializeSize(screen->minSize());
    obj[QLatin1String("maxActiveOutputsCount")] = screen->maxActiveOutputsCount();

    return obj;
}

QPoint ConfigSerializer::deserializePoint(const QJsonObject &obj)
{
    return QPoint(obj[QLatin1String("x")].toInt(),
                  obj[QLatin1String("y")].toInt());
}

QSize ConfigSerializer::deserializeSize(const QJsonObject &obj)
{
    return QSize(obj[QLatin1String("width")].toInt(),
                 obj[QLatin1String("height")].toInt());
}

ConfigPtr ConfigSerializer::deserializeConfig(const QJsonObject &obj)
{
    ConfigPtr config(new Config);

    const QJsonArray outputsArr = obj[QLatin1String("outputs")].toArray();
    OutputList outputs;
    for (int i = 0; i < outputsArr.size(); ++i) {
        const KScreen::OutputPtr output = deserializeOutput(outputsArr[i].toObject());
        outputs.insert(output->id(), output);
    }
    config->setOutputs(outputs);

    config->setScreen(deserializeScreen(obj[QLatin1String("screen")].toObject()));

    return config;
}

OutputPtr ConfigSerializer::deserializeOutput(const QJsonObject &obj)
{
    OutputPtr output(new Output);

    output->setId(obj[QLatin1String("id")].toInt());
    output->setName(obj[QLatin1String("obj")].toString());
    output->setType(static_cast<Output::Type>(obj[QLatin1String("type")].toInt()));
    output->setIcon(obj[QLatin1String("icon")].toString());
    output->setPos(deserializePoint(obj[QLatin1String("pos")].toObject()));
    output->setRotation(static_cast<Output::Rotation>(obj[QLatin1String("rotation")].toInt()));
    output->setCurrentModeId(obj[QLatin1String("currentModeId")].toString());
    output->setPreferredModes(deserializeList<QString>(obj[QLatin1String("preferredModes")].toArray()));
    output->setConnected(obj[QLatin1String("connected")].toBool());
    output->setEnabled(obj[QLatin1String("enabled")].toBool());
    output->setPrimary(obj[QLatin1String("primary")].toBool());
    output->setClones(deserializeList<int>(obj[QLatin1String("clones")].toArray()));
    //output->setEdid(obj[QLatin1String("edid")].toByteArray());
    output->setSizeMm(deserializeSize(obj[QLatin1String("sizeMM")].toObject()));

    const QJsonArray modesArr = obj[QLatin1String("modes")].toArray();
    ModeList modes;
    for (int i = 0; i < modesArr.size(); ++i) {
        const KScreen::ModePtr mode = deserializeMode(modesArr[i].toObject());
        modes.insert(mode->id(), mode);
    }
    output->setModes(modes);

    return output;
}

ModePtr ConfigSerializer::deserializeMode(const QJsonObject &obj)
{
    ModePtr mode(new Mode);

    mode->setId(obj[QLatin1String("id")].toString());
    mode->setName(obj[QLatin1String("name")].toString());
    mode->setSize(deserializeSize(obj[QLatin1String("size")].toObject()));
    mode->setRefreshRate(obj[QLatin1String("refreshRate")].toDouble());

    return mode;
}

ScreenPtr ConfigSerializer::deserializeScreen(const QJsonObject &obj)
{
    ScreenPtr screen(new Screen);

    screen->setId(obj[QLatin1String("id")].toInt());
    screen->setCurrentSize(deserializeSize(obj[QLatin1String("currentSize")].toObject()));
    screen->setMinSize(deserializeSize(obj[QLatin1String("minSize")].toObject()));
    screen->setMaxSize(deserializeSize(obj[QLatin1String("maxSize")].toObject()));
    screen->setMaxActiveOutputsCount(obj[QLatin1String("maxActiveOutputsCount")].toInt());

    return screen;
}
