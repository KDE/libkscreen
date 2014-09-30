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

#include <KWayland/Client/output.h>

#include <QtCore/QRect>

#include <QGuiApplication>
#include <QScreen>

using namespace KScreen;

WaylandOutput::WaylandOutput(KWayland::Client::Output *wloutput, QObject *parent)
    : QObject(parent)
    , m_output(wloutput)
    , m_edid(0)
    , m_id(-1)
{
    qCDebug(KSCREEN_WAYLAND) << "KWayland::Client::Output_add_listener";

    connect(m_output, &KWayland::Client::Output::changed, this, &WaylandOutput::update, Qt::QueuedConnection);
    qCDebug(KSCREEN_WAYLAND) << "Listening ...";

}

WaylandOutput::~WaylandOutput()
{
    if (m_output) {
        //wl_output_destroy(m_output);
    }
}

KWayland::Client::Output* WaylandOutput::output() const
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

void WaylandOutput::update()
{
    setGlobalPosition(m_output->globalPosition());
    setManufacturer(m_output->manufacturer());
    setModel(m_output->model());
    setPhysicalSize(m_output->physicalSize());
    flush();
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

