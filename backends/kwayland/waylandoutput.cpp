/*************************************************************************************
 *  Copyright 2014-2015 Sebastian Kügler <sebas@kde.org>                             *
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
#include "../utils.h"

#include <mode.h>
#include <edid.h>

using namespace KScreen;
namespace Wl = KWayland::Client;

const QMap<Wl::OutputDevice::Transform, Output::Rotation>
s_rotationMap = {
    {Wl::OutputDevice::Transform::Normal, Output::None},
    {Wl::OutputDevice::Transform::Rotated90, Output::Right},
    {Wl::OutputDevice::Transform::Rotated180, Output::Inverted},
    {Wl::OutputDevice::Transform::Rotated270, Output::Left},
    {Wl::OutputDevice::Transform::Flipped, Output::None},
    {Wl::OutputDevice::Transform::Flipped90, Output::Right},
    {Wl::OutputDevice::Transform::Flipped180, Output::Inverted},
    {Wl::OutputDevice::Transform::Flipped270, Output::Left}
};

WaylandOutput::WaylandOutput(quint32 id, WaylandConfig *parent)
    : QObject(parent)
    , m_id(id)
    , m_output(nullptr)
{
}

Output::Rotation
WaylandOutput::toKScreenRotation(const Wl::OutputDevice::Transform transform) const
{
    auto it = s_rotationMap.constFind(transform);
    return it.value();
}

Wl::OutputDevice::Transform
WaylandOutput::toKWaylandTransform(const Output::Rotation rotation) const
{
    return s_rotationMap.key(rotation);
}

QString WaylandOutput::toKScreenModeId(int kwaylandmodeid) const
{
    auto it = std::find(m_modeIdMap.constBegin(), m_modeIdMap.constEnd(), kwaylandmodeid);
    if (it == m_modeIdMap.constEnd()) {
        qCWarning(KSCREEN_WAYLAND) << "Invalid kwayland mode id:" << kwaylandmodeid << m_modeIdMap;
        return QStringLiteral("invalid_mode_id");
    }
    return it.key();
}

int WaylandOutput::toKWaylandModeId(const QString &kscreenmodeid) const
{
    if (!m_modeIdMap.contains(kscreenmodeid)) {
        qCWarning(KSCREEN_WAYLAND) << "Invalid kscreen mode id:" << kscreenmodeid << m_modeIdMap;
    }
    return m_modeIdMap.value(kscreenmodeid, -1);
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

Wl::OutputDevice* WaylandOutput::outputDevice() const
{
    return m_output;
}

void WaylandOutput::createOutputDevice(Wl::Registry *registry, quint32 name, quint32 version)
{
    Q_ASSERT(!m_output);
    m_output = registry->createOutputDevice(name, version);

    connect(m_output, &Wl::OutputDevice::removed, this, &WaylandOutput::deviceRemoved);
    connect(m_output, &Wl::OutputDevice::done, this, [this]() {
                Q_EMIT complete();
                connect(m_output, &Wl::OutputDevice::changed, this, &WaylandOutput::changed);
    });
}

OutputPtr WaylandOutput::toKScreenOutput()
{
    OutputPtr output(new Output());
    output->setId(m_id);
    updateKScreenOutput(output);
    return output;
}

void WaylandOutput::updateKScreenOutput(OutputPtr &output)
{
    // Initialize primary output
    output->setId(m_id);
    output->setEnabled(m_output->enabled() == Wl::OutputDevice::Enablement::Enabled);
    output->setConnected(true);
    output->setPrimary(true); // FIXME: wayland doesn't have the concept of a primary display
    output->setName(name());
    output->setSizeMm(m_output->physicalSize());
    output->setPos(m_output->globalPosition());
    output->setRotation(s_rotationMap[m_output->transform()]);

    ModeList modeList;
    QStringList preferredModeIds;
    m_modeIdMap.clear();
    QString currentModeId = QStringLiteral("-1");

    for (const Wl::OutputDevice::Mode &wlMode : m_output->modes()) {
        ModePtr mode(new Mode());
        const QString name = modeName(wlMode);

        QString modeId = QString::number(wlMode.id);
        if (modeId.isEmpty()) {
            qCDebug(KSCREEN_WAYLAND) << "Could not create mode id from"
                                     << wlMode.id << ", using" << name << "instead.";
            modeId = name;
        }

        if (m_modeIdMap.contains(modeId)) {
            qCWarning(KSCREEN_WAYLAND) << "Mode id already in use:" << modeId;
        }
        mode->setId(modeId);

        // KWayland gives the refresh rate as int in mHz
        mode->setRefreshRate(wlMode.refreshRate / 1000.0);
        mode->setSize(wlMode.size);
        mode->setName(name);

        if (wlMode.flags.testFlag(Wl::OutputDevice::Mode::Flag::Current)) {
            currentModeId = modeId;
        }
        if (wlMode.flags.testFlag(Wl::OutputDevice::Mode::Flag::Preferred)) {
            preferredModeIds << modeId;
        }

        // Update the kscreen => kwayland mode id translation map
        m_modeIdMap.insert(modeId, wlMode.id);
        // Add to the modelist which gets set on the output
        modeList[modeId] = mode;
    }

    if (currentModeId == QLatin1String("-1")) {
        qCWarning(KSCREEN_WAYLAND) << "Could not find the current mode id" << modeList;
    }

    output->setCurrentModeId(currentModeId);
    output->setPreferredModes(preferredModeIds);
    output->setModes(modeList);
    output->setScale(m_output->scale());
    output->setType(Utils::guessOutputType(m_output->model(), m_output->model()));
}

QString WaylandOutput::modeName(const Wl::OutputDevice::Mode &m) const
{
    return QString::number(m.size.width()) + QLatin1Char('x') +
           QString::number(m.size.height()) + QLatin1Char('@') +
           QString::number(qRound(m.refreshRate/1000.0));
}

QString WaylandOutput::name() const
{
    Q_ASSERT(m_output);
    return QStringLiteral("%1 %2").arg(m_output->manufacturer(), m_output->model());
}

QDebug operator<<(QDebug dbg, const WaylandOutput *output)
{
    dbg << "WaylandOutput(Id:" << output->id() <<", Name:" << \
        QString(output->outputDevice()->manufacturer() + QLatin1Char(' ') + \
        output->outputDevice()->model())  << ")";
    return dbg;
}
