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

bool XRandR::has_1_3 = false;
XRandR *XRandR::s_instance = 0;

XRandR* XRandR::self()
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

    int majorVersion, minorVersion;
    XRRQueryVersion(display, &majorVersion, &minorVersion);

    if (minorVersion == 1 && majorVersion == 1) {
        //1.1 doesn't support more than one output, so makes no sense to support it
        kDebug() << "The needed XRandR version to run this lib is 1.1";
        return 0;
    }

    XRandR::has_1_3 = (majorVersion > 1 || (majorVersion == 1 && minorVersion >= 3));

    kDebug() << "Using XRandR " << majorVersion << " " << minorVersion;

    s_instance = new XRandR();
    s_instance->setVersion(minorVersion, majorVersion);
    s_instance->setDisplay(display);

    return s_instance;
}

XRandR::XRandR() : QObject()
{
}

QPair< int, int > XRandR::version()
{
    return m_version;
}

void XRandR::setVersion(int major, int minor)
{
    m_version = QPair<int, int>(minor, major);
}

void XRandR::setDisplay(Display* display)
{
    m_display = display;
}

QList< QRandR::Screen* > XRandR::screens()
{
    int numScreens = ScreenCount(m_display);
    for (int i = 0; i < numScreens; i++) {
        m_screens.append(new QRandR::Screen(i, m_display));
    }

    return m_screens;
}
