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

#include "waylandconfigreader.h"
#include <QDebug>

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QFile>
#include <QSignalSpy>
#include <QStandardPaths>

#include <KConfig>
#include <KConfigGroup>
#include <KSharedConfig>
#include <KDirWatch>

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

    QString os;
    foreach (auto o, m_outputs) {
        bool enabled = (o->enabled() == KWayland::Server::OutputDeviceInterface::Enablement::Enabled);
        //os.append(QString());
        os.append(QString("[ %1 : %2-%3 (%4)] ").arg(o->uuid(), o->manufacturer(), o->model(), (enabled ? "enabled" : "disabled")));

    }
    qDebug() << "Wayland server running. Outputs: " << m_outputs.count() << os;
}

void WaylandTestServer::stop()
{
    for (auto o: m_outputs) {
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

void WaylandTestServer::configurationChangeRequested(KWayland::Server::OutputConfigurationInterface* configurationInterface)
{
    qDebug() << "Server received change request, changes:" << configurationInterface->changes().count();

    auto changes = configurationInterface->changes();
    for (auto outputdevice: changes.keys()) {
        auto c = changes[outputdevice];
        if (c->enabledChanged()) {
            qDebug() << "Setting enabled!!!";
            outputdevice->setEnabled(c->enabled());
        }
        if (c->modeChanged()) {
            outputdevice->setCurrentMode(c->mode());
        }
        if (c->transformChanged()) {
            outputdevice->setTransform(c->transform());
        }
        if (c->positionChanged()) {
            outputdevice->setGlobalPosition(c->position());
        }
        if (c->scaleChanged()) {
            outputdevice->setScale(c->scale());
        }
    }

    configurationInterface->setApplied();
    Q_EMIT configChanged();
}
