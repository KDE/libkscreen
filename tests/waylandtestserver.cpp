/*************************************************************************************
 *  Copyright 2014 Sebastian Kügler <sebas@kde.org>                                  *
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

#include "waylandtestserver.h"

#include <QDebug>

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QFile>

#include "../src/edid.h"

using namespace KScreen;
using namespace KWayland::Server;

WaylandTestServer::WaylandTestServer(QObject *parent)
    : QObject(parent)
    , m_display(nullptr)
    , m_compositor(nullptr)
    , m_seat(nullptr)
    , m_shell(nullptr)
{
    init();
}

WaylandTestServer::~WaylandTestServer()
{

    qDebug() << "Shutting down server";
    m_display->terminate();
    delete m_display;
}

void WaylandTestServer::init()
{
    m_display = new KWayland::Server::Display(this);
    m_display->setSocketName(s_socketName);
    m_display->start();

    // Enable once we actually use these things...
    m_display->createShm();
    m_compositor = m_display->createCompositor();
    m_compositor->create();
    m_seat = m_display->createSeat();
    m_seat->create();
    m_shell = m_display->createShell();
    m_shell->create();
}

void KScreen::WaylandTestServer::setConfig(const QString& configfile)
{
    m_outputs = outputsFromConfig(configfile, m_display);
}

QList<KWayland::Server::OutputInterface*> WaylandTestServer::outputsFromConfig(const QString& configfile, KWayland::Server::Display* display)
{
    QList<KWayland::Server::OutputInterface*> wloutputs;
    QFile file(configfile);
    file.open(QIODevice::ReadOnly);

    QJsonDocument jsonDoc = QJsonDocument::fromJson(file.readAll());
    QJsonObject json = jsonDoc.object();

    QJsonArray outputs = json["outputs"].toArray();
    Q_FOREACH(const QJsonValue &value, outputs) {
        const QVariantMap &output = value.toObject().toVariantMap();
        wloutputs << createOutput(output, display);
        qDebug() << "Output created: " << output["name"].toString();
    }
    return wloutputs;
}


OutputInterface* WaylandTestServer::createOutput(const QVariantMap& outputConfig, KWayland::Server::Display *display)
{
    OutputInterface *output = display->createOutput(display);

    QByteArray data = QByteArray::fromBase64(outputConfig["edid"].toByteArray());
    Edid edid(data, display);

    qDebug() << "EDID Info: ";
    if (edid.isValid()) {
        qDebug() << "\tDevice ID: " << edid.deviceId();
        qDebug() << "\tName: " << edid.name();
        qDebug() << "\tVendor: " << edid.vendor();
//         qDebug() << "\tSerial: " << edid.serial();
//         qDebug() << "\tEISA ID: " << edid.eisaId();
//         qDebug() << "\tHash: " << edid.hash();
//         qDebug() << "\tWidth: " << edid.width();
//         qDebug() << "\tHeight: " << edid.height();
//         qDebug() << "\tGamma: " << edid.gamma();
//         qDebug() << "\tRed: " << edid.red();
//         qDebug() << "\tGreen: " << edid.green();
//         qDebug() << "\tBlue: " << edid.blue();
//         qDebug() << "\tWhite: " << edid.white();
        output->setPhysicalSize(QSize(edid.width() * 10, edid.height() * 10));
        output->setManufacturer(edid.vendor());
        output->setModel(edid.name());
    }

    int currentModeId = outputConfig["currentModeId"].toInt();
    QVariantList preferredModes = outputConfig["preferredModes"].toList();

    Q_FOREACH(const QVariant &_mode, outputConfig["modes"].toList()) {
        const QVariantMap &mode = _mode.toMap();
        const QSize _size = sizeFromJson(mode["size"]);
        int refresh = 60000;

        if (outputConfig.keys().contains("refreshRate")) {
            refresh = qRound(outputConfig["refreshRate"].toReal() * 1000);
        }
        bool isCurrent = currentModeId == mode["id"].toInt();
        bool isPreferred = preferredModes.contains(mode["id"]);

        qDebug() << "Mode: " << _size << isCurrent << isPreferred;
        OutputInterface::ModeFlags flags;
        if (isPreferred) {
            flags &= OutputInterface::ModeFlags(OutputInterface::ModeFlag::Preferred);
        }
        if (isCurrent) {
            flags &= OutputInterface::ModeFlags(OutputInterface::ModeFlag::Preferred);
        }

        output->addMode(_size, flags, refresh);

        if (isCurrent) {
            output->setCurrentMode(_size, refresh);
        }
    }

    output->setGlobalPosition(pointFromJson(outputConfig["pos"]));
    output->create();

    return output;
}

QSize WaylandTestServer::sizeFromJson(const QVariant& data)
{
    QVariantMap map = data.toMap();

    QSize size;
    size.setWidth(map["width"].toInt());
    size.setHeight(map["height"].toInt());

    return size;
}

QPoint WaylandTestServer::pointFromJson(const QVariant& data)
{
    QVariantMap map = data.toMap();

    QPoint point;
    point.setX(map["x"].toInt());
    point.setY(map["y"].toInt());

    return point;
}

QRect WaylandTestServer::rectFromJson(const QVariant& data)
{
    QRect rect;
    rect.setSize(WaylandTestServer::sizeFromJson(data));
    rect.setBottomLeft(WaylandTestServer::pointFromJson(data));

    return rect;
}

