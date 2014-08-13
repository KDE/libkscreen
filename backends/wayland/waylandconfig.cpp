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

#include "waylandconfig.h"
#include "waylandoutput.h"
#include "waylandscreen.h"
#include "waylandbackend.h"

// Wayland
#include <wayland-client-protocol.h>


#include <configmonitor.h>
#include <mode.h>

#include <QtCore/QRect>
#include <QGuiApplication>
#include <QScreen>

using namespace KScreen;



/**
 * Callback for announcing global objects in the registry
 **/
static void registryHandleGlobal(void *data, struct wl_registry *registry,
                                 uint32_t name, const char *interface, uint32_t version)
{
    Q_UNUSED(version)
    qDebug() << "Wayland Interface: " << interface << name << version;
//     WaylandBackend *d = reinterpret_cast<WaylandBackend*>(data);
//
//     if (strcmp(interface, "wl_compositor") == 0) {
//         d->setCompositor(reinterpret_cast<wl_compositor*>(wl_registry_bind(registry, name, &wl_compositor_interface, 1)));
//     } else if (strcmp(interface, "wl_shell") == 0) {
//         d->setShell(reinterpret_cast<wl_shell *>(wl_registry_bind(registry, name, &wl_shell_interface, 1)));
//     } else if (strcmp(interface, "wl_seat") == 0) {
//         d->createSeat(name);
//     } else if (strcmp(interface, "wl_shm") == 0) {
//         d->createShm(name);
    if (strcmp(interface, "wl_output") == 0) {
//         qDebug() << "wl_output!";
        WaylandBackend::internalConfig()->addOutput(name, reinterpret_cast<wl_output *>(wl_registry_bind(registry, name, &wl_output_interface, 1)));
    }
}

/**
 * Callback for removal of global objects in the registry
 **/
static void registryHandleGlobalRemove(void *data, struct wl_registry *registry, uint32_t name)
{
    Q_UNUSED(data)
    Q_UNUSED(registry)
    Q_UNUSED(name)
    // TODO: implement me
}


// handlers
static const struct wl_registry_listener s_registryListener = {
    registryHandleGlobal,
    registryHandleGlobalRemove
};


WaylandConfig::WaylandConfig(QObject *parent)
    : QObject(parent)
    , m_screen(new WaylandScreen(this))
    , m_blockSignals(true)
    , m_runtimeDir(qgetenv("XDG_RUNTIME_DIR"))
{
    m_socketName = qgetenv("WAYLAND_DISPLAY");
    if (m_socketName.isEmpty()) {
        m_socketName = QStringLiteral("wayland-0");
    }

    qCDebug(KSCREEN_WAYLAND) << " Config creating.";
    m_blockSignals = false;
    initConnection();
    //connect(qApp, &QGuiApplication::screenAdded, this, &WaylandConfig::screenAdded);
}

WaylandConfig::~WaylandConfig()
{
//     foreach (auto output, m_outputMap.values()) {
//         delete output;
//     }
}

void WaylandConfig::initConnection()
{
//    qDebug() << "wl_display_connect";
    m_display = wl_display_connect(nullptr);
    if (!m_display) {
        // TODO: maybe we should now really tear down
        qWarning() << "Failed connecting to Wayland display";
        return;
    }
    m_registry = wl_display_get_registry(m_display);
    // setup the registry
    wl_registry_add_listener(m_registry, &s_registryListener, this);
    wl_display_dispatch(m_display);
    int fd = wl_display_get_fd(m_display);
    QSocketNotifier *notifier = new QSocketNotifier(fd, QSocketNotifier::Read, this);
    connect(notifier, &QSocketNotifier::activated, this, &WaylandConfig::readEvents);

    readEvents(); // force-flush display command queue
}

void WaylandConfig::readEvents()
{
    qCDebug(KSCREEN_WAYLAND) << "readEvents...";
    wl_display_flush(m_display);
    wl_display_dispatch(m_display);
}

void WaylandConfig::addOutput(quint32 name, wl_output* o)
{
    qDebug() << "Addoutput " << name;
//     if (m_outputMap.keys().contains(o)) {
//         return;
//     }
    WaylandOutput *waylandoutput = new WaylandOutput(o, this);
    qDebug() << "WLO created";
    waylandoutput->setId(name); // FIXME: name is uint32 and doesn't fit in there.
    //m_outputMap.insert(name, waylandoutput);

    qDebug() << "WLO inserted";
    //connect(qscreen, SIGNAL(destroyed(QObject*)), this, SLOT(screenDestroyed(QObject*)));
    //connect(qscreen, &QObject::destroyed, this, &QScreenConfig::screenDestroyed);

    if (!m_blockSignals) {
        //KScreen::ConfigMonitor::instance()->notifyUpdate();
    }
    QList<WaylandOutput*> os;
    //os << waylandoutput;
    //qDebug() << "screen updated with output" << m_screen;
    //m_screen->setOutputs(os);
}

wl_display* WaylandConfig::display() const
{
    return m_display;
}


Config* WaylandConfig::toKScreenConfig() const
{
    Config *config = new Config();
    config->setScreen(m_screen->toKScreenScreen(config));
    updateKScreenConfig(config);
    return config;
}

int WaylandConfig::outputId(wl_output *wlo)
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

void WaylandConfig::screenAdded(const QScreen* qscreen)
{
    /*
    qCDebug(KSCREEN_WAYLAND) << "Screen added!!! Updating config.." << qscreen << qscreen->name();
    WaylandOutput *qscreenoutput = new WaylandOutput(qscreen, this);
    qscreenoutput->setId(outputId(qscreen));
    m_outputMap.insert(qscreenoutput->id(), qscreenoutput);

    //connect(qscreen, SIGNAL(destroyed(QObject*)), this, SLOT(screenDestroyed(QObject*)));
    connect(qscreen, &QObject::destroyed, this, &WaylandConfig::screenDestroyed);

    */
    if (!m_blockSignals) {
        KScreen::ConfigMonitor::instance()->notifyUpdate();
    }
}

void WaylandConfig::screenDestroyed(QObject* qscreen)
{
    qCDebug(KSCREEN_WAYLAND) << "Screen Removed!!! .." << qscreen << QGuiApplication::screens().count();
    /*
    // Find output matching the QScreen object and remove it
    int removedOutputId = -1;
    foreach (auto output, m_outputMap.values()) {
        if (output->qscreen() == qscreen) {
            qDebug() << "Found output matching the qscreen " << output;
            removedOutputId = output->id();
            m_outputMap.remove(removedOutputId);
            delete output;
        }
    }
    */
    KScreen::ConfigMonitor::instance()->notifyUpdate();
}

void WaylandConfig::updateKScreenConfig(Config* config) const
{
    /*
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
            qDebug() << "Adding output" << output->qscreen()->name();
            config->addOutput(kscreenOutput);
        }
        output->updateKScreenOutput(kscreenOutput);
        if (QGuiApplication::primaryScreen() == output->qscreen()) {
            config->setPrimaryOutput(kscreenOutput);
        }
    }
    */
}

QMap< int, WaylandOutput * > WaylandConfig::outputMap() const
{
    QMap< int, WaylandOutput * > map; // FIXME
    return map;
}


