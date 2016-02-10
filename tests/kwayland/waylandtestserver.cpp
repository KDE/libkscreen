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

#include "waylandtestserver.h"

#include "waylandconfigreader.h"
#include <QDebug>

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QFile>
#include <QStandardPaths>

#include "../src/edid.h"

using namespace KScreen;
using namespace KWayland::Server;

WaylandTestServer::WaylandTestServer(QObject *parent)
    : QObject(parent)
    , m_configFile(TEST_DATA + QStringLiteral("default.json"))
    , m_display(nullptr)
{
}

WaylandTestServer::~WaylandTestServer()
{
    stop();
    qDebug() << "Wayland server shut down.";
}

void WaylandTestServer::start()
{
    using namespace KWayland::Server;
    delete m_display;
    m_display = new KWayland::Server::Display(this);
    if (qgetenv("WAYLAND_DISPLAY").isEmpty()) {
        m_display->setSocketName(s_socketName);
    } else {
        m_display->setSocketName(qgetenv("WAYLAND_DISPLAY").constData());
    }
    m_display->start();

    m_outputManagement = m_display->createOutputManagement();
    m_outputManagement->create();
    connect(m_outputManagement, &OutputManagementInterface::configurationChangeRequested, this, &WaylandTestServer::configurationChangeRequested);

    KScreen::WaylandConfigReader::outputsFromConfig(m_configFile, m_display, m_outputs);
    qDebug() << QString("export WAYLAND_DISPLAY="+m_display->socketName());
    qDebug() << QString("You can specify the WAYLAND_DISPLAY for this server by exporting it in the environment");
    //showOutputs();
}

void WaylandTestServer::stop()
{
    Q_FOREACH (const auto &o, m_outputs) {
        delete o;
    }
    m_outputs.clear();
    // actually stop the Wayland server
    delete m_display;
    m_display = nullptr;
}

KWayland::Server::Display* WaylandTestServer::display()
{
    return m_display;
}

void WaylandTestServer::setConfig(const QString& configfile)
{
    m_configFile = configfile;
}

int WaylandTestServer::outputCount() const
{
    return m_outputs.count();
}
QList<KWayland::Server::OutputDeviceInterface*> WaylandTestServer::outputs() const
{
    return m_outputs;
}

void WaylandTestServer::configurationChangeRequested(KWayland::Server::OutputConfigurationInterface* configurationInterface)
{
    qDebug() << "Server received change request, changes:" << configurationInterface->changes().count();

    auto changes = configurationInterface->changes();
    Q_FOREACH (const auto &outputdevice, changes.keys()) {
        auto c = changes[outputdevice];
        if (c->enabledChanged()) {
            qDebug() << "Setting enabled:";
            outputdevice->setEnabled(c->enabled());
        }
        if (c->modeChanged()) {
            qDebug() << "Setting new mode:" << c->mode() << modeString(outputdevice, c->mode());
            outputdevice->setCurrentMode(c->mode());
        }
        if (c->transformChanged()) {
            qDebug() << "Server setting transform: " << (int)(c->transform());
            outputdevice->setTransform(c->transform());
        }
        if (c->positionChanged()) {
            qDebug() << "Server setting position: " << c->position();
            outputdevice->setGlobalPosition(c->position());
        }
        if (c->scaleChanged()) {
            qDebug() << "Setting enabled:";
            outputdevice->setScale(c->scale());
        }
    }

    configurationInterface->setApplied();
    //showOutputs();
    Q_EMIT configChanged();
}

void WaylandTestServer::showOutputs()
{
    qDebug() << "******** Wayland server running: " << m_outputs.count() << " outputs. ********";
    foreach (auto o, m_outputs) {
        bool enabled = (o->enabled() == KWayland::Server::OutputDeviceInterface::Enablement::Enabled);
        qDebug() << "  * Output id: " << o->uuid();
        qDebug() << "      Enabled: " << (enabled ? "enabled" : "disabled");
        qDebug() << "         Name: " << QString("%2-%3").arg(o->manufacturer(), o->model());
        qDebug() << "         Mode: " << modeString(o, o->currentModeId());
        qDebug() << "          Pos: " << o->globalPosition();
        qDebug() << "         Edid: " << o->edid();
        // << o->currentMode().size();

    }
    qDebug() << "******************************************************";
}

QString WaylandTestServer::modeString(KWayland::Server::OutputDeviceInterface* outputdevice, int mid)
{
    QString s;
    QString ids;
    int _i = 0;
    Q_FOREACH (const auto &_m, outputdevice->modes()) {
        _i++;
        if (_i < 6) {
            ids.append(QString::number(_m.id) + ", ");
        } else {
            ids.append(".");
        }
        if (_m.id == mid) {
            s = QString("%1x%2 @%3").arg(QString::number(_m.size.width()), \
            QString::number(_m.size.height()), QString::number(_m.refreshRate));
        }
    }
    return QString("[%1] %2 (%4 modes: %3)").arg(QString::number(mid), s, ids, QString::number(outputdevice->modes().count()));

}
