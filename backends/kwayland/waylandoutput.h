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

    KWayland::Client::OutputDevice* outputDevice() const;
    void createOutputDevice(KWayland::Client::Registry *registry, quint32 name, quint32 version);

    bool setWlConfig(KWayland::Client::OutputConfiguration *wlConfig,
                   const KScreen::OutputPtr &output);

Q_SIGNALS:
    void deviceRemoved();

    void complete();
    // only emitted after complete signal
    void changed();

private:
    void showOutput();
    QString modeName(const KWayland::Client::OutputDevice::Mode &m) const;

    quint32 m_id;
    KWayland::Client::OutputDevice *m_output;
    KWayland::Client::Registry *m_registry;

    // left-hand-side: KScreen::Mode, right-hand-side: KWayland's mode.id
    QMap<QString, int> m_modeIdMap;
};

}

KSCREEN_EXPORT QDebug operator<<(QDebug dbg, const KScreen::WaylandOutput *output);
