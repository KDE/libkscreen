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

#include "../src/edid.h"

using namespace KScreen;
//using namespace KWayland::Server;

WaylandTestServer::WaylandTestServer(QObject *parent)
    : QObject(parent)
    , m_configFile(TEST_DATA + QStringLiteral("default.json"))
    , m_display(nullptr)
    , m_compositor(nullptr)
    , m_seat(nullptr)
    , m_shell(nullptr)
{
}

WaylandTestServer::~WaylandTestServer()
{

    qDebug() << "Shutting down server";
    delete m_display;
}

void KScreen::WaylandTestServer::start()
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

    m_outputs = KScreen::WaylandConfigReader::outputsFromConfig(m_configFile, m_display);

    qDebug() << "Wayland server running. Outputs: " << m_outputs.count();
}

void WaylandTestServer::stop()
{
    for (auto o: m_outputs) {
        delete o;
    }
    // actually stop the Wayland server
    delete m_display;
    m_display = nullptr;
    m_outputs.clear();
}

KWayland::Server::Display* KScreen::WaylandTestServer::display()
{
    return m_display;
}

void WaylandTestServer::setConfig(const QString& configfile)
{
    m_configFile = configfile;
}

void WaylandTestServer::pickupConfigFile(const QString& configfile)
{

}
