/*************************************************************************************
 *  Copyright 2015 Sebastian Kügler <sebas@kde.org>                                  *
 *  Copyright 2015 Martin Gräßlin <mgraesslin@kde.org>                               *
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

#include "drmbackend.h"
#include "logind.h"

#include <QDebug>
#include <QLoggingCategory>

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QFile>
#include <QSocketNotifier>
#include <QStandardPaths>

// drm
#include <xf86drm.h>
#include <xf86drmMode.h>
#include <libdrm/drm_mode.h>


using namespace KScreen;

Q_LOGGING_CATEGORY(KSCREEN_WAYLAND, "kscreen.wayland");

DrmBackend::DrmBackend(QObject *parent)
    : QObject(parent)
    , m_udev(new Udev)
    , m_udevMonitor(m_udev->monitor())
    {
}

DrmBackend::~DrmBackend()
{
}

void DrmBackend::start()
{
    LogindIntegration::create(this);
    LogindIntegration *logind = LogindIntegration::self();
    auto takeControl = [logind, this]() {
        if (logind->hasSessionControl()) {
            openDrm();
        } else {
            logind->takeControl();
            connect(logind, &LogindIntegration::hasSessionControlChanged, this, &DrmBackend::openDrm);
        }
    };
    if (logind->isConnected()) {
        takeControl();
    } else {
        connect(logind, &LogindIntegration::connectedChanged, this, takeControl);
    }
//     auto v = VirtualTerminal::create(this);
//     connect(v, &VirtualTerminal::activeChanged, this, &DrmBackend::activate);
}

void DrmBackend::openDrm()
{
    qDebug() << "Opening DRM";

    //connect(LogindIntegration::self(), &LogindIntegration::sessionActiveChanged, this, &DrmBackend::activate);
    //VirtualTerminal::self()->init();
    UdevDevice::Ptr device = m_udev->primaryGpu();
    if (!device) {
        qCWarning(KSCREEN_WAYLAND) << "Did not find a GPU";
        return;
    }
    int fd = LogindIntegration::self()->takeDevice(device->devNode());
    if (fd < 0) {
        qCWarning(KSCREEN_WAYLAND) << "failed to open drm device at" << device->devNode();
        return;
    }
    m_fd = fd;
    //m_active = true;
    QSocketNotifier *notifier = new QSocketNotifier(m_fd, QSocketNotifier::Read, this);
    connect(notifier, &QSocketNotifier::activated, this,
            [this] {
//                 if (!VirtualTerminal::self()->isActive()) {
//                     return;
//                 }
                drmEventContext e;
                memset(&e, 0, sizeof e);
                e.version = DRM_EVENT_CONTEXT_VERSION;
//                 e.page_flip_handler = pageFlipHandler;
                drmHandleEvent(m_fd, &e);
                qCDebug(KSCREEN_WAYLAND) << "drm event" << e.version;
            }
    );
    m_drmId = device->sysNum();
    queryResources();

    // setup udevMonitor
    if (m_udevMonitor) {
        m_udevMonitor->filterSubsystemDevType("drm");
        const int fd = m_udevMonitor->fd();
        if (fd != -1) {
            QSocketNotifier *notifier = new QSocketNotifier(fd, QSocketNotifier::Read, this);
            connect(notifier, &QSocketNotifier::activated, this,
                    [this] {
                        auto device = m_udevMonitor->getDevice();
                        if (!device) {
                            return;
                        }
                        if (device->sysNum() != m_drmId) {
                            return;
                        }
                        if (device->hasProperty("HOTPLUG", "1")) {
                            qCDebug(KSCREEN_WAYLAND) << "Received hot plug event for monitored drm device";
                            //queryResources();
                            //m_cursorIndex = (m_cursorIndex + 1) % 2;
                            //updateCursor();
                        }
                    }
            );
            m_udevMonitor->enable();
        }
    }
    //setReady(true);


//     qApp.quit();
}

void DrmBackend::queryResources()
{
    if (m_fd < 0) {
        return;
    }
    ScopedDrmPointer<_drmModeRes, &drmModeFreeResources> resources(drmModeGetResources(m_fd));
    if (!resources) {
        qCWarning(KSCREEN_WAYLAND) << "drmModeGetResources failed";
        return;
    }

    QVector<DrmOutput*> connectedOutputs;
    for (int i = 0; i < resources->count_connectors; ++i) {
        const auto id = resources->connectors[i];
        ScopedDrmPointer<_drmModeConnector, &drmModeFreeConnector> connector(drmModeGetConnector(m_fd, id));
        if (!connector) {
            continue;
        }
        if (connector->connection != DRM_MODE_CONNECTED) {
//             qCDebug(KSCREEN_WAYLAND) << "   Disconnected" << id;
            continue;
        }
        qCDebug(KSCREEN_WAYLAND) << "Found DRM connector" << id;
        qCDebug(KSCREEN_WAYLAND) << "   Modes" << connector->count_modes;
        if (connector->count_modes == 0) {
            continue;
        }

        if (DrmOutput *o = findOutput(connector->connector_id)) {
            connectedOutputs << o;
            continue;
        }
        bool crtcFound = false;
        const quint32 crtcId = findCrtc(resources.data(), connector.data(), &crtcFound);
        if (!crtcFound) {
            continue;
        }
        ScopedDrmPointer<_drmModeCrtc, &drmModeFreeCrtc> crtc(drmModeGetCrtc(m_fd, crtcId));
        if (!crtc) {
            continue;
        }
        DrmOutput *drmOutput = new DrmOutput(this);
        drmOutput->m_crtcId = crtcId;
        if (crtc->mode_valid) {
            drmOutput->m_mode = crtc->mode;
        } else {
            drmOutput->m_mode = connector->modes[0];
        }
        drmOutput->m_connector = connector->connector_id;
        drmOutput->init(connector.data());
        connectedOutputs << drmOutput;
    }

    // check for outputs which got removed
    auto it = m_outputs.begin();
    while (it != m_outputs.end()) {
        if (connectedOutputs.contains(*it)) {
            it++;
            continue;
        }
        DrmOutput *removed = *it;
        it = m_outputs.erase(it);
//         emit outputRemoved(removed);
        delete removed;
    }
    for (auto it = connectedOutputs.constBegin(); it != connectedOutputs.constEnd(); ++it) {
        if (!m_outputs.contains(*it)) {
//             emit outputAdded(*it);
        }
    }
    m_outputs = connectedOutputs;

//     emit screensQueried();
}

DrmOutput *DrmBackend::findOutput(quint32 connector)
{
    auto it = std::find_if(m_outputs.constBegin(), m_outputs.constEnd(), [connector] (DrmOutput *o) {
        return o->m_connector == connector;
    });
    if (it != m_outputs.constEnd()) {
        return *it;
    }
    return nullptr;
}

quint32 DrmBackend::findCrtc(drmModeRes *res, drmModeConnector *connector, bool *ok)
{
    if (ok) {
        *ok = false;
    }
    ScopedDrmPointer<_drmModeEncoder, &drmModeFreeEncoder> encoder(drmModeGetEncoder(m_fd, connector->encoder_id));
    if (encoder) {
        if (!crtcIsUsed(encoder->crtc_id)) {
            if (ok) {
                *ok = true;
            }
            return encoder->crtc_id;
        }
    }
    // let's iterate over all encoders to find a suitable crtc
    for (int i = 0; i < connector->count_encoders; ++i) {
        ScopedDrmPointer<_drmModeEncoder, &drmModeFreeEncoder> encoder(drmModeGetEncoder(m_fd, connector->encoders[i]));
        if (!encoder) {
            continue;
        }
        for (int j = 0; j < res->count_crtcs; ++j) {
            if (!(encoder->possible_crtcs & (1 << j))) {
                continue;
            }
            if (!crtcIsUsed(res->crtcs[j])) {
                if (ok) {
                    *ok = true;
                }
                return res->crtcs[j];
            }
        }
    }
    return 0;
}

bool DrmBackend::crtcIsUsed(quint32 crtc)
{
    auto it = std::find_if(m_outputs.constBegin(), m_outputs.constEnd(),
                           [crtc] (DrmOutput *o) {
                               return o->m_crtcId == crtc;
                           }
    );
    return it != m_outputs.constEnd();
}
