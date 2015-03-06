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

#ifndef XRANDRCRTC_H
#define XRANDRCRTC_H

#include <QObject>
#include <QRect>
#include <QVector>
#include <QMap>

#include <xcb/randr.h>

class XRandRConfig;

class XRandRCrtc : public QObject
{
    Q_OBJECT

public:
    typedef QMap<xcb_randr_crtc_t, XRandRCrtc*> Map;


    XRandRCrtc(xcb_randr_crtc_t crtc, XRandRConfig *config);

    xcb_randr_crtc_t crtc() const;
    xcb_randr_mode_t mode() const;
    xcb_randr_rotation_t rotation() const;
    QRect geometry() const;
    QVector<xcb_randr_output_t> possibleOutputs();
    QVector<xcb_randr_output_t> outputs() const;

    bool connectOutput(xcb_randr_output_t output);
    void disconectOutput(xcb_randr_output_t output);

    bool isFree() const;

    void update();
    void update(xcb_randr_crtc_t mode, xcb_randr_rotation_t rotation, const QRect &geom);

private:
    xcb_randr_crtc_t m_crtc;
    xcb_randr_mode_t m_mode;
    xcb_randr_rotation_t m_rotation;
    QRect m_geometry;
    QVector<xcb_randr_output_t> m_possibleOutputs;
    QVector<xcb_randr_output_t> m_outputs;
};

#endif // XRANDRCRTC_H
