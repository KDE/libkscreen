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

#include <xcb/randr.h>

#include <QX11Info>
#include <QGuiApplication>

Q_LOGGING_CATEGORY(KSCREEN_XCB_HELPER, "kscreen.xcb.helper")
XRandRXCBHelper::XRandRXCBHelper():
    QObject(),
    m_isRandrPresent(false),
    m_event11(false),
    m_randrBase(0),
    m_randrErrorBase(0),
    m_majorOpcode(0),
    m_eventType(0),
    m_versionMajor(0),
    m_versionMinor(0),
    m_window(0)
{
    QLoggingCategory::setFilterRules(QStringLiteral("kscreen.xcb.helper = true"));

    xcb_connection_t* c = QX11Info::connection();
    xcb_prefetch_extension_data(c, &xcb_randr_id);
    xcb_randr_query_version_cookie_t cookie = xcb_randr_query_version(c,
                                                                      XCB_RANDR_MAJOR_VERSION,
                                                                      XCB_RANDR_MINOR_VERSION
                                                                     );
    const xcb_query_extension_reply_t *queryExtension = xcb_get_extension_data(c, &xcb_randr_id);
    if (!queryExtension) {
        qCDebug(KSCREEN_XCB_HELPER) << "Fail to query for xrandr extension";
        return;
    }
    if (!queryExtension->present) {
        qCDebug(KSCREEN_XCB_HELPER) << "XRandR extension is not present at all";
        return;
    }

    m_isRandrPresent = queryExtension->present;
    m_randrBase = queryExtension->first_event;
    m_randrErrorBase = queryExtension->first_error;
    m_majorOpcode = queryExtension->major_opcode;

    xcb_generic_error_t *error = NULL;
    xcb_randr_query_version_reply_t* versionReply = xcb_randr_query_version_reply(c, cookie, &error);
    Q_ASSERT_X(versionReply, "xrandrxcbhelper", "Query to fetch xrandr version failed");
    if (error) {
        qFatal("Error while querying for xrandr version: %d", error->error_code);
    }
    m_versionMajor = versionReply->major_version;
    m_versionMinor = versionReply->minor_version;
    free(versionReply);

    if (m_versionMajor == 1 && m_versionMinor <= 1) {
        m_event11 = true;
    }
    qCDebug(KSCREEN_XCB_HELPER).nospace() << "Detected XRandR " << m_versionMajor << "." << m_versionMinor;
    qCDebug(KSCREEN_XCB_HELPER) << "Event Base: " << m_randrBase;
    qCDebug(KSCREEN_XCB_HELPER) << "Event Error: "<< m_randrErrorBase;

    uint32_t rWindow = QX11Info::appRootWindow();
    m_window = xcb_generate_id(c);
    xcb_create_window(c, XCB_COPY_FROM_PARENT, m_window,
                      rWindow,
                      0, 0, 1, 1, 0, XCB_COPY_FROM_PARENT,
                      XCB_COPY_FROM_PARENT, 0, NULL);

    xcb_randr_select_input(c, m_window,
            XCB_RANDR_NOTIFY_MASK_SCREEN_CHANGE |
            XCB_RANDR_NOTIFY_MASK_OUTPUT_CHANGE |
            XCB_RANDR_NOTIFY_MASK_CRTC_CHANGE |
            XCB_RANDR_NOTIFY_MASK_OUTPUT_PROPERTY
    );

    qApp->installNativeEventFilter(this);
}

XRandRXCBHelper::~XRandRXCBHelper()
{
    if (m_window) {
        xcb_destroy_window(QX11Info::connection(), m_window);
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
    Q_UNUSED(result);

    if (eventType != "xcb_generic_event_t") {
        return false;
    }

    xcb_generic_event_t* e = static_cast<xcb_generic_event_t *>(message);
    const uint8_t xEventType = e->response_type & ~0x80;

    //If this event is not xcb_randr_notify, we don't want it
    if (xEventType == m_randrBase + XCB_RANDR_SCREEN_CHANGE_NOTIFY && m_event11) {
        handleScreenChange(e);
    }
    if (xEventType == m_randrBase + XCB_RANDR_NOTIFY) {
        handleXRandRNotify(e);
    }

    return false;
}

void XRandRXCBHelper::handleScreenChange(xcb_generic_event_t* e)
{
    xcb_randr_screen_change_notify_event_t *e2 =
        (xcb_randr_screen_change_notify_event_t *) e;
    qCDebug(KSCREEN_XCB_HELPER) << "Timestamp: " << e2->timestamp;
    qCDebug(KSCREEN_XCB_HELPER) << "Window: " << e2->request_window;
    qCDebug(KSCREEN_XCB_HELPER) << "Root: "<< e2->root;
    qCDebug(KSCREEN_XCB_HELPER) << "Subpixel Order:" << e2->subpixel_order;
    qCDebug(KSCREEN_XCB_HELPER) << "Rotation: " << rotationToString(e2->rotation);
    qCDebug(KSCREEN_XCB_HELPER) << "Size: " << e2->width << e2->height;
    qCDebug(KSCREEN_XCB_HELPER) << "SizeMM: " << e2->mwidth << e2->mheight;

    Q_EMIT outputsChanged();
}

void XRandRXCBHelper::handleXRandRNotify(xcb_generic_event_t* e)
{
    xcb_randr_notify_event_t*
    randrEvent = reinterpret_cast<xcb_randr_notify_event_t*>(e);

    if (randrEvent->subCode == XCB_RANDR_NOTIFY_CRTC_CHANGE) {
        xcb_randr_crtc_change_t crtc = randrEvent->u.cc;
        qCDebug(KSCREEN_XCB_HELPER) << "CRTC CHANGE";
        qCDebug(KSCREEN_XCB_HELPER) << "CRTC: " << crtc.crtc;
        qCDebug(KSCREEN_XCB_HELPER) << "Mode: " << crtc.mode;
        qCDebug(KSCREEN_XCB_HELPER) << "Rotation: " << rotationToString(crtc.rotation);
        qCDebug(KSCREEN_XCB_HELPER) << "Geometry: " << crtc.x << crtc.y << crtc.width << crtc.height;

        Q_EMIT crtcChanged(crtc.crtc);
    } else if(randrEvent->subCode == XCB_RANDR_NOTIFY_OUTPUT_CHANGE) {
        xcb_randr_output_change_t output = randrEvent->u.oc;
        qCDebug(KSCREEN_XCB_HELPER) << "OUTPUT CHANGE";
        qCDebug(KSCREEN_XCB_HELPER) << "Output: " << output.output;
        qCDebug(KSCREEN_XCB_HELPER) << "CRTC: " << output.crtc;
        qCDebug(KSCREEN_XCB_HELPER) << "Mode: " << output.mode;
        qCDebug(KSCREEN_XCB_HELPER) << "Rotation: " << rotationToString(output.rotation);
        qCDebug(KSCREEN_XCB_HELPER) << "Connection: " << connectionToString(output.connection);
        qCDebug(KSCREEN_XCB_HELPER) << "Subpixel Order: " << output.subpixel_order;

        Q_EMIT outputChanged(output.output);
    } else if(randrEvent->subCode == XCB_RANDR_NOTIFY_OUTPUT_PROPERTY) {
        xcb_randr_output_property_t property = randrEvent->u.op;

        xcb_get_atom_name_reply_t *reply = xcb_get_atom_name_reply(QX11Info::connection(), xcb_get_atom_name(QX11Info::connection(), property.atom), NULL) ;

        qCDebug(KSCREEN_XCB_HELPER) << "OUTPUT PROPERTY";
        qCDebug(KSCREEN_XCB_HELPER) << "Output: " << property.output;
        qCDebug(KSCREEN_XCB_HELPER) << "Property: " << xcb_get_atom_name_name(reply);
        qCDebug(KSCREEN_XCB_HELPER) << "State (newValue, Deleted): " << property.status;
        free(reply);
    }
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
