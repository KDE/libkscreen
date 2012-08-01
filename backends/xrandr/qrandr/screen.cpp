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
#include "output.h"
#include "mode.h"
#include "xrandr.h"

#include <QtCore/QDebug>

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
    XRRFreeScreenResources(m_resources);
    m_resources = 0;
}

int Screen::id()
{
    return m_id;
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

QHash<RRCrtc, Crtc *> Screen::crtc()
{
    if (!m_crtc.isEmpty()) {
        return m_crtc;
    }

    XRRScreenResources *screenResources = resources();

    RRCrtc id;
    for (int i = 0; i < screenResources->ncrtc; ++i)
    {
        id = screenResources->crtcs[i];
        m_crtc[id] =new Crtc(this, id);
    }

    return m_crtc;
}

QHash<RROutput, Output *> Screen::outputs()
{
    if (!m_outputs.isEmpty()) {
        return m_outputs;
    }

    XRRScreenResources *screenResources = resources();

    RROutput id;
    m_primary = XRRGetOutputPrimary(m_display, rootWindow());

    for (int i = 0; i < screenResources->noutput; ++i)
    {
        id = screenResources->outputs[i];
        m_outputs[id] = new Output(this, id);
        if (m_primary == id) {
            m_outputs[id]->setPrimary(true);
        }
    }

    return m_outputs;
}

QHash<RRMode,  Mode* > Screen::modes()
{
    if (!m_modes.isEmpty()) {
        return m_modes;
    }

    RRMode id;
    XRRScreenResources *screenResources = resources();
    for (int i = 0; i < screenResources->nmode; ++i)
    {
        id = screenResources->modes[i].id;
        m_modes[id] = new Mode(this,  &screenResources->modes[i]);
    }

    return m_modes;
}

Mode* Screen::mode(RRMode id)
{
     if (m_modes.isEmpty()) {
         modes();
     }

     return m_modes.value(id);
}

Crtc* Screen::crtc(RRCrtc id)
{
    if (m_crtc.isEmpty()) {
        crtc();
    }

    return m_crtc.value(id);
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
        QRandR::s_Timestamp = m_resources->timestamp;
    }

    return m_resources;
}

void Screen::setPrimaryOutput(Output* output)
{
    qDebug() << "Setting primary: " << output->name();
    XRRSetOutputPrimary(m_display, rootWindow(), output->id());
}

void Screen::handleEvent(XRRScreenChangeNotifyEvent* event)
{
    qDebug() << "Handle Event";

    QRandR::s_Timestamp = event->timestamp;
    m_currentSize.setWidth(event->width);
    m_currentSize.setHeight(event->height);

    RROutput primary = XRRGetOutputPrimary(m_display, rootWindow());

    qDebug() << primary << " " << m_primary;

    if (primary != m_primary) {
        outputs()[m_primary]->setPrimary(false);
        outputs()[primary]->setPrimary(true);

        m_primary = primary;

        qDebug() << "Priary changed";
    }
}
}