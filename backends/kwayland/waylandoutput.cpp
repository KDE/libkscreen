/*
 *  SPDX-FileCopyrightText: 2014-2015 Sebastian Kügler <sebas@kde.org>
 *
 *  SPDX-License-Identifier: LGPL-2.1-or-later
 */
#include "waylandoutput.h"
#include "../utils.h"
#include "waylandbackend.h"
#include "waylandconfig.h"

#include <edid.h>
#include <mode.h>

#include <KWayland/Client/outputconfiguration.h>
#include <KWayland/Client/outputdevice.h>

using namespace KScreen;
namespace Wl = KWayland::Client;

Output::Rotation toKScreenRotation(const Wl::OutputDevice::Transform transform)
{
    switch (transform) {
    case KWayland::Client::OutputDevice::Transform::Normal:
        return Output::None;
    case KWayland::Client::OutputDevice::Transform::Rotated90:
        return Output::Left;
    case KWayland::Client::OutputDevice::Transform::Rotated180:
        return Output::Inverted;
    case KWayland::Client::OutputDevice::Transform::Rotated270:
        return Output::Right;
    case KWayland::Client::OutputDevice::Transform::Flipped:
        qCWarning(KSCREEN_WAYLAND) << "flipped transform is unsupported by kscreen";
        return Output::None;
    case KWayland::Client::OutputDevice::Transform::Flipped90:
        qCWarning(KSCREEN_WAYLAND) << "flipped-90 transform is unsupported by kscreen";
        return Output::Left;
    case KWayland::Client::OutputDevice::Transform::Flipped180:
        qCWarning(KSCREEN_WAYLAND) << "flipped-180 transform is unsupported by kscreen";
        return Output::Inverted;
    case KWayland::Client::OutputDevice::Transform::Flipped270:
        qCWarning(KSCREEN_WAYLAND) << "flipped-270 transform is unsupported by kscreen";
        return Output::Right;
    default:
        Q_UNREACHABLE();
    }
}

Wl::OutputDevice::Transform toKWaylandTransform(const Output::Rotation rotation)
{
    switch (rotation) {
    case Output::None:
        return KWayland::Client::OutputDevice::Transform::Normal;
    case Output::Left:
        return KWayland::Client::OutputDevice::Transform::Rotated90;
    case Output::Inverted:
        return KWayland::Client::OutputDevice::Transform::Rotated180;
    case Output::Right:
        return KWayland::Client::OutputDevice::Transform::Rotated270;
    default:
        Q_UNREACHABLE();
    }
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

Wl::OutputDevice *WaylandOutput::outputDevice() const
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
    output->setRotation(toKScreenRotation(m_device->transform()));
    if (!output->edid()) {
        output->setEdid(m_device->edid());
    }

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
            qCDebug(KSCREEN_WAYLAND) << "Could not create mode id from" << wlMode.id << ", using" << name << "instead.";
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

bool WaylandOutput::setWlConfig(Wl::OutputConfiguration *wlConfig, const KScreen::OutputPtr &output)
{
    bool changed = false;

    // enabled?
    if ((m_device->enabled() == Wl::OutputDevice::Enablement::Enabled) != output->isEnabled()) {
        changed = true;
        const auto enablement = output->isEnabled() ? Wl::OutputDevice::Enablement::Enabled : Wl::OutputDevice::Enablement::Disabled;
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
        qCWarning(KSCREEN_WAYLAND) << "Invalid kscreen mode id:" << output->currentModeId() << "\n\n" << m_modeIdMap;
    }
    return changed;
}

QString WaylandOutput::modeName(const Wl::OutputDevice::Mode &m) const
{
    return QString::number(m.size.width()) + QLatin1Char('x') + QString::number(m.size.height()) + QLatin1Char('@')
        + QString::number(qRound(m.refreshRate / 1000.0));
}

QString WaylandOutput::name() const
{
    Q_ASSERT(m_device);
    return QStringLiteral("%1 %2").arg(m_device->manufacturer(), m_device->model());
}

QDebug operator<<(QDebug dbg, const WaylandOutput *output)
{
    dbg << "WaylandOutput(Id:" << output->id()
        << ", Name:" << QString(output->outputDevice()->manufacturer() + QLatin1Char(' ') + output->outputDevice()->model()) << ")";
    return dbg;
}
