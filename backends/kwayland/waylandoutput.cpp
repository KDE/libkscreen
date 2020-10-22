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

#include <KWayland/Client/outputconfiguration.h>
#include <KWayland/Client/outputdevice.h>

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

Output::Rotation toKScreenRotation(const Wl::OutputDevice::Transform transform)
{
    auto it = s_rotationMap.constFind(transform);
    return it.value();
}

Wl::OutputDevice::Transform toKWaylandTransform(const Output::Rotation rotation)
{
    return s_rotationMap.key(rotation);
}

WaylandOutput::WaylandOutput(quint32 id, WaylandConfig *parent)
    : QObject(parent)
    , m_id(id)
    , m_device(nullptr)
{
}

quint32 WaylandOutput::id() const
{
    Q_ASSERT(m_device);
    return m_id;
}

bool WaylandOutput::enabled() const
{
    return m_device != nullptr;
}

Wl::OutputDevice* WaylandOutput::outputDevice() const
{
    return m_device;
}

void WaylandOutput::createOutputDevice(Wl::Registry *registry, quint32 name, quint32 version)
{
    Q_ASSERT(!m_device);
    m_device = registry->createOutputDevice(name, version);

    connect(m_device, &Wl::OutputDevice::removed, this, &WaylandOutput::deviceRemoved);
    connect(m_device, &Wl::OutputDevice::done, this, [this]() {
                Q_EMIT complete();
                connect(m_device, &Wl::OutputDevice::changed, this, &WaylandOutput::changed);
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
    output->setEnabled(m_device->enabled() == Wl::OutputDevice::Enablement::Enabled);
    output->setConnected(true);
    output->setPrimary(true); // FIXME: wayland doesn't have the concept of a primary display
    output->setName(name());
    output->setSizeMm(m_device->physicalSize());
    output->setPos(m_device->globalPosition());
    output->setRotation(s_rotationMap[m_device->transform()]);

    ModeList modeList;
    QStringList preferredModeIds;
    m_modeIdMap.clear();
    QString currentModeId = QStringLiteral("-1");

    QSize currentSize;
    for (const Wl::OutputDevice::Mode &wlMode : m_device->modes()) {
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
            currentSize = wlMode.size;
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

    output->setSize(output->isHorizontal() ? currentSize : currentSize.transposed());
    output->setCurrentModeId(currentModeId);
    output->setPreferredModes(preferredModeIds);
    output->setModes(modeList);
    output->setScale(m_device->scaleF());
    output->setType(Utils::guessOutputType(m_device->model(), m_device->model()));
}

bool WaylandOutput::setWlConfig(Wl::OutputConfiguration *wlConfig,
                                const KScreen::OutputPtr &output)
{
    bool changed = false;

    // enabled?
    if ((m_device->enabled() == Wl::OutputDevice::Enablement::Enabled)
            != output->isEnabled()) {
        changed = true;
        const auto enablement = output->isEnabled() ? Wl::OutputDevice::Enablement::Enabled :
                                                      Wl::OutputDevice::Enablement::Disabled;
        wlConfig->setEnabled(m_device, enablement);
    }

    // position
    if (m_device->globalPosition() != output->pos()) {
        changed = true;
        wlConfig->setPosition(m_device, output->pos());
    }

    // scale
    if (!qFuzzyCompare(m_device->scaleF(), output->scale())) {
        changed = true;
        wlConfig->setScaleF(m_device, output->scale());
    }

    // rotation
    if (toKScreenRotation(m_device->transform()) != output->rotation()) {
        changed = true;
        wlConfig->setTransform(m_device, toKWaylandTransform(output->rotation()));
    }

    // mode
    if (m_modeIdMap.contains(output->currentModeId())) {
        const int newModeId = m_modeIdMap.value(output->currentModeId(), -1);
        if (newModeId != m_device->currentMode().id) {
            changed = true;
            wlConfig->setMode(m_device, newModeId);
        }
    } else {
        qCWarning(KSCREEN_WAYLAND) << "Invalid kscreen mode id:" << output->currentModeId()
                                   << "\n\n" << m_modeIdMap;
    }
    return changed;
}

QString WaylandOutput::modeName(const Wl::OutputDevice::Mode &m) const
{
    return QString::number(m.size.width()) + QLatin1Char('x') +
           QString::number(m.size.height()) + QLatin1Char('@') +
           QString::number(qRound(m.refreshRate/1000.0));
}

QString WaylandOutput::name() const
{
    Q_ASSERT(m_device);
    return QStringLiteral("%1 %2").arg(m_device->manufacturer(), m_device->model());
}

QDebug operator<<(QDebug dbg, const WaylandOutput *output)
{
    dbg << "WaylandOutput(Id:" << output->id() <<", Name:" << \
        QString(output->outputDevice()->manufacturer() + QLatin1Char(' ') + \
        output->outputDevice()->model())  << ")";
    return dbg;
}
