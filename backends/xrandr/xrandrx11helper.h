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

#ifndef XRANDRX11HELPER_H
#define XRANDRX11HELPER_H

#include <QWidget>
#include "xlibandxrandr.h"

class XRandRX11Helper : public QWidget
{
    Q_OBJECT

    public:
        XRandRX11Helper();
        virtual ~XRandRX11Helper();

    Q_SIGNALS:
        /* Emitted when only XRandR 1.1 or older is available */
        void outputsChanged();

        /* Emitted only when XRandR 1.2 or newer is available */
        void crtcChanged(RRCrtc crtc);
        void outputChanged(RROutput output);
        void outputPropertyChanged(RROutput output);

    private:
        QString rotationToString(Rotation rotation);
        QString connectionToString(Connection connection);

    protected:
        virtual bool x11Event(XEvent *);

        int m_randrBase;
        int m_randrError;
        int m_versionMajor;
        int m_versionMinor;

        bool m_debugMode;
        Window m_window;
};

#endif // XRANDRX11HELPER_H
