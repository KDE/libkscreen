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

#include "config.h"
#include "output.h"
#include "edid.h"
#include <configmonitor.h>

#include <QtCore/QDebug>
#include <QtCore/QFile>
#include <QtCore/qplugin.h>
#include <QtCore/QRect>
#include <QAbstractEventDispatcher>

#include <QtGui/QX11Info>
#include <QApplication>

Q_EXPORT_PLUGIN2(XRandR, XRandR)

Display* XRandR::s_display = 0;
int XRandR::s_screen = 0;
Window XRandR::s_rootWindow = 0;
XRandRConfig* XRandR::s_internalConfig = 0;
int XRandR::s_randrBase = 0;
int XRandR::s_randrError = 0;
bool XRandR::s_monitorInitialized = false;

using namespace KScreen;

XRandR::XRandR(QObject* parent)
    : QObject(parent)
{
    if (s_display == 0) {
        s_display = QX11Info::display();
        s_screen = DefaultScreen(s_display);
        s_rootWindow = XRootWindow(s_display, s_screen);

        XRRQueryExtension(s_display, &s_randrBase, &s_randrError);
    }

    if (s_internalConfig == 0) {
        s_internalConfig = new XRandRConfig();
    }

    if (!s_monitorInitialized) {
        initMonitor();
        s_monitorInitialized = true;
    }
}

XRandR::~XRandR()
{

}

QString XRandR::name() const
{
    return QString("XRandR");
}

void XRandR::initMonitor()
{
    /* Use a separate window for getting events so that we don't change Qt's event mask */
    Window window = XCreateSimpleWindow(s_display, DefaultRootWindow(s_display), 0, 0, 1, 1, 0, 0, 0);
    XRRSelectInput(s_display, window, RROutputChangeNotifyMask);

    QAbstractEventDispatcher::instance()->setEventFilter((QAbstractEventDispatcher::EventFilter) handleX11Event);
}

bool XRandR::handleX11Event(void *message)
{
    XEvent *event = (XEvent *) message;

    if (event->xany.type == s_randrBase + RRNotify) {
        XRRNotifyEvent* e2 = reinterpret_cast< XRRNotifyEvent* >(event);
        if (e2->subtype == RRNotify_OutputChange) { // TODO && e2->window == window )
            s_internalConfig->update();
            //KScreen::ConfigMonitor::changeNotify();
            qDebug() << "Monitor change detected";
        }
    }

    return false;
}


Config* XRandR::config() const
{
    return s_internalConfig->toKScreenConfig();
}

void XRandR::setConfig(Config* config) const
{
    s_internalConfig->applyKScreenConfig(config);
}

Edid *XRandR::edid(int outputId) const
{
    XRandROutput::Map outputs = s_internalConfig->outputs();
    XRandROutput *output = outputs.value(outputId);
    if (!output) {
        return 0;
    }

    Edid *edid = output->outputProperty(XRandROutput::PropertyEdid).value<Edid*>();
    if (!edid) {
        size_t len;
        quint8 *data = outputEdid(outputId, len);
        if (data) {
            edid = new Edid(data, len, output);
            output->setOutputProperty(XRandROutput::PropertyEdid, QVariant::fromValue(edid));
            delete data;
        }
    }

    return edid;
}

bool XRandR::isValid() const
{
    return true;
}

quint8* XRandR::getXProperty(Display *dpy, RROutput output, Atom atom, size_t &len)
{
    unsigned char *prop;
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

    crtcId = outputInfo->crtc;
    XRRFreeOutputInfo(outputInfo);

    return crtcId;
}

RRCrtc XRandR::freeCrtc()
{
    XRRScreenResources* resources = screenResources();

    XRRCrtcInfo *crtc;
    for (int i = 0; i < resources->ncrtc; ++i)
    {
       RRCrtc crtcId = resources->crtcs[i];
       crtc = XRRCrtc(crtcId);
       if (!crtc->noutput) {
           qDebug() << "Returning: " << crtcId;
           XRRFreeCrtcInfo(crtc);
           return crtcId;
       }
       XRRFreeCrtcInfo(crtc);
    }

    qDebug() << "Returning: " << "ZERO";
    return 0;
}

XRRScreenResources* XRandR::screenResources()
{
    return XRRGetScreenResources(s_display, s_rootWindow);
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

#include "xrandr.moc"
