/*************************************************************************************
 *  Copyright (C) 2013 by Dan Vrátil <dvratil@redhat.com>                            *
 *                                                                                   *
 *  This program is free software; you can redistribute it and/or                    *
 *  modify it under the terms of the GNU General Public License                      *
 *  as published by the Free Software Foundation; either version 2                   *
 *  of the License, or (at your option) any later version.                           *
 *                                                                                   *
 *  This program is distributed in the hope that it will be useful,                  *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of                   *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                    *
 *  GNU General Public License for more details.                                     *
 *                                                                                   *
 *  You should have received a copy of the GNU General Public License                *
 *  along with this program; if not, write to the Free Software                      *
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA   *
 *************************************************************************************/

#include "xrandrx11helper.h"
#include "xrandr.h"
#include "xlibandxrandr.h"

#include <QX11Info>

#include <KSystemEventFilter>
#include <kdebug.h>

XRandRX11Helper::XRandRX11Helper():
    QWidget()
{
    XRRQueryVersion (QX11Info::display(), &m_versionMajor, &m_versionMinor);

    kDebug(dXndr()).nospace() << "Detected XRandR " << m_versionMajor << "." << m_versionMinor;

    XRRQueryExtension(QX11Info::display(), &m_randrBase, &m_randrError);

    kDebug(dXndr()) << "Event Base: " << m_randrBase;
    kDebug(dXndr()) << "Event Error: "<< m_randrError;



    m_window = XCreateSimpleWindow(QX11Info::display(),
                                   XRootWindow(QX11Info::display(), DefaultScreen(QX11Info::display()))
                                   , 0, 0, 1, 1, 0, 0, 0 );

    XRRSelectInput(QX11Info::display(), m_window,
                   RRScreenChangeNotifyMask | RRCrtcChangeNotifyMask |
                   RROutputChangeNotifyMask | RROutputPropertyNotifyMask);

    KSystemEventFilter::installEventFilter(this);
}

XRandRX11Helper::~XRandRX11Helper()
{
    KSystemEventFilter::removeEventFilter(this);
    XDestroyWindow(QX11Info::display(), m_window);
}

QString XRandRX11Helper::rotationToString(Rotation rotation)
{
    switch (rotation) {
        case RR_Rotate_0:
            return "RR_Rotate_0";
        case RR_Rotate_90:
            return "RR_Rotate_90";
        case RR_Rotate_180:
            return "RR_Rotate_180";
        case RR_Rotate_270:
            return "RR_Rotate_270";
    }

    return QString("invalid value (%1)").arg(rotation);
}

QString XRandRX11Helper::connectionToString(Connection connection)
{
    switch (connection) {
        case RR_Connected:
            return "RR_Connected";
        case RR_Disconnected:
            return "RR_Disconnected";
        case RR_UnknownConnection:
            return "RR_UnknownConnection";
    }

    return QString("invalid value (%1)").arg(connection);
}



bool XRandRX11Helper::x11Event(XEvent *event)
{
    /* XRandR <= 1.1 */
    if (m_versionMajor >= 1 && m_versionMinor <= 1) {
        if (event->xany.type == m_randrBase + RRScreenChangeNotify) {
            KDebug::Block changeNotify("RRScreenChangeNotify", dXndr());

            XRRScreenChangeNotifyEvent* e2 = reinterpret_cast< XRRScreenChangeNotifyEvent* >(event);

            kDebug(dXndr()) << "Timestamp: " << e2->timestamp;
            kDebug(dXndr()) << "Window: " << e2->window;
            kDebug(dXndr()) << "Root: "<< e2->root;
            kDebug(dXndr()) << "Size Index: " << e2->size_index;
            kDebug(dXndr()) << "Subpixel Order:" << e2->subpixel_order;
            kDebug(dXndr()) << "Rotation: " << rotationToString(e2->rotation);
            kDebug(dXndr()) << "Size: " << e2->width << e2->height;
            kDebug(dXndr()) << "SizeMM: " << e2->mwidth << e2->mheight;

            Q_EMIT outputsChanged();
        }

        return false;
    }

    /* XRandR >= 1.2 */
    if (event->xany.type == m_randrBase + RRNotify) {
        XRRNotifyEvent* e2 = reinterpret_cast< XRRNotifyEvent* >(event);
        if (e2->subtype == RRNotify_CrtcChange) {
            XRRCrtcChangeNotifyEvent* e2 = reinterpret_cast< XRRCrtcChangeNotifyEvent* >(event);

            KDebug::Block crtcChange("RRNotify_CrtcChange", dXndr());
            kDebug(dXndr()) << "CRTC: " << e2->crtc;
            kDebug(dXndr()) << "Mode: " << e2->mode;
            kDebug(dXndr()) << "Rotation: " << rotationToString(e2->rotation);
            kDebug(dXndr()) << "Geometry: " << e2->x << e2->y << e2->width << e2->height;

            Q_EMIT crtcChanged(e2->crtc);

        } else if (e2->subtype == RRNotify_OutputChange) {
            XRROutputChangeNotifyEvent* e2 = reinterpret_cast< XRROutputChangeNotifyEvent* >(event);

            KDebug::Block outputChange("RRNotify_OutputChange", dXndr());
            kDebug(dXndr()) << "Output: " << e2->output;
            kDebug(dXndr()) << "CRTC: " << e2->crtc;
            kDebug(dXndr()) << "Mode: " << e2->mode;
            kDebug(dXndr()) << "Rotation: " << rotationToString(e2->rotation);
            kDebug(dXndr()) << "Connection: " << connectionToString(e2->connection);
            kDebug(dXndr()) << "Subpixel Order: " << e2->subpixel_order;

            Q_EMIT outputChanged(e2->output);

        } else if (e2->subtype == RRNotify_OutputProperty) {
            XRROutputPropertyNotifyEvent* e2 = reinterpret_cast< XRROutputPropertyNotifyEvent* >(event);

            char *atom_name = XGetAtomName(QX11Info::display(), e2->property);
            KDebug::Block changeProperty("RRNotify_Property", dXndr());
            kDebug(dXndr()) << "Timestamp: " << e2->timestamp;
            kDebug(dXndr()) << "Output: " << e2->output;
            kDebug(dXndr()) << "Property: " << XGetAtomName(QX11Info::display(), e2->property);
            kDebug(dXndr()) << "State (newValue, Deleted): " << e2->state;
            XFree(atom_name);
        }
    }

    return false;
}

#include "xrandrx11helper.moc"
