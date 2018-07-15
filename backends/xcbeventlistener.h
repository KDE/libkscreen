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

#include <QObject>
#include <QLoggingCategory>
#include <QAbstractNativeEventFilter>
#include <QRect>

#include "xcbwrapper.h"

class XCBEventListener : public QObject,
                         public QAbstractNativeEventFilter
{
    Q_OBJECT

    public:
        XCBEventListener();
        ~XCBEventListener() override;

        bool nativeEventFilter(const QByteArray& eventType, void* message, long int* result) override;

    Q_SIGNALS:
        /* Emitted when only XRandR 1.1 or older is available */
        void screenChanged(xcb_randr_rotation_t rotation,
                           const QSize &sizePx,
                           const QSize &sizeMm);
        void outputsChanged();

        /* Emitted only when XRandR 1.2 or newer is available */
        void crtcChanged(xcb_randr_crtc_t crtc,
                         xcb_randr_mode_t mode,
                         xcb_randr_rotation_t rotation,
                         const QRect &geom);
        void outputChanged(xcb_randr_output_t output,
                           xcb_randr_crtc_t crtc,
                           xcb_randr_mode_t mode,
                           xcb_randr_connection_t connection);
        void outputPropertyChanged(xcb_randr_output_t output);

    private:
        QString rotationToString(xcb_randr_rotation_t rotation);
        QString connectionToString(xcb_randr_connection_t connection);
        void handleScreenChange(xcb_generic_event_t *e);
        void handleXRandRNotify(xcb_generic_event_t *e);

    protected:
        bool m_isRandrPresent;
        bool m_event11;
        uint8_t m_randrBase;
        uint8_t m_randrErrorBase;
        uint8_t m_majorOpcode;
        uint32_t m_versionMajor;
        uint32_t m_versionMinor;

        uint32_t m_window;
};

Q_DECLARE_LOGGING_CATEGORY(KSCREEN_XCB_HELPER)
#endif // XRANDRX11HELPER_H
