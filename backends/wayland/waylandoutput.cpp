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

#include <mode.h>
#include <edid.h>

#include <QtCore/QRect>

#include <QGuiApplication>
#include <QScreen>

using namespace KScreen;

WaylandOutput::WaylandOutput(const QScreen *qscreen, QObject *parent)
    : QObject(parent)
    , m_qscreen(qscreen)
    , m_edid(0)
    , m_id(-1)
{
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

