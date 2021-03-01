/*
 *  SPDX-FileCopyrightText: 2012 Alejandro Fiestas Olivares <afiestas@kde.org>
 *
 *  SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef XRANDR11_BACKEND_H
#define XRANDR11_BACKEND_H

#include "../xcbwrapper.h"
#include "abstractbackend.h"

#include <QLoggingCategory>
#include <QObject>

class XCBEventListener;

class XRandR11 : public KScreen::AbstractBackend
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.kf5.kscreen.backends.xrandr11")

public:
    explicit XRandR11();
    ~XRandR11() override;

    QString name() const override;
    QString serviceName() const override;
    KScreen::ConfigPtr config() const override;
    void setConfig(const KScreen::ConfigPtr &config) override;
    bool isValid() const override;

private Q_SLOTS:
    void updateConfig();

private:
    bool m_valid;
    XCBEventListener *m_x11Helper;
    KScreen::ConfigPtr m_currentConfig;
    xcb_timestamp_t m_currentTimestamp;
};

Q_DECLARE_LOGGING_CATEGORY(KSCREEN_XRANDR11)

#endif // FAKE_BACKEND_H
