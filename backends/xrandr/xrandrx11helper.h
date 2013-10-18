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

        Window m_window;
};

#endif // XRANDRX11HELPER_H
