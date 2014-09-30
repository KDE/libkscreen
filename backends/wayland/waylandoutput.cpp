/*************************************************************************************
 *  Copyright 2014 Sebastian Kügler <sebas@kde.org>                                  *
 *  Copyright 2013 Martin Gräßlin <mgraesslin@kde.org>                               *
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

#include "waylandoutput.h"
#include "waylandbackend.h"
#include "waylandconfig.h"

#include <mode.h>
#include <edid.h>

#include <QtCore/QRect>

#include <QGuiApplication>
#include <QScreen>

using namespace KScreen;

/*
static void outputHandleGeometry(void *data, wl_output *output, int32_t x, int32_t y,
                                 int32_t physicalWidth, int32_t physicalHeight, int32_t subPixel,
                                 const char *make, const char *model, int32_t transform)
{
    Q_UNUSED(subPixel)
    Q_UNUSED(transform)
    qCDebug(KSCREEN_WAYLAND) << "wl_output::outputHandleGeometry: " << output << make << " model : " << model;
    qCDebug(KSCREEN_WAYLAND) << "                               : " << physicalWidth << physicalHeight << " subpixel / transform: " << subPixel << transform << " x y: " << x << y;

    WaylandOutput *o = reinterpret_cast<WaylandOutput*>(data);
    if (o->output() != output) {
        qCDebug(KSCREEN_WAYLAND) << "sender (data) is not a WaylandOutput";
        return;
    }
    qCDebug(KSCREEN_WAYLAND) << "sender (data) to WaylandOutput cast went OK";

//     Output *o = reinterpret_cast<Output*>(data);
//     if (o->output() != output) {
//         return;
//     }
    o->setGlobalPosition(QPoint(x, y));
    o->setManufacturer(make);
    o->setModel(model);
    o->setPhysicalSize(QSize(physicalWidth, physicalHeight));
//     o->emitChanged();
}

static void outputHandleMode(void *data, wl_output *output, uint32_t flags, int32_t width, int32_t height, int32_t refresh)
{
    bool current = flags & WL_OUTPUT_MODE_CURRENT;
    if (!(flags & WL_OUTPUT_MODE_CURRENT)) {
        // ignore all non-current modes;
        return;
    }


    Q_UNUSED(flags)
    qCDebug(KSCREEN_WAYLAND) << "wl_output::outputHandleMode: " << output << width << height << refresh;
    WaylandOutput *o = reinterpret_cast<WaylandOutput*>(data);
    if (o->output() != output) {
        qCDebug(KSCREEN_WAYLAND) << "sender (data) is not a WaylandOutput";
        return;
    }
    qCDebug(KSCREEN_WAYLAND) << "sender (data) to WaylandOutput cast went OK";
    qCDebug(KSCREEN_WAYLAND) << "current Mode?" << true;


    o->addMode(width, height, refresh, current);

    if (current) {
        o->setPixelSize(QSize(width, height));
        o->setRefreshRate(refresh);
    }
    o->flush();
}

static void outputHandleDone(void *data, wl_output *output)
{
    qCDebug(KSCREEN_WAYLAND) << "wl_output::outputHandleDone: " << output;
    Q_UNUSED(data)
    Q_UNUSED(output)

    WaylandOutput *o = reinterpret_cast<WaylandOutput*>(data);
    if (o->output() != output) {
        qCDebug(KSCREEN_WAYLAND) << "sender (data) is not a WaylandOutput";
        return;
    }
    o->flush();
}

static void outputHandleScale(void *data, wl_output *output, int32_t scale)
{
    qCDebug(KSCREEN_WAYLAND) << "wl_output::outputHandleScale: " << scale;
    Q_UNUSED(data)
    Q_UNUSED(output)
    Q_UNUSED(scale)
}

static const struct wl_output_listener s_outputListener = {
    outputHandleGeometry,
    outputHandleMode,
    outputHandleDone,
    outputHandleScale
};
*/

WaylandOutput::WaylandOutput(wl_output *wloutput, QObject *parent)
    : QObject(parent)
    , m_output(wloutput)
    , m_edid(0)
    , m_id(-1)
{
    qCDebug(KSCREEN_WAYLAND) << "wl_output_add_listener";

    // static_cast<wl_output*>(wl_display_bind(display_->display(), id, &wl_output_interface));

//    wl_display *_display = WaylandBackend::internalConfig()->display();
//     wl_output *_output = static_cast<wl_output*>(wl_display_bind(_display, 1, &wl_output_interface));

    //wl_output *o = reinterpret_cast<wl_output *>(wl_registry_bind(registry, name, &wl_output_interface, 1))
//    wl_output_add_listener(m_output, &s_outputListener, this);
    //wl_display_dispatch(_display);
    qCDebug(KSCREEN_WAYLAND) << "Listening ...";

}

WaylandOutput::~WaylandOutput()
{
    if (m_output) {
        wl_output_destroy(m_output);
    }
}

wl_output* WaylandOutput::output() const
{
    return m_output;
}


quint32 WaylandOutput::id() const
{
    return m_id;
}

void WaylandOutput::setId(const quint32 newId)
{
    m_id = newId;
}



KScreen::Edid *WaylandOutput::edid()
{
    if (!m_edid) {
        m_edid = new KScreen::Edid(0, 0, this);
    }
    return m_edid;
}

Output* WaylandOutput::toKScreenOutput(Config* parent) const
{
    Output *output = new Output(parent);
    output->setId(m_id);
    output->setName(QString::number(m_id));
    updateKScreenOutput(output);
    return output;
}

void WaylandOutput::updateKScreenOutput(Output* output) const
{
    qCDebug(KSCREEN_WAYLAND) << "updateKScreenOutput OUTPUT";
    // Initialize primary output
    output->setEnabled(true);
    output->setConnected(true);
    output->setPrimary(true);
    // FIXME: Rotation

    // Physical size
    output->setSizeMm(physicalSize());
    qCDebug(KSCREEN_WAYLAND) << "  ####### setSizeMm: " << physicalSize();
    output->setPos(globalPosition());

    ModeList modes;
    Q_FOREACH (const QString &modename, m_modes.keys()) {
        const WaylandMode &wlmode = m_modes[modename];
        qCDebug(KSCREEN_WAYLAND) << "Creating mode: " << modename << m_modes[modename] << wlmode;
        Mode *mode = new Mode(output);
        mode->setId(modename);
        mode->setRefreshRate(wlmode.at(2));
        mode->setSize(QSize(wlmode.at(0), wlmode.at(1)));
        mode->setName(modename);
        if (wlmode.at(3)) {
            output->setCurrentModeId(modename);
        }
        modes[modename] = mode;
    }

    output->setModes(modes);
}

void WaylandOutput::flush()
{
    qCDebug(KSCREEN_WAYLAND) << "_______________ ";
    qCDebug(KSCREEN_WAYLAND) << "Output changes... ";
    qCDebug(KSCREEN_WAYLAND) << "  id:              " << id();
//     qCDebug(KSCREEN_WAYLAND) << "  name:            " << name();
    qCDebug(KSCREEN_WAYLAND) << "  Pixel Size:      " << pixelSize();
    qCDebug(KSCREEN_WAYLAND) << "  Physical Size:   " << physicalSize();
    qCDebug(KSCREEN_WAYLAND) << "  Global Position: " << globalPosition();
    qCDebug(KSCREEN_WAYLAND) << "  Manufacturer   : " << manufacturer();
    qCDebug(KSCREEN_WAYLAND) << "  Model:           " << model();
    // TODO
    emit complete();
}

const QPoint& WaylandOutput::globalPosition() const
{
    return m_globalPosition;
}

void WaylandOutput::setGlobalPosition(const QPoint &pos)
{
    m_globalPosition = pos;
}

const QString& WaylandOutput::manufacturer() const
{
    return m_manufacturer;
}

void WaylandOutput::setManufacturer(const QString &manufacturer)
{
    m_manufacturer = manufacturer;
}

const QString& WaylandOutput::model() const
{
    return m_model;
}

void WaylandOutput::setModel(const QString &model)
{
    m_model = model;
}

const QSize& WaylandOutput::physicalSize() const
{
    return m_physicalSize;
}

void WaylandOutput::setPhysicalSize(const QSize &size)
{
    m_physicalSize = size;
}

const QSize& WaylandOutput::pixelSize() const
{
    return m_pixelSize;
}

void WaylandOutput::setPixelSize(const QSize& size)
{
    m_pixelSize = size;
}

int WaylandOutput::refreshRate() const
{
    return m_refreshRate;
}

void WaylandOutput::setRefreshRate(int refreshRate)
{
    m_refreshRate = refreshRate;
}

void WaylandOutput::addMode(quint32 w, quint32 h, quint32 refresh, bool current)
{
    const QString modename = QString::number(w) + QStringLiteral("x") + QString::number(h) + QStringLiteral("@") + QString::number(refresh);

    QList<quint32> mode;
    if (m_modes.contains(modename)) {
        mode = m_modes[modename];
    } else {
        mode << w << h << refresh << current;
        m_modes[modename] = mode;
    }
}

