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
#include <QStandardPaths>

#include <KConfig>
#include <KConfigGroup>
#include <KSharedConfig>
#include <KDirWatch>

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
    , m_configWatch(nullptr)
{
    m_outputConfigFile = QStandardPaths::writableLocation(
                                 QStandardPaths::GenericConfigLocation) +
                                 "/waylandconfigrc";
                                 //"/waylandconfigtestrc";
    qDebug() << "m_outputConfigFile" << m_outputConfigFile;

}

WaylandTestServer::~WaylandTestServer()
{

    qDebug() << "Shutting down server";
    delete m_display;
}

void WaylandTestServer::start()
{
    m_display = new KWayland::Server::Display(this);
    if (qgetenv("WAYLAND_DISPLAY").isEmpty()) {
        m_display->setSocketName(s_socketName);
    } else {
        m_display->setSocketName(qgetenv("WAYLAND_DISPLAY").constData());

    }
    m_display->start();

    // Enable once we actually use these things...
    /*
    m_display->createShm();
    m_compositor = m_display->createCompositor();
    m_compositor->create();
    m_seat = m_display->createSeat();
    m_seat->create();
    m_shell = m_display->createShell();
    m_shell->create();
    */
    m_outputManagement = m_display->createOutputManagement();
    m_outputManagement->create();

    KScreen::WaylandConfigReader::outputsFromConfig(m_configFile, m_display, m_outputs);

    qDebug() << "WL m_outputs" << m_outputs.count();

    m_configWatch = new KDirWatch(this);
    m_configWatch->addFile(m_outputConfigFile);
    qDebug() << "KDirWatch::Added: " << m_outputConfigFile;
    connect(m_configWatch, &KDirWatch::dirty, this, &WaylandTestServer::pickupConfigFile);
    connect(m_configWatch, &KDirWatch::created, this, &WaylandTestServer::pickupConfigFile);


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

bool WaylandTestServer::outputFromConfigGroup(const KConfigGroup& config, KWayland::Server::OutputDeviceInterface* output)
{

    bool changed = false;

    const QSize ps = QSize(config.readEntry("width", -1), config.readEntry("height", -1));
    const int refresh = config.readEntry("refreshRate", 60000);
    if (ps != output->pixelSize() || refresh != output->refreshRate()) {
        output->setCurrentMode(ps, refresh);
        changed = true;
    }

    const QPoint pos = QPoint(config.readEntry("x", 0), config.readEntry("y", 0));
    if (pos != output->globalPosition()) {
        output->setGlobalPosition(pos);
        changed = true;
    }

    // FIXME : rotation

    return changed;
}

void WaylandTestServer::pickupConfigFile(const QString& configfile)
{
    bool changed = false;
    //KConfig cfg(m_outputConfigFile, KConfig::SimpleConfig);
    auto cfg = KSharedConfig::openConfig(configfile, KConfig::SimpleConfig);
    cfg->reparseConfiguration();
    qDebug() << "===== Updating outputs from config" << configfile;
    qDebug() << "Groups:" << cfg->groupList();
    QStringList os;
    // Check for changed outputs
    for (auto o: m_outputs) {
        // Note: this string concatenation is roughly what kwin does
        // it should not be encoded in the backend since it is too fragile,
        // but for tests it suffices
        const QString oname = o->manufacturer() + QStringLiteral("-") + o->model();

        qDebug() << " Before" << o->globalPosition() << o->pixelSize();

        // Check for changed or removed outputs
        if (cfg->groupList().contains(oname)) {
            if (outputFromConfigGroup(cfg->group(oname), o)) {
                changed = true;
                qDebug() << " ***** " << oname << " :: " << o->manufacturer() << o->model();
                qDebug() << " CHANGED! After" << o->globalPosition() << o->pixelSize();
            }
            os << oname;
        } else {

            qDebug() << " ---- " << oname << " :: " << o->manufacturer() << o->model();
            qDebug() << " ++Output gone++";
            m_outputs.removeAll(o);
            m_display->removeOutputDevice(o);
            changed = true;
            //delete o;
        }
    }

    // Check for added outputs
    foreach (const QString& oname, cfg->groupList()) {
        if (!os.contains(oname)) {

            KWayland::Server::OutputDeviceInterface *o = m_display->createOutputDevice(m_display);
            o->setManufacturer(oname.split("-").at(0));
            o->setModel(oname.split("-").at(1));
            outputFromConfigGroup(cfg->group(oname), o);
            qDebug() << " ***** " << oname << " :: " << o->manufacturer() << o->model();
            qDebug() << " Added: " << o->globalPosition() << o->pixelSize();
            m_outputs << o;

            changed = true;
        }
    }


    // Notify
    if (changed) {
        qDebug() << "Outputs: " << m_outputs.count();
        emit outputsChanged();
    }
}

int WaylandTestServer::outputCount() const
{
    return m_outputs.count();
}

