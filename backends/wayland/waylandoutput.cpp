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
    : QObject(parent)
    , m_edid(new Edid(QByteArray(), this))
    , m_output(nullptr)
    , m_protocolName(0)
    , m_protocolVersion(0)
{
    /*
    enum class Transform {
        Normal,
        Rotated90,
        Rotated180,
        Rotated270,
        Flipped,
        Flipped90,
        Flipped180,
        Flipped270
    };

    enum Rotation {
        None = 1,
        Left = 2,
        Inverted = 4,
        Right = 8
    };
    */

    m_rotationMap[KWayland::Client::OutputDevice::Transform::Normal] = KScreen::Output::None;
    m_rotationMap[KWayland::Client::OutputDevice::Transform::Rotated90] = KScreen::Output::Right;
    m_rotationMap[KWayland::Client::OutputDevice::Transform::Rotated180] = KScreen::Output::Inverted;
    m_rotationMap[KWayland::Client::OutputDevice::Transform::Rotated270] = KScreen::Output::Left;
    m_rotationMap[KWayland::Client::OutputDevice::Transform::Flipped] = KScreen::Output::None;
    m_rotationMap[KWayland::Client::OutputDevice::Transform::Flipped90] = KScreen::Output::Right;
    m_rotationMap[KWayland::Client::OutputDevice::Transform::Flipped180] = KScreen::Output::Inverted;
    m_rotationMap[KWayland::Client::OutputDevice::Transform::Flipped270] = KScreen::Output::Left;
}

WaylandOutput::~WaylandOutput()
{
    qDebug() << "bye bye output";
}

quint32 WaylandOutput::id() const
{
    Q_ASSERT(m_output);
    return m_id;
}

bool WaylandOutput::enabled() const
{
    return m_output != nullptr;
}

KWayland::Client::OutputDevice* WaylandOutput::output() const
{
    return m_output;
}

void WaylandOutput::setOutput(KWayland::Client::Registry* registry, KWayland::Client::OutputDevice* op, quint32 name, quint32 version)
{
    //qDebug() << "WL setOUtput" << registry << op << name;
    if (m_output == op) {
        return;
    }
    m_output = op;

    connect(m_output, &KWayland::Client::OutputDevice::done,
            this, &WaylandOutput::complete, Qt::QueuedConnection);

    m_output->setup(registry->bindOutputDevice(name, version));
    //qDebug() << "WL Client::OutputDevice bound";
    emit changed();
}


/*
KScreen::Edid WaylandOutput::edid()
{
    if (!m_edid) {
        m_edid(new KScreen::Edid(QByteArray(), this));
    }
    return m_edid;
    //return KScreen::Edid(QByteArray(), this);
    return 0;
}
*/

KScreen::OutputPtr WaylandOutput::toKScreenOutput(KScreen::ConfigPtr &parent) const
{
    KScreen::OutputPtr output(new KScreen::Output());
    return output;
}

void WaylandOutput::updateKScreenOutput(KScreen::OutputPtr &output) const
{
    qCDebug(KSCREEN_WAYLAND) << "updateKScreenOutput OUTPUT";
    // Initialize primary output
    const QString id_string = m_output->manufacturer() + QStringLiteral("-") + m_output->model();
    int id = id_string.toUtf8().toInt();
    output->setId(id);
    output->setEnabled(m_output->enabled() == KWayland::Client::OutputDevice::Enablement::Enabled);
    output->setConnected(true);
    output->setPrimary(true); // FIXME

    output->setName(m_output->manufacturer() + QStringLiteral("-") + m_output->model());
    // Physical size
    output->setSizeMm(m_output->physicalSize());
    //qCDebug(KSCREEN_WAYLAND) << "  ####### setSizeMm: " << physicalSize() << geometry();
    output->setPos(m_output->globalPosition());
    output->setRotation(m_rotationMap[m_output->transform()]);
    KScreen::ModeList modeList;
    Q_FOREACH (const KWayland::Client::OutputDevice::Mode &m, m_output->modes()) {
        KScreen::ModePtr mode(new KScreen::Mode());
        const QString modename = modeName(m);
        mode->setId(modename);
        mode->setRefreshRate(m.refreshRate);
        mode->setSize(m.size);
        mode->setName(modename);
        if (m.flags.testFlag(KWayland::Client::OutputDevice::Mode::Flag::Current)) {
            output->setCurrentModeId(modename);
        }
        modeList[modename] = mode;
    }

    output->setModes(modeList);
}

QString WaylandOutput::modeName(const KWayland::Client::OutputDevice::Mode &m) const
{
    return QString::number(m.size.width()) + QLatin1Char('x') + QString::number(m.size.height()) + QLatin1Char('@') + QString::number((int)(m.refreshRate/1000));
}

void WaylandOutput::showOutput()
{
    if (true) {
        qCDebug(KSCREEN_WAYLAND) << "_______________ " << (m_output->isValid() ? "Valid" : "Invalid");
        qCDebug(KSCREEN_WAYLAND) << "Output changes... ";
        qCDebug(KSCREEN_WAYLAND) << "  id:              " << id();
        qCDebug(KSCREEN_WAYLAND) << "  Pixel Size:      " << m_output->pixelSize();
        qCDebug(KSCREEN_WAYLAND) << "  Physical Size:   " << m_output->physicalSize();
        qCDebug(KSCREEN_WAYLAND) << "  Global Position: " << m_output->globalPosition();
        qCDebug(KSCREEN_WAYLAND) << "  Manufacturer   : " << m_output->manufacturer();
        qCDebug(KSCREEN_WAYLAND) << "  Model:           " << m_output->model();
    }
    foreach (auto m, m_output->modes()) {
        QString modename = modeName(m);
        if (m.flags.testFlag(KWayland::Client::OutputDevice::Mode::Flag::Current)) {
            modename = modename + " (current)";
        }
        if (m.flags.testFlag(KWayland::Client::OutputDevice::Mode::Flag::Preferred)) {
            modename = modename + " (Preferred)";
        }
        //qCDebug(KSCREEN_WAYLAND) << "            Mode : " << modename;

    }
}
