/*
 * SPDX-FileCopyrightText: 2015 Daniel Vrátil <dvratil@redhat.com>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 *
 */
#pragma once

#include <QMap>
#include <QObject>
#include <QRect>
#include <QVector>

#include <xcb/randr.h>

class XRandRConfig;

class XRandRCrtc : public QObject
{
    Q_OBJECT

public:
    typedef QMap<xcb_randr_crtc_t, XRandRCrtc *> Map;

    XRandRCrtc(xcb_randr_crtc_t crtc, XRandRConfig *config);

    xcb_randr_crtc_t crtc() const;
    xcb_randr_mode_t mode() const;

    QRect geometry() const;
    xcb_randr_rotation_t rotation() const;

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

    QRect m_geometry;
    xcb_randr_rotation_t m_rotation;

    QVector<xcb_randr_output_t> m_possibleOutputs;
    QVector<xcb_randr_output_t> m_outputs;
};
