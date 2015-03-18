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
#include "debug_p.h"

#include <QtDBus/QDBusArgument>
#include <QJsonDocument>
#include <QFile>

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
    if (config->screen()) {
        obj[QLatin1String("screen")] = serializeScreen(config->screen());
    }

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
    obj[QLatin1String("refreshRate")] = mode->refreshRate();

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

QPoint ConfigSerializer::deserializePoint(const QDBusArgument &arg)
{
    int x = 0, y = 0;
    arg.beginMap();
    while (!arg.atEnd()) {
        QString key;
        QVariant value;
        arg.beginMapEntry();
        arg >> key >> value;
        if (key == QLatin1String("x")) {
            x = value.toInt();
        } else if (key == QLatin1String("y")) {
            y = value.toInt();
        } else {
            qCWarning(KSCREEN) << "Invalid key in Point map: " << key;
            return QPoint();
        }
        arg.endMapEntry();
    }
    arg.endMap();
    return QPoint(x, y);
}

QSize ConfigSerializer::deserializeSize(const QDBusArgument &arg)
{
    int w = 0, h = 0;
    arg.beginMap();
    while (!arg.atEnd()) {
        QString key;
        QVariant value;
        arg.beginMapEntry();
        arg >> key >> value;
        if (key == QLatin1String("width")) {
            w = value.toInt();
        } else if (key == QLatin1String("height")) {
            h = value.toInt();
        } else {
            qCWarning(KSCREEN) << "Invalid key in size struct: " << key;
            return QSize();
        }
        arg.endMapEntry();
    }
    arg.endMap();

    return QSize(w, h);
}

ConfigPtr ConfigSerializer::deserializeConfig(const QVariantMap &map)
{
    ConfigPtr config(new Config);

    if (map.contains(QLatin1String("outputs"))) {
        const QDBusArgument &outputsArg = map[QLatin1String("outputs")].value<QDBusArgument>();
        outputsArg.beginArray();
        OutputList outputs;
        while (!outputsArg.atEnd()) {
            QVariant value;
            outputsArg >> value;
            const KScreen::OutputPtr output = deserializeOutput(value.value<QDBusArgument>());
            if (!output) {
                return ConfigPtr();
            }
            outputs.insert(output->id(), output);
        }
        outputsArg.endArray();
        config->setOutputs(outputs);
    }

    if (map.contains(QLatin1String("screen"))) {
        const QDBusArgument &screenArg = map[QLatin1String("screen")].value<QDBusArgument>();
        const KScreen::ScreenPtr screen = deserializeScreen(screenArg);
        if (!screen) {
            return ConfigPtr();
        }
        config->setScreen(screen);
    }

    return config;
}

OutputPtr ConfigSerializer::deserializeOutput(const QDBusArgument &arg)
{
    OutputPtr output(new Output);

    arg.beginMap();
    while (!arg.atEnd()) {
        QString key;
        QVariant value;
        arg.beginMapEntry();
        arg >> key >> value;
        if (key == QLatin1String("id")) {
            output->setId(value.toInt());
        } else if (key == QLatin1String("name")) {
            output->setName(value.toString());
        } else if (key == QLatin1String("type")) {
            output->setType(static_cast<Output::Type>(value.toInt()));
        } else if (key == QLatin1String("icon")) {
            output->setIcon(value.toString());
        } else if (key == QLatin1String("pos")) {
            output->setPos(deserializePoint(value.value<QDBusArgument>()));
        } else if (key == QLatin1String("rotation")) {
            output->setRotation(static_cast<Output::Rotation>(value.toInt()));
        } else if (key == QLatin1String("currentModeId")) {
            output->setCurrentModeId(value.toString());
        } else if (key == QLatin1String("preferredModes")) {
            output->setPreferredModes(deserializeList<QString>(value.value<QDBusArgument>()));
        } else if (key == QLatin1String("connected")) {
            output->setConnected(value.toBool());
        } else if (key == QLatin1String("enabled")) {
            output->setEnabled(value.toBool());
        } else if (key == QLatin1String("primary")) {
            output->setPrimary(value.toBool());
        } else if (key == QLatin1String("clones")) {
            output->setClones(deserializeList<int>(value.value<QDBusArgument>()));
        } else if (key == QLatin1String("sizeMM")) {
            output->setSizeMm(deserializeSize(value.value<QDBusArgument>()));
        } else if (key == QLatin1String("modes")) {
            const QDBusArgument arg = value.value<QDBusArgument>();
            ModeList modes;
            arg.beginArray();
            while (!arg.atEnd()) {
                QVariant value;
                arg >> value;
                const KScreen::ModePtr mode = deserializeMode(value.value<QDBusArgument>());
                if (!mode) {
                    return OutputPtr();
                }
                modes.insert(mode->id(), mode);
            }
            arg.endArray();
            output->setModes(modes);
        } else {
            qCWarning(KSCREEN) << "Invalid key in Output map: " << key;
            return OutputPtr();
        }
        arg.endMapEntry();
    }
    arg.endMap();
    return output;
}

ModePtr ConfigSerializer::deserializeMode(const QDBusArgument &arg)
{
    ModePtr mode(new Mode);

    arg.beginMap();
    while (!arg.atEnd()) {
        QString key;
        QVariant value;
        arg.beginMapEntry();
        arg >> key >> value;

        if (key == QLatin1String("id")) {
            mode->setId(value.toString());
        } else if (key == QLatin1String("name")) {
            mode->setName(value.toString());
        } else if (key == QLatin1String("size")) {
            mode->setSize(deserializeSize(value.value<QDBusArgument>()));
        } else if (key == QLatin1String("refreshRate")) {
            mode->setRefreshRate(value.toFloat());
        } else {
            qCWarning(KSCREEN) << "Invalid key in Mode map: " << key;
            return ModePtr();
        }
        arg.endMapEntry();
    }
    arg.endMap();
    return mode;
}

ScreenPtr ConfigSerializer::deserializeScreen(const QDBusArgument &arg)
{
    ScreenPtr screen(new Screen);

    arg.beginMap();
    QString key;
    QVariant value;
    while (!arg.atEnd()) {
        arg.beginMapEntry();
        arg >> key >> value;
        if (key == QLatin1String("id")) {
            screen->setId(value.toInt());
        } else if (key == QLatin1String("maxActiveOutputsCount")) {
            screen->setMaxActiveOutputsCount(value.toInt());
        } else if (key == QLatin1String("currentSize")) {
            screen->setCurrentSize(deserializeSize(value.value<QDBusArgument>()));
        } else if (key == QLatin1String("maxSize")) {
            screen->setMaxSize(deserializeSize(value.value<QDBusArgument>()));
        } else if (key == QLatin1String("minSize")) {
            screen->setMinSize(deserializeSize(value.value<QDBusArgument>()));
        } else {
            qCWarning(KSCREEN) << "Invalid key in Screen map:" << key;
            return ScreenPtr();
        }
        arg.endMapEntry();
    }
    arg.endMap();
    return screen;
}
