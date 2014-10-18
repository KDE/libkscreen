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
    QFile file(configfile);
    file.open(QIODevice::ReadOnly);

    QJsonDocument jsonDoc = QJsonDocument::fromJson(file.readAll());
    QJsonObject json = jsonDoc.object();

    QJsonArray outputs = json["outputs"].toArray();
    Q_FOREACH(const QJsonValue &value, outputs) {
        const QVariantMap &output = value.toObject().toVariantMap();
        m_outputs << createOutput(output);
        qDebug() << "Output created: " << output["name"].toString();
    }
}

OutputInterface* WaylandTestServer::createOutput(const QVariantMap& outputConfig)
{
    OutputInterface *output = m_display->createOutput(this);

    QByteArray data = QByteArray::fromBase64(outputConfig["edid"].toByteArray());
    Edid edid((quint8*)data.data(), data.length());

    int currentMode = outputConfig["currentMode"].toInt();
    QVariantList preferredModes = outputConfig["preferredModes"].toList();
    //QJsonArray modes = outputConfig["modes"].toVariantMap();
    Q_FOREACH(const QVariant &_mode, outputConfig["modes"].toList()) {
        const QVariantMap &mode = _mode.toMap();

        const QSize _size = sizeFromJson(mode["size"]);
        int refresh = qRound(outputConfig["refreshRate"].toReal() * 1000);

        OutputInterface::ModeFlags flags;
        if (preferredModes.contains(mode["id"])) {
            flags = OutputInterface::ModeFlags(OutputInterface::ModeFlag::Preferred);
        }

        output->addMode(_size, flags);
        if (currentMode == mode["id"].toInt()) {
            output->setCurrentMode(_size);
        }
        qDebug() << "Mode: " << sizeFromJson(mode["size"]);
        //output->addMode(QSize(800, 600), OutputInterface::ModeFlags(OutputInterface::ModeFlag::Preferred));
        //output->addMode(QSize(1024, 768));
        //output->addMode(QSize(1280, 1024), OutputInterface::ModeFlags(), 90000);
    }

    output->setCurrentMode(QSize(1024, 768));
    output->setGlobalPosition(QPoint(0, 0));
    output->setPhysicalSize(QSize(400, 300)); // FIXME mm?
    output->setManufacturer("Darknet Industries");
    output->setModel("Small old monitor");
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




