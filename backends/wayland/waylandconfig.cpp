/*************************************************************************************
 *  Copyright 2014 Sebastian Kügler <sebas@kde.org>                                  *
 *  Copyright 2013 Martin Gräßlin <mgraesslin@kde.org>                               *
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

#include "waylandconfig.h"
#include "waylandoutput.h"
#include "waylandscreen.h"
#include "waylandbackend.h"

#include <QThread>

// Wayland
#include <wayland-client-protocol.h>

// KWayland
#include <KWayland/Client/connection_thread.h>
#include <KWayland/Client/event_queue.h>
#include <KWayland/Client/registry.h>

#include <configmonitor.h>
#include <mode.h>


using namespace KScreen;


WaylandConfig::WaylandConfig(QObject *parent)
    : QObject(parent)
    , m_runtimeDir(qgetenv("XDG_RUNTIME_DIR"))
    , m_screen(new WaylandScreen(this))
    , m_blockSignals(true)
{
    m_socketName = qgetenv("WAYLAND_DISPLAY");
    if (m_socketName.isEmpty()) {
        m_socketName = QStringLiteral("wayland-0");
    }
    m_blockSignals = false;
    qCDebug(KSCREEN_WAYLAND) << " Config creating.";
    initConnection();
}

WaylandConfig::~WaylandConfig()
{
    qDebug() << "Byebye";
    foreach (auto output, m_outputMap.values()) {
        delete output;
    }
}

void WaylandConfig::initConnection()
{
    qDebug() << "wl_display_connect";
    m_connection = new KWayland::Client::ConnectionThread;
    QThread *thread = new QThread;
    m_connection->moveToThread(thread);
    thread->start();

    connect(m_connection, &KWayland::Client::ConnectionThread::connected, this, &WaylandConfig::setupRegistry, Qt::QueuedConnection);

//     connect(m_connection, &KWayland::Client::ConnectionThread::failed, &[] {
//         qDebug() << "Failed to connect to Wayland server at socket:" << m_connection->socketName();
//     });
    m_connection->initConnection();


    return;
}

void WaylandConfig::readEvents()
{

    //qCDebug(KSCREEN_WAYLAND) << "readEvents...";
}

void WaylandConfig::setupRegistry()
{
    qDebug() << "Connected to Wayland server at socket:" << m_connection->socketName();

    m_queue = new KWayland::Client::EventQueue(this);
    m_queue->setup(m_connection);

    m_registry = new KWayland::Client::Registry(this);
    m_registry->setEventQueue(m_queue);

    connect(m_registry, &KWayland::Client::Registry::outputAnnounced, this, &WaylandConfig::addOutput, Qt::QueuedConnection);

    m_registry->create(m_connection);
    m_registry->setup();

}


void WaylandConfig::addOutput(quint32 name, quint32 version)
{
    qDebug() << "!!! Addoutput " << name;
    if (m_outputMap.keys().contains(name)) {
        qDebug() << "Output already known";
        return;
    }
    KWayland::Client::Output *o = m_registry->createOutput(name, version);

    WaylandOutput *waylandoutput = new WaylandOutput(o, this);
    connect(waylandoutput, &WaylandOutput::complete, [=]{
        qDebug() << "WLO created";
        waylandoutput->setId(name);
        m_outputMap.insert(name, waylandoutput);
        qDebug() << "WLO setPhysicalSize: " << waylandoutput->physicalSize();
        //m_outputMap[waylandoutput->id()] = waylandoutput;
        qDebug() << "WLO inserted";

        if (!m_blockSignals) {
            KScreen::ConfigMonitor::instance()->notifyUpdate();
        }
        //m_blockSignals = false;
    });

}

Config* WaylandConfig::toKScreenConfig() const
{
    Config *config = new Config();
    config->setScreen(m_screen->toKScreenScreen(config));
    updateKScreenConfig(config);
    return config;
}

int WaylandConfig::outputId(KWayland::Client::Output *wlo)
{
    QList<int> ids;
    foreach (auto output, m_outputMap.values()) {
        if (wlo == output->output()) {
            return output->id();
        }
    }
    m_lastOutputId++;
    return m_lastOutputId;
}

void WaylandConfig::removeOutput(quint32 id)
{
    qCDebug(KSCREEN_WAYLAND) << "output screen Removed!!! .." << id << m_outputMap[id];
    // Find output matching the QScreen object and remove it
    int removedOutputId = -1;
    foreach (auto output, m_outputMap.values()) {
//         if (output->qscreen() == qscreen) {
//             qDebug() << "Found output matching the qscreen " << output;
//             removedOutputId = output->id();
//             m_outputMap.remove(removedOutputId);
//             delete output;
//         }
    }
    if (!m_blockSignals) {
        KScreen::ConfigMonitor::instance()->notifyUpdate();
    }
}

void WaylandConfig::updateKScreenConfig(Config* config) const
{
    m_screen->updateKScreenScreen(config->screen());

    //Removing removed outputs
    KScreen::OutputList outputs = config->outputs();
    Q_FOREACH(KScreen::Output *output, outputs) {
        if (!m_outputMap.keys().contains(output->id())) {
            config->removeOutput(output->id());
        }
    }
    // Add KScreen::Outputs that aren't in the list yet, handle primaryOutput
    foreach(auto output, m_outputMap.values()) {

        KScreen::Output *kscreenOutput = config->output(output->id());

        if (!kscreenOutput) {
            kscreenOutput = output->toKScreenOutput(config);
            qDebug() << "Adding output" << output->id();
            config->addOutput(kscreenOutput);
        }
        output->updateKScreenOutput(kscreenOutput);
        // FIXME: primaryScreen?
    }
}

QMap< int, WaylandOutput * > WaylandConfig::outputMap() const
{
    //QMap< int, WaylandOutput * > map; // FIXME
    return outputMap();
}


