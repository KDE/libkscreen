/*************************************************************************************
 *  Copyright (C) 2011 by Alex Fiestas <afiestas@kde.org>                            *
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

#include <QX11Info>
#include <QtCore/QTextStream>
#include <KDebug>

#include <X11/extensions/Xrandr.h>

#include "xrandr.h"
#include "screen.h"

namespace QRandR {

int QRandR::s_RRNotify = 0;
int QRandR::s_RRScreen = 0;
int QRandR::s_Timestamp = 0;
bool QRandR::has_1_3 = false;
QRandR *QRandR::s_instance = 0;
QCoreApplication::EventFilter QRandR::s_oldEventFilter = 0;

QRandR* QRandR::self()
{
    if (s_instance) {
        return s_instance;
    }

    Display *display = QX11Info::display();

    int eventBase, errorBase;
    if(XRRQueryExtension(display, &eventBase, &errorBase) == False) {
        kDebug() << "The XRandR extension doesn't seems to be installed";
        return 0;
    }

    s_RRNotify = eventBase + RRNotify;
    s_RRScreen = eventBase + RRScreenChangeNotify;

    int majorVersion, minorVersion;
    XRRQueryVersion(display, &majorVersion, &minorVersion);

    if (minorVersion == 1 && majorVersion == 1) {
        //1.1 doesn't support more than one output, so makes no sense to support it
        kDebug() << "The needed XRandR version to run this lib is 1.1";
        return 0;
    }

    QRandR::has_1_3 = (majorVersion > 1 || (majorVersion == 1 && minorVersion >= 3));

    kDebug() << "Using XRandR " << majorVersion << " " << minorVersion;

    s_instance = new QRandR();
    s_instance->setVersion(minorVersion, majorVersion);
    s_instance->setDisplay(display);

    s_oldEventFilter = QCoreApplication::instance()->setEventFilter(QRandR::x11EventFilter);

    return s_instance;
}

bool QRandR::x11EventFilter(void* message, long int* result)
{
    XEvent *event = reinterpret_cast<XEvent*>(message);
    if (event->type == s_RRNotify || event->type == s_RRScreen) {
        s_instance->handleEvent(event);
    }

    if (s_oldEventFilter) {
        return s_oldEventFilter(message, result);
    }

    //If we don't have any previous eventFilter we assume that we don't have any GUI ergo
    //we can stop any propagation
    return true;
}

QRandR::QRandR() : QObject()
, m_display(0)
{
}

QPair< int, int > QRandR::version()
{
    return m_version;
}

void QRandR::setVersion(int major, int minor)
{
    m_version = QPair<int, int>(minor, major);
}

void QRandR::setDisplay(Display* display)
{
    m_display = display;
}

void QRandR::handleEvent(XEvent* event)
{
    if (event->type == QRandR::s_RRScreen) {
        XRRScreenChangeNotifyEvent *screenEvent = (XRRScreenChangeNotifyEvent*) event;
        qDebug() << "RRScreenChangeNotify";
        passEventToScreens(screenEvent);
    } else if (event->type == QRandR::s_RRNotify) {
        XRRNotifyEvent *notifyEvent = (XRRNotifyEvent*) event;
        qDebug() << "RRNotify";
        if (notifyEvent->subtype == RRNotify_CrtcChange) {
            qDebug() << "RRNotify_CrtcChange";
        } else if (notifyEvent->subtype == RRNotify_OutputChange) {
            XRROutputChangeNotifyEvent *rolf = (XRROutputChangeNotifyEvent*) notifyEvent;
            qDebug() << "RRNotify_OutputChange: " << rolf->output;
        } else {
            qDebug() << "RRNotify_OutputProperty";
        }
    }
}

void QRandR::passEventToScreens(XRRScreenChangeNotifyEvent* event)
{
    Q_FOREACH(Screen *screen, m_screens) {
        screen->handleEvent(event);
    }
}

Display* QRandR::display()
{
    return m_display;
}

Screen* QRandR::screen()
{
    if (m_screens.isEmpty()) {
        screens();
    }

    return m_screens.at(DefaultScreen(m_display));
}

QList< Screen* > QRandR::screens()
{
    int numScreens = ScreenCount(m_display);
    for (int i = 0; i < numScreens; i++) {
        m_screens.append(new Screen(i, m_display));
    }

    return m_screens;
}
}