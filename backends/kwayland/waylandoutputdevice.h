/*
 *  SPDX-FileCopyrightText: 2014-2015 Sebastian Kügler <sebas@kde.org>
 *  SPDX-FileCopyrightText: 2021 Méven Car <meven.car@enioka.com>
 *
 *  SPDX-License-Identifier: LGPL-2.1-or-later
 */
#pragma once

#include "waylandoutputdevicemode.h"

#include "qwayland-kde-output-device-v2.h"

#include "kscreen_export.h"
#include "types.h"

#include <QPoint>
#include <QSize>

namespace KScreen
{
class WaylandOutputConfiguration;

class WaylandOutputDevice : public QObject, public QtWayland::kde_output_device_v2
{
    Q_OBJECT

public:
    WaylandOutputDevice(int id);

    ~WaylandOutputDevice();

    QByteArray edid() const;
    bool enabled() const;
    int id() const;
    QString name() const;
    QString model() const;
    QString manufacturer() const;
    qreal scale() const;
    QPoint globalPosition() const;
    QSize pixelSize() const;
    float refreshRate() const;
    uint32_t vrrPolicy() const;
    uint32_t overscan() const;
    uint32_t capabilities() const;
    uint32_t rgbRange() const;

    OutputPtr toKScreenOutput(const QMap<int, WaylandOutputDevice *> &outputMap);
    void updateKScreenOutput(OutputPtr &output, const QMap<int, WaylandOutputDevice *> &outputMap);
    void updateKScreenModes(OutputPtr &output);

    void setIndex(uint32_t priority);
    uint32_t index() const;
    bool setWlConfig(WaylandOutputConfiguration *wlConfig, const KScreen::OutputPtr &output, const QMap<int, WaylandOutputDevice *> &outputMap);

    QString modeId() const;
    QString uuid() const
    {
        return m_uuid;
    }

Q_SIGNALS:
    void done();

protected:
    void kde_output_device_v2_geometry(int32_t x,
                                       int32_t y,
                                       int32_t physical_width,
                                       int32_t physical_height,
                                       int32_t subpixel,
                                       const QString &make,
                                       const QString &model,
                                       int32_t transform) override;
    void kde_output_device_v2_current_mode(struct ::kde_output_device_mode_v2 *mode) override;
    void kde_output_device_v2_mode(struct ::kde_output_device_mode_v2 *mode) override;
    void kde_output_device_v2_done() override;
    void kde_output_device_v2_scale(wl_fixed_t factor) override;
    void kde_output_device_v2_edid(const QString &raw) override;
    void kde_output_device_v2_enabled(int32_t enabled) override;
    void kde_output_device_v2_uuid(const QString &uuid) override;
    void kde_output_device_v2_serial_number(const QString &serialNumber) override;
    void kde_output_device_v2_eisa_id(const QString &eisaId) override;
    void kde_output_device_v2_capabilities(uint32_t flags) override;
    void kde_output_device_v2_overscan(uint32_t overscan) override;
    void kde_output_device_v2_vrr_policy(uint32_t vrr_policy) override;
    void kde_output_device_v2_rgb_range(uint32_t rgb_range) override;
    void kde_output_device_v2_name(const QString &name) override;
    void kde_output_device_v2_high_dynamic_range(uint32_t hdr_enabled) override;
    void kde_output_device_v2_sdr_brightness(uint32_t sdr_brightness) override;
    void kde_output_device_v2_wide_color_gamut(uint32_t wcg_enabled) override;
    void kde_output_device_v2_auto_rotate_policy(uint32_t policy) override;
    void kde_output_device_v2_icc_profile_path(const QString &profile) override;
    void kde_output_device_v2_brightness_metadata(uint32_t max_peak_brightness, uint32_t max_frame_average_brightness, uint32_t min_brightness) override;
    void kde_output_device_v2_brightness_overrides(int32_t max_peak_brightness, int32_t max_average_brightness, int32_t min_brightness) override;
    void kde_output_device_v2_sdr_gamut_wideness(uint32_t value) override;
    void kde_output_device_v2_color_profile_source(uint32_t source) override;
    void kde_output_device_v2_brightness(uint32_t brightness) override;
    void kde_output_device_v2_color_power_tradeoff(uint32_t preference) override;
    void kde_output_device_v2_dimming(uint32_t dimming) override;
    void kde_output_device_v2_replication_source(const QString &source) override;
    void kde_output_device_v2_ddc_ci_allowed(uint32_t allowed) override;
    void kde_output_device_v2_max_bits_per_color(uint32_t max_bpc) override;
    void kde_output_device_v2_max_bits_per_color_range(uint32_t min_value, uint32_t max_value) override;
    void kde_output_device_v2_automatic_max_bits_per_color_limit(uint32_t max_bpc_limit) override;
    void kde_output_device_v2_edr_policy(uint32_t policy) override;
    void kde_output_device_v2_sharpness(uint32_t sharpness) override;
    void kde_output_device_v2_abm_level(uint32_t level) override;

private:
    QString modeName(const WaylandOutputDeviceMode *m) const;
    WaylandOutputDeviceMode *deviceModeFromId(const QString &id) const;

    WaylandOutputDeviceMode *m_mode;
    QList<WaylandOutputDeviceMode *> m_modes;

    int m_id;
    QPoint m_pos;
    QSize m_physicalSize;
    int32_t m_subpixel;
    QString m_manufacturer;
    QString m_model;
    int32_t m_transform;
    qreal m_factor;
    QByteArray m_edid;
    int32_t m_enabled;
    QString m_uuid;
    QString m_serialNumber;
    QString m_outputName;
    QString m_eisaId;
    uint32_t m_capabilities;
    uint32_t m_overscan;
    uint32_t m_vrr_policy;
    uint32_t m_rgbRange;
    uint32_t m_index;
    bool m_hdrEnabled = false;
    uint32_t m_sdrBrightness = 200;
    bool m_wideColorGamutEnabled = false;
    uint32_t m_autoRotatePolicy = 1;
    QString m_iccProfilePath;
    double m_maxPeakBrightness = 0;
    double m_maxAverageBrightness = 0;
    double m_minBrightness = 0;
    std::optional<double> m_maxPeakBrightnessOverride;
    std::optional<double> m_maxAverageBrightnessOverride;
    std::optional<double> m_minBrightnessOverride;
    double m_sdrGamutWideness = 0;
    uint32_t m_colorProfileSource = color_profile_source_sRGB;
    bool m_ddcCiAllowed = true;
    uint32_t m_brightness = 10'000;
    color_power_tradeoff m_colorPowerPreference = color_power_tradeoff_efficiency;
    uint32_t m_dimming = 10'000;
    QString m_replicationSource;
    uint32_t m_maxBpc = 0;
    uint32_t m_autoMaxBpcLimit = 0;
    struct {
        uint32_t min = 0;
        uint32_t max = 0;
    } bpcRange;
    uint32_t m_edrPolicy = edr_policy_always;
    uint32_t m_sharpness = 0;
    uint32_t m_abmLevel = abm_level_off;
};

}

KSCREEN_EXPORT QDebug operator<<(QDebug dbg, const KScreen::WaylandOutputDevice *output);
