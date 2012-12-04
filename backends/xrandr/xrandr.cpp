/*************************************************************************************
 *  Copyright (C) 2012 by Alejandro Fiestas Olivares <afiestas@kde.org>              *
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

#include <QtCore/QDebug>
#include <QtCore/QFile>
#include <QtCore/qplugin.h>
#include <QtCore/QRect>

#include <QtGui/QX11Info>

Q_EXPORT_PLUGIN2(XRandR, XRandR)

using namespace KScreen;

XRandR::XRandR(QObject* parent)
    : QObject(parent)
    , m_display(QX11Info::display())
    , m_screen(DefaultScreen(m_display))
    , m_rootWindow(XRootWindow(m_display, m_screen))
    , m_internalConfig(new XRandRConfig(this))
{
    qDebug() << "XRandR Cto";
    qDebug() << "\t" << "Display: " << m_display;
    qDebug() << "\t" << "Screen: " << m_screen;
    qDebug() << "\t" << "RootWindow: " << m_rootWindow;
}

XRandR::~XRandR()
{

}

QString XRandR::name() const
{
    return QString("XRandR");
}


Config* XRandR::config() const
{
    return m_internalConfig->toKScreenConfig();
}

void XRandR::setConfig(Config* config) const
{
    m_internalConfig->applyKScreenConfig(config);
}

Edid *XRandR::edid(int outputId) const
{
    XRandROutput::Map outputs = m_internalConfig->outputs();
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

quint8* XRandR::getXProperty(Display *dpy, RROutput output, Atom atom, size_t &len) const
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

quint8 *XRandR::outputEdid(int outputId, size_t &len) const
{
   Atom edid_atom;
    quint8 *result;

    edid_atom = XInternAtom(QX11Info::display(), RR_PROPERTY_RANDR_EDID, false);
    result = getXProperty(QX11Info::display(), outputId, edid_atom, len);
    if (result == NULL) {
        edid_atom = XInternAtom(QX11Info::display(), "EDID_DATA", false);
        result = getXProperty(QX11Info::display(), outputId, edid_atom, len);
    }

    if (result == NULL) {
        edid_atom = XInternAtom(QX11Info::display(), "XFree86_DDC_EDID1_RAWDATA", false);
        result = getXProperty(QX11Info::display(), outputId, edid_atom, len);
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

RRCrtc XRandR::outputCrtc(int outputId) const
{
    RRCrtc crtcId;
    XRROutputInfo* outputInfo = XRROutput(outputId);

    crtcId = outputInfo->crtc;
    XRRFreeOutputInfo(outputInfo);

    return crtcId;
}

RRCrtc XRandR::freeCrtc() const
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

XRRScreenResources* XRandR::screenResources() const
{
    return XRRGetScreenResources(m_display, m_rootWindow);
}

XRROutputInfo* XRandR::XRROutput(int outputId) const
{
    return XRRGetOutputInfo(m_display, screenResources(), outputId);
}

XRRCrtcInfo* XRandR::XRRCrtc(int crtcId) const
{
    return XRRGetCrtcInfo(m_display, screenResources(), crtcId);
}

Display *XRandR::display() const
{
    return m_display;
}

Window XRandR::rootWindow() const
{
    return m_rootWindow;
}

int XRandR::screen() const
{
    return m_screen;
}

#include "xrandr.moc"
