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

#include <QRect>
#include <QGuiApplication>


using namespace KScreen;

WaylandOutput::WaylandOutput(QObject *parent)
    : KWayland::Client::Output(parent)
    , m_edid(0)
    , m_id(-1)
    , m_completed(false)
{
    connect(this, &KWayland::Client::Output::changed,
            this, &WaylandOutput::update, Qt::QueuedConnection);
    connect(this, &KWayland::Client::Output::modeAdded,
            this, &WaylandOutput::updateModes, Qt::QueuedConnection);
}

WaylandOutput::~WaylandOutput()
{
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
    KScreen::Output *output = new KScreen::Output(parent);
    output->setId(m_id);
    output->setName(QString::number(m_id));
    qCDebug(KSCREEN_WAYLAND) << "toKScreenOutput OUTPUT";
    //updateKScreenOutput(output); // Doesn't seem to be needed, but verify!
    return output;
}

void WaylandOutput::updateKScreenOutput(KScreen::Output* output) const
{
    qCDebug(KSCREEN_WAYLAND) << "updateKScreenOutput OUTPUT";
    // Initialize primary output
    output->setEnabled(true);
    output->setConnected(true);
    output->setPrimary(true); // FIXME
    // FIXME: Rotation

    // Physical size
    output->setSizeMm(physicalSize());
    //qCDebug(KSCREEN_WAYLAND) << "  ####### setSizeMm: " << physicalSize();
    output->setPos(globalPosition());
    ModeList modeList;
    Q_FOREACH (const KWayland::Client::Output::Mode &m, modes()) {
        KScreen::Mode *mode = new KScreen::Mode(output);
        const QString modename = modeName(m);
        mode->setId(modename);
        mode->setRefreshRate(m.refreshRate);
        mode->setSize(m.size);
        mode->setName(modename);
        if (m.flags.testFlag(Output::Mode::Flag::Current)) {
            output->setCurrentModeId(modename);
        }
        modeList[modename] = mode;
    }

    output->setModes(modeList);
}

void WaylandOutput::update()
{
    //qDebug() << "update()";
    flush();
}

QString WaylandOutput::modeName(const KWayland::Client::Output::Mode &m) const
{
    return QString::number(m.size.width()) + QLatin1Char('x') + QString::number(m.size.height()) + QLatin1Char('@') + QString::number((int)(m.refreshRate/1000));
}


void WaylandOutput::updateModes()
{
    //qDebug() << "updateModes()";
    flush();
}

bool WaylandOutput::isComplete()
{
    // FIXME: we want smarter tracking when the whole initialization storm is done and ...
    // the data structures are complete (for now).
    return (m_id != -1 &&
            modes().count() > 0);
}

void WaylandOutput::flush()
{
    if (isComplete() && !m_completed) {
        m_completed = true;

//         qCDebug(KSCREEN_WAYLAND) << "_______________ " << (isValid() ? "Valid" : "Invalid");
//         qCDebug(KSCREEN_WAYLAND) << "Output changes... ";
//         qCDebug(KSCREEN_WAYLAND) << "  id:              " << id();
//         qCDebug(KSCREEN_WAYLAND) << "  Pixel Size:      " << pixelSize();
//         qCDebug(KSCREEN_WAYLAND) << "  Physical Size:   " << physicalSize();
//         qCDebug(KSCREEN_WAYLAND) << "  Global Position: " << globalPosition();
//         qCDebug(KSCREEN_WAYLAND) << "  Manufacturer   : " << manufacturer();
//         qCDebug(KSCREEN_WAYLAND) << "  Model:           " << model();

        foreach (auto m, modes()) {
            QString modename = modeName(m);
            if (m.flags.testFlag(Output::Mode::Flag::Current)) {
                modename = modename + " (current)";
            }
            if (m.flags.testFlag(Output::Mode::Flag::Preferred)) {
                modename = modename + " (Preferred)";
            }
            qCDebug(KSCREEN_WAYLAND) << "            Mode : " << modename;

        }

        emit complete();
    }
}
