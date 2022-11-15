/*
 *  SPDX-FileCopyrightText: 2012 Alejandro Fiestas Olivares <afiestas@kde.org>
 *
 *  SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "parser.h"
#include "fake.h"

#include "config.h"
#include "mode.h"
#include "output.h"
#include "screen.h"

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLoggingCategory>
#include <QMetaObject>
#include <QMetaProperty>
#include <utility>

using namespace KScreen;

ConfigPtr Parser::fromJson(const QByteArray &data)
{
    ConfigPtr config(new Config);

    const QJsonObject json = QJsonDocument::fromJson(data).object();

    ScreenPtr screen = Parser::screenFromJson(json[QStringLiteral("screen")].toObject().toVariantMap());
    config->setScreen(screen);

    const QVariantList outputs = json[QStringLiteral("outputs")].toArray().toVariantList();
    if (outputs.isEmpty()) {
        return config;
    }

    OutputList outputList;
    for (const QVariant &value : outputs) {
        const OutputPtr output = Parser::outputFromJson(value.toMap());
        outputList.insert(output->id(), output);
    }

    config->setOutputs(outputList);
    return config;
}

ConfigPtr Parser::fromJson(const QString &path)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << file.errorString();
        qWarning() << "File: " << path;
        return ConfigPtr();
    }

    return Parser::fromJson(file.readAll());
}

ScreenPtr Parser::screenFromJson(const QVariantMap &data)
{
    ScreenPtr screen(new Screen);
    screen->setId(data[QStringLiteral("id")].toInt());
    screen->setMinSize(Parser::sizeFromJson(data[QStringLiteral("minSize")].toMap()));
    screen->setMaxSize(Parser::sizeFromJson(data[QStringLiteral("maxSize")].toMap()));
    screen->setCurrentSize(Parser::sizeFromJson(data[QStringLiteral("currentSize")].toMap()));
    screen->setMaxActiveOutputsCount(data[QStringLiteral("maxActiveOutputsCount")].toInt());

    return screen;
}

void Parser::qvariant2qobject(const QVariantMap &variant, QObject *object)
{
    const QMetaObject *metaObject = object->metaObject();
    for (QVariantMap::const_iterator iter = variant.begin(); iter != variant.end(); ++iter) {
        const int propertyIndex = metaObject->indexOfProperty(qPrintable(iter.key()));
        if (propertyIndex == -1) {
            // qWarning() << "Skipping non-existent property" << iter.key();
            continue;
        }
        const QMetaProperty metaProperty = metaObject->property(propertyIndex);
        if (!metaProperty.isWritable()) {
            // qWarning() << "Skipping read-only property" << iter.key();
            continue;
        }

        const QVariant property = object->property(iter.key().toLatin1().constData());
        Q_ASSERT(property.isValid());
        if (property.isValid()) {
            QVariant value = iter.value();
            if (value.canConvert(property.type())) {
                value.convert(property.type());
                object->setProperty(iter.key().toLatin1().constData(), value);
            } else if (QLatin1String("QVariant") == QLatin1String(property.typeName())) {
                object->setProperty(iter.key().toLatin1().constData(), value);
            }
        }
    }
}

OutputPtr Parser::outputFromJson(QMap<QString, QVariant> map)
{
    Output::Builder builder;

    builder.id = map[QStringLiteral("id")].toInt();

    QStringList preferredModes;
    const QVariantList prefModes = map[QStringLiteral("preferredModes")].toList();
    for (const QVariant &mode : prefModes) {
        preferredModes.append(mode.toString());
    }
    builder.preferredModes = preferredModes;
    map.remove(QStringLiteral("preferredModes"));

    ModeList modelist;
    const QVariantList modes = map[QStringLiteral("modes")].toList();
    for (const QVariant &modeValue : modes) {
        const ModePtr mode = Parser::modeFromJson(modeValue);
        modelist.insert(mode->id(), mode);
    }
    builder.modes = modelist;
    map.remove(QStringLiteral("modes"));

    if (map.contains(QStringLiteral("clones"))) {
        QList<int> clones;
        for (const QVariant &id : map[QStringLiteral("clones")].toList()) {
            clones.append(id.toInt());
        }

        builder.clones = clones;
        map.remove(QStringLiteral("clones"));
    }

    const QByteArray type = map[QStringLiteral("type")].toByteArray().toUpper();
    if (type.contains("LVDS") || type.contains("EDP") || type.contains("IDP") || type.contains("7")) {
        builder.type = Output::Panel;
    } else if (type.contains("VGA")) {
        builder.type = Output::VGA;
    } else if (type.contains("DVI")) {
        builder.type = Output::DVI;
    } else if (type.contains("DVI-I")) {
        builder.type = Output::DVII;
    } else if (type.contains("DVI-A")) {
        builder.type = Output::DVIA;
    } else if (type.contains("DVI-D")) {
        builder.type = Output::DVID;
    } else if (type.contains("HDMI") || type.contains("6")) {
        builder.type = Output::HDMI;
    } else if (type.contains("Panel")) {
        builder.type = Output::Panel;
    } else if (type.contains("TV")) {
        builder.type = Output::TV;
    } else if (type.contains("TV-Composite")) {
        builder.type = Output::TVComposite;
    } else if (type.contains("TV-SVideo")) {
        builder.type = Output::TVSVideo;
    } else if (type.contains("TV-Component")) {
        builder.type = Output::TVComponent;
    } else if (type.contains("TV-SCART")) {
        builder.type = Output::TVSCART;
    } else if (type.contains("TV-C4")) {
        builder.type = Output::TVC4;
    } else if (type.contains("DisplayPort") || type.contains("14")) {
        builder.type = Output::DisplayPort;
    } else if (type.contains("Unknown")) {
        builder.type = Output::Unknown;
    } else {
        qCWarning(KSCREEN_FAKE) << "Output Type not translated:" << type;
    }
    map.remove(QStringLiteral("type"));

    if (map.contains(QStringLiteral("pos"))) {
        builder.pos = Parser::pointFromJson(map[QStringLiteral("pos")].toMap());
        map.remove(QStringLiteral("pos"));
    }

    if (map.contains(QStringLiteral("size"))) {
        builder.size = Parser::sizeFromJson(map[QStringLiteral("size")].toMap());
        map.remove(QStringLiteral("size"));
    }

    // This is not supported in real configs; only set this value in fake test
    // configs and don't add logic to kscreen to set this data in real configs.
    if (map.contains(QStringLiteral("sizeMM"))) {
        builder.sizeMm = Parser::sizeFromJson(map[QStringLiteral("sizeMM")].toMap());
        map.remove(QStringLiteral("sizeMM"));
    }

    auto scale = QStringLiteral("scale");
    if (map.contains(scale)) {
        qDebug() << "Scale found:" << map[scale].toReal();
        builder.scale = map[scale].toReal();
        map.remove(scale);
    }

    // the deprecated "primary" property may exist for compatibility, but "priority" should override it whenever present.
    if (map.contains(QStringLiteral("primary"))) {
        builder.primary = map[QStringLiteral("primary")].toBool();
        map.remove(QStringLiteral("primary"));
    }
    if (map.contains(QStringLiteral("priority"))) {
        builder.primary = map[QStringLiteral("priority")].toUInt() == 1;
        map.remove(QStringLiteral("priority"));
    }

    // Remove some extra properties that we do not want or need special treatment
    map.remove(QStringLiteral("edid"));

    OutputPtr output(new Output(std::move(builder)));
    Parser::qvariant2qobject(map, output.data());
    return output;
}

ModePtr Parser::modeFromJson(const QVariant &data)
{
    const QVariantMap map = data.toMap();
    ModePtr mode(new Mode);
    Parser::qvariant2qobject(map, mode.data());

    mode->setSize(Parser::sizeFromJson(map[QStringLiteral("size")].toMap()));

    return mode;
}

QSize Parser::sizeFromJson(const QVariant &data)
{
    const QVariantMap map = data.toMap();

    QSize size;
    size.setWidth(map[QStringLiteral("width")].toInt());
    size.setHeight(map[QStringLiteral("height")].toInt());

    return size;
}

QPoint Parser::pointFromJson(const QVariant &data)
{
    const QVariantMap map = data.toMap();

    QPoint point;
    point.setX(map[QStringLiteral("x")].toInt());
    point.setY(map[QStringLiteral("y")].toInt());

    return point;
}

QRect Parser::rectFromJson(const QVariant &data)
{
    QRect rect;
    rect.setSize(Parser::sizeFromJson(data));
    rect.setBottomLeft(Parser::pointFromJson(data));

    return rect;
}

bool Parser::validate(const QByteArray &data)
{
    Q_UNUSED(data);
    return true;
}

bool Parser::validate(const QString &data)
{
    Q_UNUSED(data);
    return true;
}
