/*
 *  SPDX-FileCopyrightText: 2014-2015 Sebastian Kügler <sebas@kde.org>
 *  SPDX-FileCopyrightText: 2021 Méven Car <meven.car@enioka.com>
 *
 *  SPDX-License-Identifier: LGPL-2.1-or-later
 */
#include "waylandoutputdevice.h"
#include "../utils.h"
#include "waylandoutputmanagement.h"

#include "kscreen_kwayland_logging.h"

#include <wayland-server-protocol.h>

using namespace KScreen;

WaylandOutputDevice::WaylandOutputDevice(int id)
    : QObject()
    , kde_output_device_v2()
    , m_id(id)
{
}

WaylandOutputDevice::~WaylandOutputDevice()
{
    qDeleteAll(m_modes);

    kde_output_device_v2_destroy(object());
}

void WaylandOutputDevice::kde_output_device_v2_geometry(int32_t x,
                                                        int32_t y,
                                                        int32_t physical_width,
                                                        int32_t physical_height,
                                                        int32_t subpixel,
                                                        const QString &make,
                                                        const QString &model,
                                                        int32_t transform)
{
    m_pos = QPoint(x, y);
    m_physicalSize = QSize(physical_width, physical_height);
    m_subpixel = subpixel;
    m_manufacturer = make;
    m_model = model;
    m_transform = transform;
}

void WaylandOutputDevice::kde_output_device_v2_current_mode(struct ::kde_output_device_mode_v2 *mode)
{
    auto m = WaylandOutputDeviceMode::get(mode);

    if (*m == *m_mode) {
        // unchanged
        return;
    }
    m_mode = m;
}

void WaylandOutputDevice::kde_output_device_v2_mode(struct ::kde_output_device_mode_v2 *mode)
{
    WaylandOutputDeviceMode *m = new WaylandOutputDeviceMode(mode);
    // last mode sent is the current one
    m_mode = m;
    m_modes.append(m);

    connect(m, &WaylandOutputDeviceMode::removed, this, [this, m]() {
        m_modes.removeOne(m);
        if (m_mode == m) {
            if (!m_modes.isEmpty()) {
                m_mode = m_modes.first();
            } else {
                // was last mode
                qFatal("KWaylandBackend: no output modes available anymore, this seems like a compositor bug");
            }
        }

        delete m;
    });
}

OutputPtr WaylandOutputDevice::toKScreenOutput()
{
    OutputPtr output(new Output());
    output->setId(m_id);
    updateKScreenOutput(output);
    return output;
}

Output::Rotation toKScreenRotation(int32_t transform)
{
    switch (transform) {
    case WL_OUTPUT_TRANSFORM_NORMAL:
        return Output::None;
    case WL_OUTPUT_TRANSFORM_90:
        return Output::Left;
    case WL_OUTPUT_TRANSFORM_180:
        return Output::Inverted;
    case WL_OUTPUT_TRANSFORM_270:
        return Output::Right;
    case WL_OUTPUT_TRANSFORM_FLIPPED:
        qCWarning(KSCREEN_WAYLAND) << "flipped transform is unsupported by kscreen";
        return Output::None;
    case WL_OUTPUT_TRANSFORM_FLIPPED_90:
        qCWarning(KSCREEN_WAYLAND) << "flipped-90 transform is unsupported by kscreen";
        return Output::Left;
    case WL_OUTPUT_TRANSFORM_FLIPPED_180:
        qCWarning(KSCREEN_WAYLAND) << "flipped-180 transform is unsupported by kscreen";
        return Output::Inverted;
    case WL_OUTPUT_TRANSFORM_FLIPPED_270:
        qCWarning(KSCREEN_WAYLAND) << "flipped-270 transform is unsupported by kscreen";
        return Output::Right;
    default:
        Q_UNREACHABLE();
    }
}

wl_output_transform toKWaylandTransform(const Output::Rotation rotation)
{
    switch (rotation) {
    case Output::None:
        return WL_OUTPUT_TRANSFORM_NORMAL;
    case Output::Left:
        return WL_OUTPUT_TRANSFORM_90;
    case Output::Inverted:
        return WL_OUTPUT_TRANSFORM_180;
    case Output::Right:
        return WL_OUTPUT_TRANSFORM_270;
    default:
        Q_UNREACHABLE();
    }
}

void KScreen::WaylandOutputDevice::updateKScreenModes(OutputPtr &output)
{
    ModeList modeList;
    QStringList preferredModeIds;
    QString currentModeId = QStringLiteral("-1");
    int modeId = 0;

    for (const WaylandOutputDeviceMode *wlMode : qAsConst(m_modes)) {
        ModePtr mode(new Mode());

        const QString modeIdStr = QString::number(modeId);
        // KWayland gives the refresh rate as int in mHz
        mode->setId(modeIdStr);
        mode->setRefreshRate(wlMode->refreshRate() / 1000.0);
        mode->setSize(wlMode->size());
        mode->setName(modeName(wlMode));

        if (m_mode == wlMode) {
            currentModeId = modeIdStr;
        }

        if (wlMode->preferred()) {
            preferredModeIds << modeIdStr;
        }

        // Add to the modelist which gets set on the output
        modeList[modeIdStr] = mode;
        modeId++;
    }
    output->setCurrentModeId(currentModeId);
    output->setPreferredModes(preferredModeIds);
    output->setModes(modeList);
}

void WaylandOutputDevice::updateKScreenOutput(OutputPtr &output)
{
    // Initialize primary output
    output->setId(m_id);
    output->setEnabled(enabled());
    output->setConnected(true);
    output->setPrimary(true); // FIXME: wayland doesn't have the concept of a primary display
    output->setName(name());
    output->setSizeMm(m_physicalSize);
    output->setPos(m_pos);
    output->setRotation(toKScreenRotation(m_transform));
    if (!output->edid()) {
        output->setEdid(m_edid);
    }

    QSize currentSize = m_mode->size();
    output->setSize(output->isHorizontal() ? currentSize : currentSize.transposed());
    output->setScale(m_factor);
    output->setType(Utils::guessOutputType(m_model, m_model));
    output->setCapabilities(static_cast<Output::Capabilities>(static_cast<uint32_t>(m_flags)));
    output->setOverscan(m_overscan);
    output->setVrrPolicy(static_cast<Output::VrrPolicy>(m_vrr_policy));
    output->setRgbRange(static_cast<Output::RgbRange>(m_rgbRange));

    updateKScreenModes(output);
}

QString WaylandOutputDevice::modeId() const
{
    return QString::number(m_modes.indexOf(m_mode));
}

WaylandOutputDeviceMode *WaylandOutputDevice::deviceModeFromId(const int modeId) const
{
    return m_modes.at(modeId);
}

bool WaylandOutputDevice::setWlConfig(WaylandOutputConfiguration *wlConfig, const KScreen::OutputPtr &output)
{
    bool changed = false;

    // enabled?
    if (enabled() != output->isEnabled()) {
        changed = true;
        wlConfig->enable(object(), output->isEnabled());
    }

    // position
    if (globalPosition() != output->pos()) {
        changed = true;
        wlConfig->position(object(), output->pos().x(), output->pos().y());
    }

    // scale
    if (!qFuzzyCompare(scale(), output->scale())) {
        changed = true;
        wlConfig->scale(object(), wl_fixed_from_double(output->scale()));
    }

    // rotation
    if (toKScreenRotation(m_transform) != output->rotation()) {
        changed = true;
        wlConfig->transform(object(), toKWaylandTransform(output->rotation()));
    }

    // mode
    const ModePtr mode = output->currentMode();
    if (mode->size() != pixelSize() || mode->refreshRate() != refreshRate()) {
        bool toIntOk;
        int modeId = mode->id().toInt(&toIntOk);
        Q_ASSERT(toIntOk);

        changed = true;
        wlConfig->mode(object(), deviceModeFromId(modeId)->object());
    }

    // overscan
    if ((output->capabilities() & Output::Capability::Overscan) && overscan() != output->overscan()) {
        wlConfig->overscan(object(), output->overscan());
        changed = true;
    }

    // vrr
    if ((output->capabilities() & Output::Capability::Vrr) && vrrPolicy() != static_cast<uint32_t>(output->vrrPolicy())) {
        wlConfig->set_vrr_policy(object(), static_cast<uint32_t>(output->vrrPolicy()));
        changed = true;
    }

    if ((output->capabilities() & Output::Capability::RgbRange) && rgbRange() != static_cast<uint32_t>(output->rgbRange())) {
        wlConfig->set_rgb_range(object(), static_cast<uint32_t>(output->rgbRange()));
        changed = true;
    }

    return changed;
}

QString WaylandOutputDevice::modeName(const WaylandOutputDeviceMode *m) const
{
    return QString::number(m->size().width()) + QLatin1Char('x') + QString::number(m->size().height()) + QLatin1Char('@')
        + QString::number(qRound(m->refreshRate() / 1000.0));
}

QString WaylandOutputDevice::name() const
{
    return QStringLiteral("%1 %2").arg(m_manufacturer, m_model);
}

QDebug operator<<(QDebug dbg, const WaylandOutputDevice *output)
{
    dbg << "WaylandOutput(Id:" << output->id() << ", Name:" << QString(output->manufacturer() + QLatin1Char(' ') + output->model()) << ")";
    return dbg;
}

void WaylandOutputDevice::kde_output_device_v2_done()
{
    Q_EMIT done();
}

void WaylandOutputDevice::kde_output_device_v2_scale(wl_fixed_t factor)
{
    m_factor = wl_fixed_to_double(factor);
}

void WaylandOutputDevice::kde_output_device_v2_edid(const QString &edid)
{
    m_edid = QByteArray::fromBase64(edid.toUtf8());
}

void WaylandOutputDevice::kde_output_device_v2_enabled(int32_t enabled)
{
    m_enabled = enabled;
}

void WaylandOutputDevice::kde_output_device_v2_uuid(const QString &uuid)
{
    m_uuid = uuid;
}

void WaylandOutputDevice::kde_output_device_v2_serial_number(const QString &serialNumber)
{
    m_serialNumber = serialNumber;
}

void WaylandOutputDevice::kde_output_device_v2_eisa_id(const QString &eisaId)
{
    m_eisaId = eisaId;
}

void WaylandOutputDevice::kde_output_device_v2_capabilities(uint32_t flags)
{
    m_flags = flags;
}

void WaylandOutputDevice::kde_output_device_v2_overscan(uint32_t overscan)
{
    m_overscan = overscan;
}

void WaylandOutputDevice::kde_output_device_v2_vrr_policy(uint32_t vrr_policy)
{
    m_vrr_policy = vrr_policy;
}

void WaylandOutputDevice::kde_output_device_v2_rgb_range(uint32_t rgb_range)
{
    m_rgbRange = rgb_range;
}

QByteArray WaylandOutputDevice::edid() const
{
    return m_edid;
}

bool WaylandOutputDevice::enabled() const
{
    return m_enabled;
}

int WaylandOutputDevice::id() const
{
    return m_id;
}

qreal WaylandOutputDevice::scale() const
{
    return m_factor;
}

QString WaylandOutputDevice::manufacturer() const
{
    return m_manufacturer;
}

QString WaylandOutputDevice::model() const
{
    return m_model;
}

QPoint WaylandOutputDevice::globalPosition() const
{
    return m_pos;
}

QSize WaylandOutputDevice::pixelSize() const
{
    return m_mode->size();
}

int WaylandOutputDevice::refreshRate() const
{
    return m_mode->refreshRate();
}

uint32_t WaylandOutputDevice::vrrPolicy() const
{
    return m_vrr_policy;
}

uint32_t WaylandOutputDevice::overscan() const
{
    return m_overscan;
}

uint32_t WaylandOutputDevice::capabilities() const
{
    return m_flags;
}

uint32_t WaylandOutputDevice::rgbRange() const
{
    return m_rgbRange;
}

#include "waylandoutputdevice.moc"
