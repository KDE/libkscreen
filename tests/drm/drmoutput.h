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

#ifndef KSCREEN_DRM_OUTPUT_H
#define KSCREEN_DRM_OUTPUT_H

#include "drmbackend.h"
#include "udev.h"

#include <xf86drmMode.h>

#include <QObject>
#include <QLoggingCategory>
#include <QRect>
#include <QSize>
#include <QByteArray>

Q_DECLARE_LOGGING_CATEGORY(KSCREEN_WAYLAND)

namespace KScreen
{
class DrmOutput
{
public:
    struct Edid {
        QByteArray eisaId;
        QByteArray monitorName;
        QByteArray serialNumber;
        QSize physicalSize;
    };
    virtual ~DrmOutput();
    //void showCursor(DrmBuffer *buffer);
    void hideCursor();
    void moveCursor(const QPoint &globalPos);
    //bool present(DrmBuffer *buffer);
    void pageFlipped();
    void init(drmModeConnector *connector);
    void restoreSaved();
    void blank();

    QSize size() const;
    QRect geometry() const;

private:
    friend class DrmBackend;
    DrmOutput(DrmBackend *backend);
//     void cleanupBlackBuffer();
//     bool setMode(DrmBuffer *buffer);
    void initEdid(drmModeConnector *connector);
    bool isCurrentMode(const drmModeModeInfo *mode) const;

    DrmBackend *m_backend;
    QPoint m_globalPos;
    quint32 m_crtcId = 0;
    quint32 m_connector = 0;
    quint32 m_lastStride = 0;
    drmModeModeInfo m_mode;
//     DrmBuffer *m_currentBuffer = nullptr;
//     DrmBuffer *m_blackBuffer = nullptr;
    struct CrtcCleanup {
        static void inline cleanup(_drmModeCrtc *ptr) {
            drmModeFreeCrtc(ptr);
        }
    };
    Edid m_edid;
    QScopedPointer<_drmModeCrtc, CrtcCleanup> m_savedCrtc;
    //QScopedPointer<KWayland::Server::OutputInterface> m_waylandOutput;
};


} // namespace

#endif // KSCREEN_DRM_OUTPUT_H
