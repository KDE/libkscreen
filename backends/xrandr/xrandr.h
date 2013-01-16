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

#ifndef XRANDR_BACKEND_H
#define XRANDR_BACKEND_H

#include "xlibandxrandr.h"
#include "../abstractbackend.h"

#include <QtCore/QSize>

class XRandRX11Helper;
class XRandRConfig;
namespace KScreen {
    class Output;
}

class XRandR : public QObject, public AbstractBackend
{
    Q_OBJECT
    Q_INTERFACES(AbstractBackend)

    public:
        explicit XRandR(QObject* parent = 0);
        virtual ~XRandR();

        virtual QString name() const;
        virtual KScreen::Config* config() const;
        virtual void setConfig(KScreen::Config* config) const;
        virtual bool isValid() const;
        virtual KScreen::Edid *edid(int outputId) const;
        virtual void updateConfig(KScreen::Config *config) const;

        static RRCrtc outputCrtc(int outputId);
        static quint8 *outputEdid(int outputId, size_t &len);
        static RRCrtc freeCrtc(int outputId);
        static XRRScreenResources* screenResources();
        static XRROutputInfo* XRROutput(int outputId);
        static XRRCrtcInfo* XRRCrtc(int crtcId);
        static Display* display();
        static int screen();
        static Window rootWindow();

    private Q_SLOTS:
        void updateConfig();

        void updateOutput(RROutput output);
        void updateCrtc(RRCrtc crtc);

    private:
        static quint8* getXProperty(Display *dpy, RROutput output, Atom atom, size_t &len);

        static Display* s_display;
        static int s_screen;
        static Window s_rootWindow;
        static XRandRConfig *s_internalConfig;
        static int s_randrBase;
        static int s_randrError;
        static bool s_monitorInitialized;
        static bool s_has_1_3;
        static bool s_xorgCacheInitialized;

        XRandRX11Helper *m_x11Helper;
        bool m_isValid;
};

extern int dXndr();
#endif //XRandR_BACKEND_H
