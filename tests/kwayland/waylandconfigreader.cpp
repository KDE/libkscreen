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
#include <QUuid>

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
            //qDebug() << "new Output created: " << output["name"].toString();
        } else {
            //qDebug() << "disconnected Output" << output["name"].toString();
        }
    }
    auto outpus = WaylandConfigReader::createOutputs(display, outputs);
    s_outputIds.clear();
}

OutputDeviceInterface* WaylandConfigReader::createOutputDevice(const QVariantMap& outputConfig, KWayland::Server::Display *display)
{
    KWayland::Server::OutputDeviceInterface *outputdevice = display->createOutputDevice(display);

    QByteArray data = QByteArray::fromBase64(outputConfig["edid"].toByteArray());
    outputdevice->setEdid(data);
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
        outputdevice->setPhysicalSize(QSize(edid.width() * 10, edid.height() * 10));
        outputdevice->setManufacturer(edid.vendor());
        outputdevice->setModel(edid.name());
    } else {
        outputdevice->setPhysicalSize(sizeFromJson(outputConfig["sizeMM"]));
        outputdevice->setManufacturer(outputConfig["manufacturer"].toString());
        outputdevice->setModel(outputConfig["model"].toString());
    }
    auto uuid = QUuid::createUuid().toByteArray();
    auto _id = outputConfig["id"].toInt();
    if (_id) {
        uuid = QString::number(_id).toLocal8Bit();
    }
    outputdevice->setUuid(uuid);

    const QMap <int, KWayland::Server::OutputDeviceInterface::Transform> transformMap = {
        {0, KWayland::Server::OutputDeviceInterface::Transform::Normal},
        {1, KWayland::Server::OutputDeviceInterface::Transform::Normal},
        {2, KWayland::Server::OutputDeviceInterface::Transform::Rotated270},
        {3, KWayland::Server::OutputDeviceInterface::Transform::Rotated180},
        {4, KWayland::Server::OutputDeviceInterface::Transform::Rotated90}
    };

    outputdevice->setTransform(transformMap[outputConfig["rotation"].toInt()]);
    int currentModeId = outputConfig["currentModeId"].toInt();
    QVariantList preferredModes = outputConfig["preferredModes"].toList();

    int mode_id = 0;
    Q_FOREACH (const QVariant &_mode, outputConfig["modes"].toList()) {
        mode_id++;
        const QVariantMap &mode = _mode.toMap();
        OutputDeviceInterface::Mode m0;
        const QSize _size = sizeFromJson(mode["size"]);

        if (mode.keys().contains("refreshRate")) {
            m0.refreshRate = qRound(mode["refreshRate"].toReal() * 1000); // config has it in Hz
        }
        bool isCurrent = currentModeId == mode["id"].toInt();
        bool isPreferred = preferredModes.contains(mode["id"]);

        OutputDeviceInterface::ModeFlags flags;
        if (isCurrent && isPreferred) {
            flags = OutputDeviceInterface::ModeFlags(OutputDeviceInterface::ModeFlag::Current | OutputDeviceInterface::ModeFlag::Preferred);
        } else if (isCurrent) {
            flags = OutputDeviceInterface::ModeFlags(OutputDeviceInterface::ModeFlag::Current);
        } else if (isPreferred) {
            flags = OutputDeviceInterface::ModeFlags(OutputDeviceInterface::ModeFlag::Preferred);
        }

        if (mode.keys().contains("id")) {
            m0.id = mode["id"].toInt();
        } else {
            m0.id = mode_id;
        }
        m0.size = _size;
        m0.flags = flags;
        outputdevice->addMode(m0);

        if (isCurrent) {
            outputdevice->setCurrentMode(m0.id);
        }
    }

    outputdevice->setGlobalPosition(pointFromJson(outputConfig["pos"]));
    outputdevice->setEnabled(outputConfig["enabled"].toBool() ? OutputDeviceInterface::Enablement::Enabled : OutputDeviceInterface::Enablement::Disabled);
    outputdevice->create();

    return outputdevice;
}

QList<KWayland::Server::OutputInterface *> KScreen::WaylandConfigReader::createOutputs(KWayland::Server::Display* display, QList<KWayland::Server::OutputDeviceInterface *>& outputdevices)
{
    const QMap <KWayland::Server::OutputDeviceInterface::Transform,
                      KWayland::Server::OutputInterface::Transform> transformMap = {
                            {KWayland::Server::OutputDeviceInterface::Transform::Normal,
                                KWayland::Server::OutputInterface::Transform::Normal},
                            {KWayland::Server::OutputDeviceInterface::Transform::Rotated270,
                                KWayland::Server::OutputInterface::Transform::Rotated270},
                            {KWayland::Server::OutputDeviceInterface::Transform::Rotated180,
                                KWayland::Server::OutputInterface::Transform::Rotated180},
                            {KWayland::Server::OutputDeviceInterface::Transform::Rotated90,
                                KWayland::Server::OutputInterface::Transform::Rotated90},
    };



    QList<KWayland::Server::OutputInterface *> outputs;
    Q_FOREACH (const auto outputdevice, outputdevices) {
        qDebug() << "New Output!";
        KWayland::Server::OutputInterface *output = display->createOutput(display);

        // Sync properties from outputdevice to the newly created output interface
        output->setManufacturer(outputdevice->manufacturer());
        output->setModel(outputdevice->model());
        //output->setUuid(outputdevice->uuid());


        Q_FOREACH (const auto mode, outputdevice->modes()) {

            bool isCurrent = mode.flags.testFlag(OutputDeviceInterface::ModeFlag::Current);
            bool isPreferred = mode.flags.testFlag(OutputDeviceInterface::ModeFlag::Current);
            OutputInterface::ModeFlags flags;
            if (isPreferred && isCurrent) {
                flags = OutputInterface::ModeFlags(OutputInterface::ModeFlag::Current | OutputInterface::ModeFlag::Preferred);
            } else if (isCurrent) {
                flags = OutputInterface::ModeFlags(OutputInterface::ModeFlag::Current);
            } else if (isPreferred) {
                flags = OutputInterface::ModeFlags(OutputInterface::ModeFlag::Preferred);
            }

            OutputInterface::Mode m0;

            m0.size = mode.size;
            output->addMode(m0.size, m0.flags, m0.refreshRate);

            if (isCurrent) {
                output->setCurrentMode(m0.size, m0.refreshRate);
            }
            //qDebug() << "mode added:" << m0.size << m0.refreshRate << isCurrent;
        }

        output->setGlobalPosition(outputdevice->globalPosition());
        output->setPhysicalSize(outputdevice->physicalSize());
        output->setTransform(transformMap.value(outputdevice->transform()));

        output->setDpmsSupported(true);
        output->setDpmsMode(OutputInterface::DpmsMode::On);
        QObject::connect(output, &OutputInterface::dpmsModeRequested,
                [] (KWayland::Server::OutputInterface::DpmsMode requestedMode) {
                    Q_UNUSED(requestedMode);
                    // FIXME: make sure this happens in the scope of an object!
                    qDebug() << "DPMS Mode change requested";

                });
        output->create();
        outputs << output;
    }
    return outputs;
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

