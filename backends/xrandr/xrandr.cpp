/*************************************************************************************
 *  Copyright (C) 2012 by Alejandro Fiestas Olivares <afiestas@kde.org>              *
 *  Copyright (C) 2012, 2013 by Daniel Vrátil <dvratil@redhat.com>                   *
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

#include "xrandr.h"
#include "xrandrconfig.h"
#include "xrandrxcbhelper.h"
#include "xrandrscreen.h"

#include "config.h"
#include "output.h"
#include "edid.h"

#include <QtCore/QFile>
#include <QtCore/qplugin.h>
#include <QtCore/QRect>
#include <QAbstractEventDispatcher>
#include <QTimer>
#include <QTime>

#include <QX11Info>
#include <QGuiApplication>

#include <xcb/randr.h>

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

Q_LOGGING_CATEGORY(KSCREEN_XRANDR, "kscreen.xrandr");

XRandR::XRandR()
    : KScreen::AbstractBackend()
    , m_x11Helper(0)
    , m_isValid(false)
    , m_configChangeCompressor(0)
{
    QLoggingCategory::setFilterRules(QLatin1Literal("kscreen.xrandr.debug = true"));

    qRegisterMetaType<RROutput>("RROutput");
    qRegisterMetaType<RRCrtc>("RRCrtc");

    // Use our own connection to make sure that we won't mess up Qt's connection
    // if something goes wrong on our side.
    xcb_generic_error_t *error = 0;
    xcb_randr_query_version_reply_t* version;
    xcb_connection_t *connection = xcb_connect(0, 0);
    version = xcb_randr_query_version_reply(connection, xcb_randr_query_version(connection, XCB_RANDR_MAJOR_VERSION, XCB_RANDR_MINOR_VERSION), &error);
    xcb_disconnect(connection);

    if (!version || error) {
        free(error);
        return;
    }

    if ((version->major_version > 1) || ((version->major_version == 1) && (version->minor_version >= 2))) {
        m_isValid = true;
    } else {
        qCWarning(KSCREEN_XRANDR) << "XRandR extension not available or unsupported version";
        return;
    }

    if (s_display == 0) {
        s_display = QX11Info::display();
        s_screen = DefaultScreen(s_display);
        s_rootWindow = XRootWindow(s_display, s_screen);

        XRRQueryExtension(s_display, &s_randrBase, &s_randrError);
    }

    XRandR::s_has_1_3 = (version->major_version > 1 || (version->major_version == 1 && version->minor_version >= 3));

    if (s_internalConfig == 0) {
        s_internalConfig = new XRandRConfig();
    }

    if (!s_monitorInitialized) {
        m_x11Helper = new XRandRXCBHelper();
        connect(m_x11Helper, &XRandRXCBHelper::outputChanged,
                this, &XRandR::outputChanged,
                Qt::QueuedConnection);
        connect(m_x11Helper, &XRandRXCBHelper::crtcChanged,
                this, &XRandR::crtcChanged,
                Qt::QueuedConnection);
        connect(m_x11Helper, &XRandRXCBHelper::screenChanged,
                this, &XRandR::screenChanged,
                Qt::QueuedConnection);

        m_configChangeCompressor = new QTimer(this);
        m_configChangeCompressor->setSingleShot(true);
        m_configChangeCompressor->setInterval(500);
        connect(m_configChangeCompressor, &QTimer::timeout,
                [&]() {
                    qCDebug(KSCREEN_XRANDR) << "Emitting configChanged()";
                    Q_EMIT configChanged(config());
                });

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

QString XRandR::serviceName() const
{
    return QLatin1Literal("org.kde.KScreen.Backend.XRandR");
}


void XRandR::outputChanged(RROutput output, RRCrtc crtc, RRMode mode, Connection connection)
{
    XRandROutput *xOutput = s_internalConfig->output(output);
    const RROutput primary = XRRGetOutputPrimary(XRandR::display(), XRandR::rootWindow());
    if (!xOutput) {
        s_internalConfig->addNewOutput(output);
    } else {
        xOutput->update(crtc, mode, connection, (primary == output));
        qCDebug(KSCREEN_XRANDR) << "Output" << xOutput->id() << ": connected =" << xOutput->isConnected() << ", enabled =" << xOutput->isEnabled();
    }

    m_configChangeCompressor->start();
}

void XRandR::crtcChanged(RRCrtc crtc, RRMode mode, Rotation rotation, const QRect& geom)
{
    XRandRCrtc *xCrtc = s_internalConfig->crtc(crtc);
    if (!xCrtc) {
        s_internalConfig->addNewCrtc(crtc);
    } else {
        xCrtc->update(mode, rotation, geom);
    }

    m_configChangeCompressor->start();
}

void XRandR::screenChanged(Rotation rotation, const QSize &sizePx, const QSize &sizeMm)
{
    Q_UNUSED(rotation);
    Q_UNUSED(sizeMm);

    XRandRScreen *xScreen = s_internalConfig->screen();
    Q_ASSERT(xScreen);
    xScreen->update(sizePx);

    m_configChangeCompressor->start();
}


ConfigPtr XRandR::config() const
{
    return s_internalConfig->toKScreenConfig();
}

void XRandR::setConfig(const ConfigPtr &config)
{
    if (!config) {
        return;
    }

    qCDebug(KSCREEN_XRANDR) << "XRandR::setConfig";
    s_internalConfig->applyKScreenConfig(config);
    qCDebug(KSCREEN_XRANDR) << "XRandR::setConfig done!";
}

QByteArray XRandR::edid(int outputId) const
{
    const XRandROutput *output = s_internalConfig->output(outputId);
    if (!output) {
        return QByteArray();
    }

    return output->edid();
}

bool XRandR::isValid() const
{
    return m_isValid;
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
            delete[] result;
        }
    }

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
    XRRScreenResources *resources = screenResources();
    XRROutputInfo *info = XRRGetOutputInfo(s_display, resources, outputId);
    XRRFreeScreenResources(resources);

    return info;
}

XRRCrtcInfo* XRandR::XRRCrtc(int crtcId)
{
    XRRScreenResources *resources = screenResources();
    XRRCrtcInfo *info = XRRGetCrtcInfo(s_display, resources, crtcId);
    XRRFreeScreenResources(resources);

    return info;
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
