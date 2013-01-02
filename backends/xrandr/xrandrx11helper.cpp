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
    XRRQueryExtension(XRandR::display(), &m_randrBase, &m_randrError);

    KSystemEventFilter::installEventFilter(this);
}

XRandRX11Helper::~XRandRX11Helper()
{
    KSystemEventFilter::removeEventFilter(this);
}

bool XRandRX11Helper::x11Event(XEvent *event)
{
    if (event->xany.type == m_randrBase + RRScreenChangeNotify) {
        qDebug() << "Screen property change detected!";
        Q_EMIT outputsChanged();
    } else if (event->xany.type == m_randrBase + RRNotify) {
        XRRNotifyEvent* e2 = reinterpret_cast< XRRNotifyEvent* >(event);
        if (e2->subtype == RRNotify_OutputChange) { // TODO && e2->window == window )
            qDebug() << "Monitor change detected";
            Q_EMIT outputPropertyChanged();
        }
    }

    return false;
}

#include "xrandrx11helper.moc"
