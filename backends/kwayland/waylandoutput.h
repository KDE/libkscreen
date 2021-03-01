/*
 *  SPDX-FileCopyrightText: 2014-2015 Sebastian Kügler <sebas@kde.org>
 *
 *  SPDX-License-Identifier: LGPL-2.1-or-later
 */
#pragma once

#include "waylandconfig.h"

#include "abstractbackend.h"
#include "output.h"

#include <KWayland/Client/outputdevice.h>
#include <KWayland/Client/registry.h>

#include <QLoggingCategory>
#include <QScreen>
#include <QSize>

namespace KWayland
{
namespace Client
{
class OutputConfiguration;
}
}

namespace KScreen
{
class WaylandOutput : public QObject
{
    Q_OBJECT

public:
    explicit WaylandOutput(quint32 id, WaylandConfig *parent = nullptr);
    ~WaylandOutput() override = default;

    KScreen::OutputPtr toKScreenOutput();
    void updateKScreenOutput(KScreen::OutputPtr &output);

    quint32 id() const;
    QString name() const;
    bool enabled() const;

    KWayland::Client::OutputDevice *outputDevice() const;
    void createOutputDevice(KWayland::Client::Registry *registry, quint32 name, quint32 version);

    bool setWlConfig(KWayland::Client::OutputConfiguration *wlConfig, const KScreen::OutputPtr &output);

Q_SIGNALS:
    void deviceRemoved();

    void complete();
    // only emitted after complete signal
    void changed();

private:
    QString modeName(const KWayland::Client::OutputDevice::Mode &m) const;

    quint32 m_id;
    KWayland::Client::OutputDevice *m_device;
    KWayland::Client::Registry *m_registry;

    // left-hand-side: KScreen::Mode, right-hand-side: KWayland's mode.id
    QMap<QString, int> m_modeIdMap;
};

}

KSCREEN_EXPORT QDebug operator<<(QDebug dbg, const KScreen::WaylandOutput *output);
