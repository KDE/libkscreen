/*************************************************************************************
 *  Copyright 2012, 2013  Daniel Vrátil <dvratil@redhat.com>                         *
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

#include "xrandrxcbhelper.h"
#include "xrandr.h"
#include "xlibandxrandr.h"

#include <QX11Info>

Q_LOGGING_CATEGORY(KSCREEN_XCB_HELPER, "kscreen.xcb.helper")
XRandRXCBHelper::XRandRXCBHelper():
    QObject(),
    m_randrBase(0),
    m_randrError(0),
    m_versionMajor(0),
    m_versionMinor(0),
    m_window(0)
{
    QLoggingCategory::setFilterRules(QStringLiteral("kscreen.xcb.helper = true"));
    XRRQueryVersion (QX11Info::display(), &m_versionMajor, &m_versionMinor);

    qCDebug(KSCREEN_XCB_HELPER).nospace() << "Detected XRandR " << m_versionMajor << "." << m_versionMinor;

    XRRQueryExtension(QX11Info::display(), &m_randrBase, &m_randrError);

    qCDebug(KSCREEN_XCB_HELPER) << "Event Base: " << m_randrBase;
    qCDebug(KSCREEN_XCB_HELPER) << "Event Error: "<< m_randrError;



    m_window = XCreateSimpleWindow(QX11Info::display(),
                                   XRootWindow(QX11Info::display(), DefaultScreen(QX11Info::display()))
                                   , 0, 0, 1, 1, 0, 0, 0 );

    XRRSelectInput(QX11Info::display(), m_window,
                   RRScreenChangeNotifyMask | RRCrtcChangeNotifyMask |
                   RROutputChangeNotifyMask | RROutputPropertyNotifyMask);
}

XRandRXCBHelper::~XRandRXCBHelper()
{
    if (m_window) {
        XDestroyWindow(QX11Info::display(), m_window);
    }
}

QString XRandRXCBHelper::rotationToString(Rotation rotation)
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

QString XRandRXCBHelper::connectionToString(Connection connection)
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

bool XRandRXCBHelper::nativeEventFilter(const QByteArray& eventType, void* message, long int* result)
{
    return true;
}

bool XRandRXCBHelper::x11Event(XEvent *event)
{
    /* XRandR <= 1.1 */
    if (m_versionMajor == 1 && m_versionMinor <= 1) {
        if (event->xany.type == m_randrBase + RRScreenChangeNotify) {

            XRRScreenChangeNotifyEvent* e2 = reinterpret_cast< XRRScreenChangeNotifyEvent* >(event);

            qCDebug(KSCREEN_XCB_HELPER) << "Timestamp: " << e2->timestamp;
            qCDebug(KSCREEN_XCB_HELPER) << "Window: " << e2->window;
            qCDebug(KSCREEN_XCB_HELPER) << "Root: "<< e2->root;
            qCDebug(KSCREEN_XCB_HELPER) << "Size Index: " << e2->size_index;
            qCDebug(KSCREEN_XCB_HELPER) << "Subpixel Order:" << e2->subpixel_order;
            qCDebug(KSCREEN_XCB_HELPER) << "Rotation: " << rotationToString(e2->rotation);
            qCDebug(KSCREEN_XCB_HELPER) << "Size: " << e2->width << e2->height;
            qCDebug(KSCREEN_XCB_HELPER) << "SizeMM: " << e2->mwidth << e2->mheight;

            Q_EMIT outputsChanged();
        }

        return false;
    }

    /* XRandR >= 1.2 */
    if (event->xany.type == m_randrBase + RRNotify) {
        XRRNotifyEvent* e2 = reinterpret_cast< XRRNotifyEvent* >(event);
        if (e2->subtype == RRNotify_CrtcChange) {
            XRRCrtcChangeNotifyEvent* e2 = reinterpret_cast< XRRCrtcChangeNotifyEvent* >(event);

            qCDebug(KSCREEN_XCB_HELPER) << "CRTC: " << e2->crtc;
            qCDebug(KSCREEN_XCB_HELPER) << "Mode: " << e2->mode;
            qCDebug(KSCREEN_XCB_HELPER) << "Rotation: " << rotationToString(e2->rotation);
            qCDebug(KSCREEN_XCB_HELPER) << "Geometry: " << e2->x << e2->y << e2->width << e2->height;

            Q_EMIT crtcChanged(e2->crtc);

        } else if (e2->subtype == RRNotify_OutputChange) {
            XRROutputChangeNotifyEvent* e2 = reinterpret_cast< XRROutputChangeNotifyEvent* >(event);

            qCDebug(KSCREEN_XCB_HELPER) << "Output: " << e2->output;
            qCDebug(KSCREEN_XCB_HELPER) << "CRTC: " << e2->crtc;
            qCDebug(KSCREEN_XCB_HELPER) << "Mode: " << e2->mode;
            qCDebug(KSCREEN_XCB_HELPER) << "Rotation: " << rotationToString(e2->rotation);
            qCDebug(KSCREEN_XCB_HELPER) << "Connection: " << connectionToString(e2->connection);
            qCDebug(KSCREEN_XCB_HELPER) << "Subpixel Order: " << e2->subpixel_order;

            Q_EMIT outputChanged(e2->output);

        } else if (e2->subtype == RRNotify_OutputProperty) {
            XRROutputPropertyNotifyEvent* e2 = reinterpret_cast< XRROutputPropertyNotifyEvent* >(event);

            char *atom_name = XGetAtomName(QX11Info::display(), e2->property);
            qCDebug(KSCREEN_XCB_HELPER) << "Timestamp: " << e2->timestamp;
            qCDebug(KSCREEN_XCB_HELPER) << "Output: " << e2->output;
            qCDebug(KSCREEN_XCB_HELPER) << "Property: " << XGetAtomName(QX11Info::display(), e2->property);
            qCDebug(KSCREEN_XCB_HELPER) << "State (newValue, Deleted): " << e2->state;
            XFree(atom_name);
        }
    }

    return false;
}

#include "xrandrxcbhelper.moc"
