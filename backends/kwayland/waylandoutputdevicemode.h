/*
 *  SPDX-FileCopyrightText: 2021 Méven Car <meven.car@enioka.com>
 *
 *  SPDX-License-Identifier: LGPL-2.1-or-later
 */
#pragma once

#include "output.h"
#include "qwayland-kde-output-device-v2.h"

#include <QObject>
#include <QSize>

namespace KScreen
{
class WaylandOutputDeviceMode : public QObject, public QtWayland::kde_output_device_mode_v2
{
    Q_OBJECT

public:
    WaylandOutputDeviceMode(struct ::kde_output_device_mode_v2 *object);

    ~WaylandOutputDeviceMode() override;

    QString id() const;
    float refreshRate() const;
    QSize size() const;
    bool preferred() const;
    ModeInfo::Flags flags() const;
    std::optional<Cvt> cvt() const;

    static WaylandOutputDeviceMode *get(struct ::kde_output_device_mode_v2 *object);

Q_SIGNALS:
    void removed();

protected:
    void kde_output_device_mode_v2_size(int32_t width, int32_t height) override;
    void kde_output_device_mode_v2_refresh(int32_t refresh) override;
    void kde_output_device_mode_v2_preferred() override;
    void kde_output_device_mode_v2_removed() override;
    void kde_output_device_mode_v2_flags(uint32_t flags) override;
    void kde_output_device_mode_v2_cvt(uint32_t dot_clock, uint32_t hdisplay, uint32_t hsync_start, uint32_t hsync_end, uint32_t htotal, uint32_t hskew, uint32_t vdisplay, uint32_t vsync_start, uint32_t vsync_end, uint32_t vtotal, uint32_t vscan, uint32_t flags) override;

private:
    QString m_id;
    float m_refreshRate = 60.0;
    QSize m_size;
    bool m_preferred = false;
    ModeInfo::Flags m_flags;
    std::optional<Cvt> m_cvt;
};

}
