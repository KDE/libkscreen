/*
 * Copyright 2015  Daniel Vrátil <dvratil@redhat.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "xrandrcrtc.h"
#include "xrandrconfig.h"
#include "xrandr.h"

#include "../xcbwrapper.h"

XRandRCrtc::XRandRCrtc(xcb_randr_crtc_t crtc, XRandRConfig *config)
    : QObject(config)
    , m_crtc(crtc)
    , m_mode(0)
    , m_rotation(XCB_RANDR_ROTATION_ROTATE_0)
{
    update();
}

xcb_randr_crtc_t XRandRCrtc::crtc() const
{
    return m_crtc;
}

xcb_randr_mode_t XRandRCrtc::mode() const
{
    return m_mode;
}

QRect XRandRCrtc::geometry() const
{
    return m_geometry;
}

xcb_randr_rotation_t XRandRCrtc::rotation() const
{
    return m_rotation;
}

QVector<xcb_randr_output_t> XRandRCrtc::possibleOutputs()
{
    return m_possibleOutputs;
}

QVector<xcb_randr_output_t> XRandRCrtc::outputs() const
{
    return m_outputs;
}

bool XRandRCrtc::connectOutput(xcb_randr_output_t output)
{
    update();
    qCDebug(KSCREEN_XRANDR) << "Connected output" << output << "to CRTC" << m_crtc;
    if (!m_possibleOutputs.contains(output)) {
        qCDebug(KSCREEN_XRANDR) << "Output" << output << "is not an allowed output for CRTC" << m_crtc;
        return false;
    }

    if (!m_outputs.contains(output)) {
        m_outputs.append(output);
    }
    return true;
}

void XRandRCrtc::disconectOutput(xcb_randr_output_t output)
{
    update();
    qCDebug(KSCREEN_XRANDR) << "Disconnected output" << output << "from CRTC" << m_crtc;
    const int index = m_outputs.indexOf(output);
    if (index > -1) {
        m_outputs.remove(index);
    }
}

bool XRandRCrtc::isFree() const
{
    return m_outputs.isEmpty();
}

void XRandRCrtc::update()
{
    XCB::CRTCInfo crtcInfo(m_crtc, XCB_TIME_CURRENT_TIME);
    m_mode = crtcInfo->mode;
    m_rotation = (xcb_randr_rotation_t) crtcInfo->rotation;
    m_geometry = QRect(crtcInfo->x, crtcInfo->y, crtcInfo->width, crtcInfo->height);
    m_possibleOutputs.clear();
    m_possibleOutputs.reserve(crtcInfo->num_possible_outputs);
    xcb_randr_output_t *possible = xcb_randr_get_crtc_info_possible(crtcInfo);
    for (int i = 0; i < crtcInfo->num_possible_outputs; ++i) {
        m_possibleOutputs.append(possible[i]);
    }

    m_outputs.clear();
    xcb_randr_output_t *outputs = xcb_randr_get_crtc_info_outputs(crtcInfo);
    for (int i = 0; i < crtcInfo->num_outputs; ++i) {
        m_outputs.append(outputs[i]);
    }
}

void XRandRCrtc::update(xcb_randr_mode_t mode, xcb_randr_rotation_t rotation, const QRect &geom)
{
    m_mode = mode;
    m_rotation = rotation;
    m_geometry = geom;
}

