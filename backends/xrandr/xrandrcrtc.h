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

#include "xlibandxrandr.h"

class XRandRConfig;

class XRandRCrtc : public QObject
{
    Q_OBJECT

public:
    typedef QMap<RRCrtc, XRandRCrtc*> Map;


    XRandRCrtc(RRCrtc crtc, XRandRConfig *config);

    RRCrtc crtc() const;
    RRMode mode() const;
    Rotation rotation() const;
    QRect geometry() const;
    QVector<RROutput> possibleOutputs();
    QVector<RROutput> outputs() const;

    bool connectOutput(RROutput output);
    void disconectOutput(RROutput output);

    bool isFree() const;

    void update();
    void update(RRMode mode, Rotation rotation, const QRect &geom);

private:
    RRCrtc m_crtc;
    RRMode m_mode;
    Rotation m_rotation;
    QRect m_geometry;
    QVector<RROutput> m_possibleOutputs;
    QVector<RROutput> m_outputs;
};

#endif // XRANDRCRTC_H
