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

#include "xrandr.h"
#include "xrandrconfig.h"
#include "xrandrx11helper.h"

#include "config.h"
#include "output.h"
#include "edid.h"
#include "configmonitor.h"

#include <QtCore/QFile>
#include <QtCore/qplugin.h>
#include <QtCore/QRect>
#include <QAbstractEventDispatcher>

#include <QtGui/QX11Info>
#include <QApplication>

#include <kdebug.h>

Q_EXPORT_PLUGIN2(XRandR, XRandR)

Display* XRandR::s_display = 0;
int XRandR::s_screen = 0;
Window XRandR::s_rootWindow = 0;
XRandRConfig* XRandR::s_internalConfig = 0;
int XRandR::s_randrBase = 0;
int XRandR::s_randrError = 0;
bool XRandR::s_monitorInitialized = false;
bool XRandR::s_has_1_3 = false;
bool XRandR::s_xorgCacheInitialized = false;

using namespace KScreen;

XRandR::XRandR(QObject* parent)
    : QObject(parent)
    , m_x11Helper(0)
    , m_isValid(false)
{
    if (s_display == 0) {
        s_display = QX11Info::display();
        s_screen = DefaultScreen(s_display);
        s_rootWindow = XRootWindow(s_display, s_screen);

        XRRQueryExtension(s_display, &s_randrBase, &s_randrError);
    }

    int majorVersion = 0, minorVersion = 0;
    XRRQueryVersion(s_display, &majorVersion, &minorVersion);

    if ((majorVersion > 1) || ((majorVersion == 1) && (minorVersion >= 2))) {
        m_isValid = true;
    } else {
        kDebug() << "XRandR extension not available or unsupported version";
        return;
    }

    XRandR::s_has_1_3 = (majorVersion > 1 || (majorVersion == 1 && minorVersion >= 3));

    if (s_internalConfig == 0) {
        s_internalConfig = new XRandRConfig();
    }

    if (!s_monitorInitialized) {
        m_x11Helper = new XRandRX11Helper();
        /* In case of XRandR 1.0 or 1.1 */
        connect(m_x11Helper, SIGNAL(outputsChanged()), SLOT(updateConfig()));

        /* XRandR >= 1.2 */
        connect(m_x11Helper, SIGNAL(outputChanged(RROutput)), SLOT(updateOutput(RROutput)));
        connect(m_x11Helper, SIGNAL(crtcChanged(RRCrtc)), SLOT(updateCrtc(RRCrtc)));
        s_monitorInitialized = true;
    }
}

XRandR::~XRandR()
{
    delete m_x11Helper;
}

QString XRandR::name() const
{
    return QString("XRandR");
}

void XRandR::updateConfig()
{
    s_internalConfig->update();
    KScreen::ConfigMonitor::instance()->notifyUpdate();
}

void XRandR::updateOutput(RROutput output)
{
    XRandROutput *xOutput = s_internalConfig->outputs().value(output);
    RROutput primary = XRRGetOutputPrimary(XRandR::display(), XRandR::rootWindow());

    xOutput->update((output == primary) ? XRandROutput::SetPrimary : XRandROutput::UnsetPrimary);
    KScreen::ConfigMonitor::instance()->notifyUpdate();
}

void XRandR::updateCrtc(RRCrtc crtc)
{
    XRRCrtcInfo* crtcInfo = XRRCrtc(crtc);
    for (int i = 0; i < crtcInfo->noutput; ++i) {
        XRandROutput *xOutput = s_internalConfig->outputs().value(crtcInfo->outputs[i]);
        xOutput->update();
    }
    XRRFreeCrtcInfo(crtcInfo);

    KScreen::ConfigMonitor::instance()->notifyUpdate();
}

Config* XRandR::config() const
{
    return s_internalConfig->toKScreenConfig();
}

void XRandR::setConfig(Config* config) const
{
    if (!config) {
        return;
    }

    s_internalConfig->applyKScreenConfig(config);
}

Edid *XRandR::edid(int outputId) const
{
    XRandROutput::Map outputs = s_internalConfig->outputs();
    XRandROutput *output = outputs.value(outputId);
    if (!output) {
        return 0;
    }

    return output->edid();
}

bool XRandR::isValid() const
{
    return m_isValid;
}

void XRandR::updateConfig(Config *config) const
{
    Q_ASSERT(config != 0);

    s_internalConfig->updateKScreenConfig(config);
}

quint8* XRandR::getXProperty(Display *dpy, RROutput output, Atom atom, size_t &len)
{
    unsigned char *prop = 0;
    int actual_format;
    unsigned long nitems, bytes_after;
    Atom actual_type;
    quint8 *result;

    XRRGetOutputProperty(dpy, output, atom,
                         0, 100, false, false,
                         AnyPropertyType,
                         &actual_type, &actual_format,
                         &nitems, &bytes_after, &prop);

    if (actual_type == XA_INTEGER && actual_format == 8) {
        result = new quint8[nitems];
        memcpy(result, prop, nitems);
        len = nitems;
    } else {
        result = NULL;
    }

    XFree (prop);
    return result;
}

quint8 *XRandR::outputEdid(int outputId, size_t &len)
{
   Atom edid_atom;
    quint8 *result;

    edid_atom = XInternAtom(QX11Info::display(), RR_PROPERTY_RANDR_EDID, false);
    result = XRandR::getXProperty(QX11Info::display(), outputId, edid_atom, len);
    if (result == NULL) {
        edid_atom = XInternAtom(QX11Info::display(), "EDID_DATA", false);
        result = XRandR::getXProperty(QX11Info::display(), outputId, edid_atom, len);
    }

    if (result == NULL) {
        edid_atom = XInternAtom(QX11Info::display(), "XFree86_DDC_EDID1_RAWDATA", false);
        result = XRandR::getXProperty(QX11Info::display(), outputId, edid_atom, len);
    }

    if (result) {
        if (len % 128 == 0) {
            return result;
        } else {
            len = 0;
            delete result;
        }
    }

    return 0;
}

RRCrtc XRandR::outputCrtc(int outputId)
{
    RRCrtc crtcId;
    XRROutputInfo* outputInfo = XRROutput(outputId);
    kDebug(dXndr()) << "Output" << outputId << "has CRTC" << outputInfo->crtc;

    crtcId = outputInfo->crtc;
    XRRFreeOutputInfo(outputInfo);

    return crtcId;
}

RRCrtc XRandR::freeCrtc(int outputId)
{
    XRROutputInfo* outputInfo = XRROutput(outputId);

    XRRCrtcInfo *crtc;
    for (int i = 0; i < outputInfo->ncrtc; ++i)
    {
        RRCrtc crtcId = outputInfo->crtcs[i];
       crtc = XRRCrtc(crtcId);
       if (!crtc->noutput) {
           kDebug(dXndr()) << "Found free CRTC" << crtcId;
           XRRFreeCrtcInfo(crtc);
           return crtcId;
       }
       XRRFreeCrtcInfo(crtc);
    }

    kDebug(dXndr()) << "No free CRTC found!";
    return 0;
}

XRRScreenResources* XRandR::screenResources()
{
    XRRScreenResources *resources;

    if (XRandR::s_has_1_3) {
        if (XRandR::s_xorgCacheInitialized) {
            resources = XRRGetScreenResourcesCurrent(s_display, s_rootWindow);
        } else {
            /* XRRGetScreenResourcesCurrent is faster then XRRGetScreenResources
             * because it returns cached values. However the cached values are not
             * available until someone calls XRRGetScreenResources first. In case
             * we happen to be the first ones, we need to fill the cache first. */
            resources = XRRGetScreenResources(s_display, s_rootWindow);
            XRandR::s_xorgCacheInitialized = true;
        }
    } else {
        resources = XRRGetScreenResources(s_display, s_rootWindow);
    }

    return resources;
}

XRROutputInfo* XRandR::XRROutput(int outputId)
{
    return XRRGetOutputInfo(s_display, screenResources(), outputId);
}

XRRCrtcInfo* XRandR::XRRCrtc(int crtcId)
{
    return XRRGetCrtcInfo(s_display, screenResources(), crtcId);
}

Display *XRandR::display()
{
    return s_display;
}

Window XRandR::rootWindow()
{
    return s_rootWindow;
}

int XRandR::screen()
{
    return s_screen;
}

extern int dXndr() { static int s_area = KDebug::registerArea("KSRandr", false); return s_area; }

#include "xrandr.moc"
