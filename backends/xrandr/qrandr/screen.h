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

#ifndef QSCREEN_H
#define QSCREEN_H

#include "xlibandxrandr.h"

#include <QtCore/QObject>
#include <QtCore/QSize>
#include <QtCore/QHash>

namespace QRandR {

class Mode;
class Crtc;
class Output;

class Screen : public QObject
{
    friend class Crtc;
    friend class Output;

    Q_OBJECT

    public:
        Screen (int screenId, Display* display);
        virtual ~Screen();

        int id();
        const QSize minSize();
        const QSize maxSize();
        const QSize currentSize();

        QHash<RROutput, Crtc *> crtc();
        QHash<RROutput, Output *> outputs();
        Output* output(RROutput id);
        Output* primaryOutput();
        QHash<RRMode, Mode *> modes();

        Mode* mode(RRMode id);
        Crtc* crtc(RRCrtc id);

        void setPrimaryOutput(Output *output);

        void handleEvent(XRRScreenChangeNotifyEvent *event);
        Window rootWindow();

        XRRScreenResources* resources();

    private:
        void getMinAndMaxSize();

    private:
        int m_id;
        Display *m_display;
        Window m_rootWindow;
        RROutput m_primary;
        QSize m_minSize;
        QSize m_maxSize;
        QSize m_currentSize;

        XRRScreenResources *m_resources;
        QHash<RROutput, Crtc *> m_crtc;
        QHash<RROutput, Output *> m_outputs;
        QHash<RRMode, Mode *> m_modes;
};

#endif //QSCREEN_H

}