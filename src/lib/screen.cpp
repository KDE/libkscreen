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

#include "screen.h"
#include "crtc.h"

#include <QDebug>

namespace QRandR {

Screen::Screen(int screenId, Display *display) : QObject()
, m_id(screenId)
, m_display(display)
, m_rootWindow(0)
, m_resources(0)
{
}

Screen::~Screen()
{

}

const QSize Screen::minSize()
{
    if (!m_minSize.isEmpty()) {
        return m_minSize;
    }

    getMinAndMaxSize();

    return m_minSize;
}

const QSize Screen::maxSize()
{
    if (!m_maxSize.isEmpty()) {
        return m_maxSize;
    }

    getMinAndMaxSize();

    return m_maxSize;
}

const QSize Screen::currentSize()
{
    if (!m_currentSize.isEmpty()) {
        return m_currentSize;
    }

    m_currentSize.setWidth(DisplayWidth(m_display, m_id));
    m_currentSize.setHeight(DisplayHeight(m_display, m_id));

    return m_currentSize;
}

QList< Crtc* > Screen::crtc()
{
    if (!m_crtc.isEmpty()) {
        return m_crtc;
    }

    XRRScreenResources *screenResources = resources();

    for (int i = 0; i < screenResources->ncrtc; ++i)
    {
        m_crtc.append(new QRandR::Crtc(this, screenResources->crtcs[i]));
    }

    return m_crtc;
}

void Screen::getMinAndMaxSize()
{
    int minWidth, minHeight, maxWidth, maxHeight;
    XRRGetScreenSizeRange (m_display, rootWindow(), &minWidth, &minHeight,
                           &maxWidth, &maxHeight);

    m_minSize.setWidth(minWidth);
    m_minSize.setHeight(minHeight);

    m_maxSize.setWidth(maxWidth);
    m_maxSize.setHeight(maxHeight);
}

Window Screen::rootWindow()
{
    if (m_rootWindow != 0) {
        return m_rootWindow;
    }

    m_rootWindow = XRootWindow(m_display, m_id);
    return m_rootWindow;
}

XRRScreenResources* Screen::resources()
{
    if (!m_resources) {
        m_resources = XRRGetScreenResources(m_display, rootWindow());
    }

    return m_resources;
}


}