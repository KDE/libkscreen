/*************************************************************************************
 *  Copyright 2014 Sebastian Kügler <sebas@kde.org>                                  *
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

static void outputHandleGeometry(void *data, wl_output *output, int32_t x, int32_t y,
                                 int32_t physicalWidth, int32_t physicalHeight, int32_t subPixel,
                                 const char *make, const char *model, int32_t transform)
{
    Q_UNUSED(subPixel)
    Q_UNUSED(transform)
    qCDebug(KSCREEN_WAYLAND) << "wl_output::outputHandleGeometry: " << output << make << " model : " << model;
//     Output *o = reinterpret_cast<Output*>(data);
//     if (o->output() != output) {
//         return;
//     }
//     o->setGlobalPosition(QPoint(x, y));
//     o->setManufacturer(make);
//     o->setModel(model);
//     o->setPhysicalSize(QSize(physicalWidth, physicalHeight));
//     o->emitChanged();
}

static void outputHandleMode(void *data, wl_output *output, uint32_t flags, int32_t width, int32_t height, int32_t refresh)
{
    Q_UNUSED(flags)
    qCDebug(KSCREEN_WAYLAND) << "wl_output::outputHandleMode: " << output << width << height << refresh;
//     Output *o = reinterpret_cast<Output*>(data);
//     if (o->output() != output) {
//         return;
//     }
//     o->setPixelSize(QSize(width, height));
//     o->setRefreshRate(refresh);
//     o->emitChanged();
}

static void outputHandleDone(void *data, wl_output *output)
{
    qCDebug(KSCREEN_WAYLAND) << "wl_output::outputHandleDone: " << output;
    Q_UNUSED(data)
    Q_UNUSED(output)
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



using namespace KScreen;

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
    wl_output_add_listener(m_output, &s_outputListener, this);
    //wl_display_dispatch(_display);
    qCDebug(KSCREEN_WAYLAND) << "Listening ...";

}

WaylandOutput::~WaylandOutput()
{
}

int WaylandOutput::id() const
{
    return m_id;
}

void WaylandOutput::setId(const int newId)
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

const QScreen* WaylandOutput::qscreen() const
{
    return m_qscreen;
}

Output* WaylandOutput::toKScreenOutput(Config* parent) const
{
    Output *output = new Output(parent);
//     output->setId(m_id);
//     output->setName(m_qscreen->name());
//     updateKScreenOutput(output);
    return output;
}

void WaylandOutput::updateKScreenOutput(Output* output) const
{
//     // Initialize primary output
//     output->setEnabled(true);
//     output->setConnected(true);
//     output->setPrimary(QGuiApplication::primaryScreen() == m_qscreen);
//     qCDebug(KSCREEN_WAYLAND) << " OUTPUT Primary? " <<  (QGuiApplication::primaryScreen() == m_qscreen);
//     // FIXME: Rotation
// 
//     // Physical size
//     QSize mm;
//     qreal physicalWidth;
//     physicalWidth = m_qscreen->size().width() / (m_qscreen->physicalDotsPerInchX() / 25.4);
//     mm.setWidth(qRound(physicalWidth));
//     qreal physicalHeight;
//     physicalHeight = m_qscreen->size().height() / (m_qscreen->physicalDotsPerInchY() / 25.4);
//     mm.setHeight(qRound(physicalHeight));
//     output->setSizeMm(mm);
// //     qCDebug(KSCREEN_WAYLAND) << "  ####### setSizeMm: " << mm;
// //     qCDebug(KSCREEN_WAYLAND) << "  ####### availableGeometry: " << m_qscreen->availableGeometry();
//     output->setPos(m_qscreen->availableGeometry().topLeft());
// 
//     // Modes: we create a single default mode and go with that
//     Mode *mode = new Mode(output);
//     const QString modeid = QStringLiteral("defaultmode");
//     mode->setId(modeid);
//     mode->setRefreshRate(m_qscreen->refreshRate());
//     mode->setSize(m_qscreen->size());
// 
// 
//     const QString modename = QString::number(m_qscreen->size().width()) + QStringLiteral("x") + QString::number(m_qscreen->size().height()) + QStringLiteral("@") + QString::number(m_qscreen->refreshRate());
//     mode->setName(modename);
// 
//     ModeList modes;
//     modes[modeid] = mode;
//     output->setModes(modes);
//     output->setCurrentModeId(modeid);
}

void WaylandOutput::setGlobalPosition(const QPoint &pos)
{
    m_globalPosition = pos;
}

void WaylandOutput::setManufacturer(const QString &manufacturer)
{
    m_manufacturer = manufacturer;
}

void WaylandOutput::setModel(const QString &model)
{
    m_model = model;
}

void WaylandOutput::setPhysicalSize(const QSize &size)
{
    m_physicalSize = size;
}

void WaylandOutput::setPixelSize(const QSize& size)
{
    m_pixelSize = size;
}

void WaylandOutput::setRefreshRate(int refreshRate)
{
    m_refreshRate = refreshRate;
}
