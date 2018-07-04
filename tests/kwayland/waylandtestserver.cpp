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

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QFile>
#include <QLoggingCategory>
#include <QStandardPaths>

#include "../src/edid.h"

Q_LOGGING_CATEGORY(KSCREEN_WAYLAND_TESTSERVER, "kscreen.kwayland.testserver")

using namespace KScreen;
using namespace KWayland::Server;

WaylandTestServer::WaylandTestServer(QObject *parent)
    : QObject(parent)
    , m_configFile(TEST_DATA + QStringLiteral("default.json"))
    , m_display(nullptr)
    , m_outputManagement(nullptr)
    , m_dpmsManager(nullptr)
    , m_suspendChanges(false)
    , m_waiting(nullptr)
{
}

WaylandTestServer::~WaylandTestServer()
{
    stop();
    qCDebug(KSCREEN_WAYLAND_TESTSERVER) << "Wayland server shut down.";
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

    auto manager = m_display->createDpmsManager();
    manager->create();

    m_outputManagement = m_display->createOutputManagement();
    m_outputManagement->create();
    connect(m_outputManagement, &OutputManagementInterface::configurationChangeRequested, this, &WaylandTestServer::configurationChangeRequested);

    KScreen::WaylandConfigReader::outputsFromConfig(m_configFile, m_display, m_outputs);
    qCDebug(KSCREEN_WAYLAND_TESTSERVER) << QString("export WAYLAND_DISPLAY="+m_display->socketName());
    qCDebug(KSCREEN_WAYLAND_TESTSERVER) << QString("You can specify the WAYLAND_DISPLAY for this server by exporting it in the environment");
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
    qCDebug(KSCREEN_WAYLAND_TESTSERVER) << "Creating Wayland server from " << configfile;
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

void WaylandTestServer::suspendChanges(bool suspend)
{
    if (m_suspendChanges == suspend) {
        return;
    }
    m_suspendChanges = suspend;
    if (!suspend && m_waiting) {
        m_waiting->setApplied();
        m_waiting = nullptr;
        Q_EMIT configChanged();
    }
}

void WaylandTestServer::configurationChangeRequested(KWayland::Server::OutputConfigurationInterface* configurationInterface)
{
    qCDebug(KSCREEN_WAYLAND_TESTSERVER) << "Server received change request, changes:" << configurationInterface->changes().count();
    Q_EMIT configReceived();

    auto changes = configurationInterface->changes();
    Q_FOREACH (const auto &outputdevice, changes.keys()) {
        auto c = changes[outputdevice];
        if (c->enabledChanged()) {
            qCDebug(KSCREEN_WAYLAND_TESTSERVER) << "Setting enabled:";
            outputdevice->setEnabled(c->enabled());
        }
        if (c->modeChanged()) {
            qCDebug(KSCREEN_WAYLAND_TESTSERVER) << "Setting new mode:" << c->mode() << modeString(outputdevice, c->mode());
            outputdevice->setCurrentMode(c->mode());
        }
        if (c->transformChanged()) {
            qCDebug(KSCREEN_WAYLAND_TESTSERVER) << "Server setting transform: " << (int)(c->transform());
            outputdevice->setTransform(c->transform());
        }
        if (c->positionChanged()) {
            qCDebug(KSCREEN_WAYLAND_TESTSERVER) << "Server setting position: " << c->position();
            outputdevice->setGlobalPosition(c->position());
        }
        if (c->scaleChanged()) {
            qCDebug(KSCREEN_WAYLAND_TESTSERVER) << "Setting scale:" << c->scale();
            outputdevice->setScale(c->scale());
        }
    }

    if (m_suspendChanges) {
        Q_ASSERT(!m_waiting);
        m_waiting = configurationInterface;
        return;
    }

    configurationInterface->setApplied();
    //showOutputs();
    Q_EMIT configChanged();
}

void WaylandTestServer::showOutputs()
{
    qCDebug(KSCREEN_WAYLAND_TESTSERVER) << "******** Wayland server running: " << m_outputs.count() << " outputs. ********";
    foreach (auto o, m_outputs) {
        bool enabled = (o->enabled() == KWayland::Server::OutputDeviceInterface::Enablement::Enabled);
        qCDebug(KSCREEN_WAYLAND_TESTSERVER) << "  * Output id: " << o->uuid();
        qCDebug(KSCREEN_WAYLAND_TESTSERVER) << "      Enabled: " << (enabled ? "enabled" : "disabled");
        qCDebug(KSCREEN_WAYLAND_TESTSERVER) << "         Name: " << QString("%2-%3").arg(o->manufacturer(), o->model());
        qCDebug(KSCREEN_WAYLAND_TESTSERVER) << "         Mode: " << modeString(o, o->currentModeId());
        qCDebug(KSCREEN_WAYLAND_TESTSERVER) << "          Pos: " << o->globalPosition();
        qCDebug(KSCREEN_WAYLAND_TESTSERVER) << "         Edid: " << o->edid();
        // << o->currentMode().size();

    }
    qCDebug(KSCREEN_WAYLAND_TESTSERVER) << "******************************************************";
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
