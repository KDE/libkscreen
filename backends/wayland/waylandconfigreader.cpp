/*************************************************************************************
 *  Copyright 2014-2015 Sebastian Kügler <sebas@kde.org>                             *
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

#include "waylandconfigreader.h"

#include <QDebug>

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QFile>

#include "edid.h"

using namespace KScreen;

static QList<int> s_outputIds;

void WaylandConfigReader::outputsFromConfig(const QString& configfile, KWayland::Server::Display* display,
                                            QList< KWayland::Server::OutputDeviceInterface* >& outputs)
{
    qDebug() << "Loading server from" << configfile;
    QFile file(configfile);
    file.open(QIODevice::ReadOnly);

    QJsonDocument jsonDoc = QJsonDocument::fromJson(file.readAll());
    QJsonObject json = jsonDoc.object();

    QJsonArray omap = json["outputs"].toArray();
    Q_FOREACH(const QJsonValue &value, omap) {
        const QVariantMap &output = value.toObject().toVariantMap();
        if (output["connected"].toBool()) {
            outputs << createOutputDevice(output, display);
            qDebug() << "new Output created: " << output["name"].toString();
        } else {
            qDebug() << "disconnected Output" << output["name"].toString();
        }
    }

    s_outputIds.clear();
}

OutputDeviceInterface* WaylandConfigReader::createOutputDevice(const QVariantMap& outputConfig, KWayland::Server::Display *display)
{
    KWayland::Server::OutputDeviceInterface *output = display->createOutputDevice(display);

    QByteArray data = QByteArray::fromBase64(outputConfig["edid"].toByteArray());
    Edid edid(data, display);

//     qDebug() << "EDID Info: ";
    if (edid.isValid()) {
//         qDebug() << "\tDevice ID: " << edid.deviceId();
//         qDebug() << "\tName: " << edid.name();
//         qDebug() << "\tVendor: " << edid.vendor();
//         qDebug() << "\tSerial: " << edid.serial();
//         qDebug() << "\tEISA ID: " << edid.eisaId();
//         qDebug() << "\tHash: " << edid.hash();
//         qDebug() << "\tWidth (mm): " << edid.width();
//         qDebug() << "\tHeight (mm): " << edid.height();
//         qDebug() << "\tGamma: " << edid.gamma();
//         qDebug() << "\tRed: " << edid.red();
//         qDebug() << "\tGreen: " << edid.green();
//         qDebug() << "\tBlue: " << edid.blue();
//         qDebug() << "\tWhite: " << edid.white();
        output->setPhysicalSize(QSize(edid.width() * 10, edid.height() * 10));
        output->setManufacturer(edid.vendor());
        output->setModel(edid.name());
    } else {
        output->setPhysicalSize(sizeFromJson(outputConfig["sizeMM"]));
        output->setManufacturer(outputConfig["manufacturer"].toString());
        output->setModel(outputConfig["model"].toString());
    }
    qDebug() << "Creating output device" << output->model() << output->manufacturer();

    QMap <int, KWayland::Server::OutputDeviceInterface::Transform> transformMap;
    transformMap[0] = KWayland::Server::OutputDeviceInterface::Transform::Normal;
    transformMap[1] = KWayland::Server::OutputDeviceInterface::Transform::Normal;
    transformMap[2] = KWayland::Server::OutputDeviceInterface::Transform::Rotated270;
    transformMap[3] = KWayland::Server::OutputDeviceInterface::Transform::Rotated180;
    transformMap[4] = KWayland::Server::OutputDeviceInterface::Transform::Rotated90;


    output->setTransform(transformMap[outputConfig["rotation"].toInt()]);
    int currentModeId = outputConfig["currentModeId"].toInt();
    QVariantList preferredModes = outputConfig["preferredModes"].toList();

    int mode_id = 0;
    Q_FOREACH(const QVariant &_mode, outputConfig["modes"].toList()) {
        mode_id++;
        const QVariantMap &mode = _mode.toMap();
        OutputDeviceInterface::Mode m0;
        const QSize _size = sizeFromJson(mode["size"]);
        int refresh = 60000;

        if (mode.keys().contains("refreshRate")) {
            m0.refreshRate = qRound(mode["refreshRate"].toReal() * 1000); // config has it in Hz
        }
        bool isCurrent = currentModeId == mode["id"].toInt();
        bool isPreferred = preferredModes.contains(mode["id"]);

        //qDebug() << "Mode: " << _size << isCurrent << isPreferred;
        OutputDeviceInterface::ModeFlags flags;
        if (isPreferred) {
            flags &= OutputDeviceInterface::ModeFlags(OutputDeviceInterface::ModeFlag::Preferred);
        }
        if (isCurrent) {
            flags &= OutputDeviceInterface::ModeFlags(OutputDeviceInterface::ModeFlag::Preferred);
        }
        //qDebug() << "add mode for " << output->model() << _size << refresh;
        if (mode.keys().contains("id")) {
            m0.id = mode["id"].toInt();
        } else {
            m0.id = mode_id;
        }
        m0.size = _size;
        m0.flags = flags;
        //OutputDeviceInterface::ModeFlags(OutputDeviceInterface::ModeFlag::Preferred);
        output->addMode(m0);
        qDebug() << "Mode: " << m0.size << m0.id << (isCurrent ? "*" : "");
        //output->addMode(_size, flags, refresh);

        if (isCurrent) {
            output->setCurrentMode(m0.id);
//             qDebug() << "Current Mode: " << m0.size << m0.id;
        }
    }

    output->setGlobalPosition(pointFromJson(outputConfig["pos"]));
    output->setEnabled(outputConfig["enabled"].toBool() ? OutputDeviceInterface::Enablement::Enabled : OutputDeviceInterface::Enablement::Disabled);

//     int _id = outputConfig["id"].toInt();
//     qDebug() << "read ID" << _id << s_outputIds;
//     while (s_outputIds.contains(_id)) {
//         _id = _id + 1000;
//     }
//     s_outputIds << _id;
//     qDebug() << "SETTING ID" << _id;

    //output->setId(_id);
    //qDebug() << "enabled? " << output->enabled();
    output->create();

    return output;
}

QSize WaylandConfigReader::sizeFromJson(const QVariant& data)
{
    QVariantMap map = data.toMap();

    QSize size;
    size.setWidth(map["width"].toInt());
    size.setHeight(map["height"].toInt());

    return size;
}

QPoint WaylandConfigReader::pointFromJson(const QVariant& data)
{
    QVariantMap map = data.toMap();

    QPoint point;
    point.setX(map["x"].toInt());
    point.setY(map["y"].toInt());

    return point;
}

QRect WaylandConfigReader::rectFromJson(const QVariant& data)
{
    QRect rect;
    rect.setSize(WaylandConfigReader::sizeFromJson(data));
    rect.setBottomLeft(WaylandConfigReader::pointFromJson(data));

    return rect;
}

