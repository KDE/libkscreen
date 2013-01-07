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

#include <KSystemEventFilter>
#include <QDebug>

XRandRX11Helper::XRandRX11Helper():
    QWidget()
{
    m_debugMode = (getenv("KSCREEN_XRANDR_DEBUG") != NULL);

    XRRQueryVersion (XRandR::display(), &m_versionMajor, &m_versionMinor);
    if (m_debugMode) {
        qDebug().nospace() << "Detected XRandR " << m_versionMajor << "." << m_versionMinor;
    }

    XRRQueryExtension(XRandR::display(), &m_randrBase, &m_randrError);
    if (m_debugMode) {
        qDebug() << "Event Base: " << m_randrBase;
        qDebug() << "Event Error: "<< m_randrError;
    }


    XRRSelectInput(XRandR::display(), XRandR::rootWindow(), 0);
    XRRSelectInput(XRandR::display(), XRandR::rootWindow(),
                   RRScreenChangeNotifyMask | RRCrtcChangeNotifyMask |
                   RROutputChangeNotifyMask | RROutputPropertyNotifyMask |
                   RRProviderChangeNotifyMask | RRProviderPropertyNotifyMask |
                   RRResourceChangeNotifyMask);

    KSystemEventFilter::installEventFilter(this);
}

XRandRX11Helper::~XRandRX11Helper()
{
    KSystemEventFilter::removeEventFilter(this);
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
            qDebug() << "RRScreenChangeNotify";
            Q_EMIT outputsChanged();

            XRRScreenChangeNotifyEvent* e2 = reinterpret_cast< XRRScreenChangeNotifyEvent* >(event);

            if (m_debugMode) {
                qDebug() << "\tTimestamp: " << e2->timestamp;
                qDebug() << "\tWindow: " << e2->window;
                qDebug() << "\tRoot: "<< e2->root;
                qDebug() << "\tSize Index: " << e2->size_index;
                qDebug() << "\tSubpixel Order:" << e2->subpixel_order;
                qDebug() << "\tRotation: " << rotationToString(e2->rotation);
                qDebug() << "\tSize: " << e2->width << e2->height;
                qDebug() << "\tSizeMM: " << e2->mwidth << e2->mheight;
                qDebug();
            }
        }

        return false;
    }

    /* XRandR >= 1.2 */
    if (event->xany.type == m_randrBase + RRNotify) {
        XRRNotifyEvent* e2 = reinterpret_cast< XRRNotifyEvent* >(event);
        if (e2->subtype == RRNotify_CrtcChange) {
            XRRCrtcChangeNotifyEvent* e2 = reinterpret_cast< XRRCrtcChangeNotifyEvent* >(event);

            if (m_debugMode) {
                qDebug() << "RRNotify_CrtcChange";
                qDebug() << "\tCRTC: " << e2->crtc;
                qDebug() << "\tMode: " << e2->mode;
                qDebug() << "\tRotation: " << rotationToString(e2->rotation);
                qDebug() << "\tGeometry: " << e2->x << e2->y << e2->width << e2->height;
                qDebug();
            }

            Q_EMIT crtcChanged(e2->crtc);

        } else if (e2->subtype == RRNotify_OutputChange) {
            XRROutputChangeNotifyEvent* e2 = reinterpret_cast< XRROutputChangeNotifyEvent* >(event);

            if (m_debugMode) {
                qDebug() << "RRNotify_OutputChange";
                qDebug() << "\tOutput: " << e2->output;
                qDebug() << "\tCRTC: " << e2->crtc;
                qDebug() << "\tMode: " << e2->mode;
                qDebug() << "\tRotation: " << rotationToString(e2->rotation);
                qDebug() << "\tConnection: " << connectionToString(e2->connection);
                qDebug() << "\tSubpixel Order: " << e2->subpixel_order;
                qDebug();
            }

            Q_EMIT outputChanged(e2->output);

        } else if (e2->subtype == RRNotify_OutputProperty) {
            XRROutputPropertyNotifyEvent* e2 = reinterpret_cast< XRROutputPropertyNotifyEvent* >(event);

            if (m_debugMode) {
                char *atom_name = XGetAtomName(XRandR::display(), e2->property);
                qDebug() << "RRNotify_Property";
                qDebug() << "\tTimestamp: " << e2->timestamp;
                qDebug() << "\tOutput: " << e2->output;
                qDebug() << "\tProperty: " << XGetAtomName(XRandR::display(), e2->property);
                qDebug() << "\tState (newValue, Deleted): " << e2->state;
                qDebug();
                XFree(atom_name);
            }

        } else if (e2->subtype == RRNotify_ProviderChange) {
            XRRProviderPropertyNotifyEvent* e2 = reinterpret_cast< XRRProviderPropertyNotifyEvent* >(event);

            if (m_debugMode) {
                char *atom_name = XGetAtomName(XRandR::display(), e2->property);
                qDebug() << "RRNotify_ProviderProperty";
                qDebug() << "\tTimestamp: " << e2->timestamp;
                qDebug() << "\tProvider: " << e2->provider;
                qDebug() << "\tProperty: " << atom_name;
                qDebug() << "\tState (newValue, Deleted): " << e2->state;
                qDebug();
                XFree(atom_name);
            }

        } else if (e2->subtype == RRNotify_ResourceChange) {
            XRRResourceChangeNotifyEvent* e2 = reinterpret_cast< XRRResourceChangeNotifyEvent* >(event);

            if (m_debugMode) {
                qDebug() << "RRNotify_ResourceChange";
                qDebug() << "\tTimestamp: " << e2->timestamp;
                qDebug();
            }
        }
    }

    return false;
}

#include "xrandrx11helper.moc"
