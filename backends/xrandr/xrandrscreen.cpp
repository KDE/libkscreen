/*************************************************************************************
 *  Copyright (C) 2012 by Alejandro Fiestas Olivares <afiestas@kde.org>              *
 *  Copyright (C) 2012 by Dan Vrátil <dvratil@redhat.com>                            *
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


#include "xrandrscreen.h"
#include "xrandrconfig.h"
#include "xlibandxrandr.h"

#include "screen.h"
#include "config.h"
#include <QX11Info>

XRandRScreen::XRandRScreen(XRandRConfig *config)
    : QObject(config)
{
    update();
}

XRandRScreen::~XRandRScreen()
{
}

void XRandRScreen::update()
{
    Display *display = QX11Info::display();
    int screen = DefaultScreen(display);
    Window rootWindow = XRootWindow(display, screen);

    XRRGetScreenSizeRange (display, rootWindow,
                           &m_minSize.rwidth(), &m_minSize.rheight(),
                           &m_maxSize.rwidth(), &m_maxSize.rheight());
    m_currentSize = QSize(DisplayWidth(display, screen),DisplayHeight(display, screen));
}

QSize XRandRScreen::currentSize()
{
    update();
    return m_currentSize;
}

KScreen::Screen *XRandRScreen::toKScreenScreen(KScreen::Config *parent) const
{
    KScreen::Screen *kscreenScreen = new KScreen::Screen(parent);
    kscreenScreen->setId(m_id);
    kscreenScreen->setMaxSize(m_maxSize);
    kscreenScreen->setMinSize(m_minSize);
    kscreenScreen->setCurrentSize(m_currentSize);

    return kscreenScreen;
}

void XRandRScreen::updateKScreenScreen(KScreen::Screen *screen) const
{
    screen->setCurrentSize(m_currentSize);
}


#include "xrandrscreen.moc"
