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

#ifndef KSCREEN_WAYLAND_OUTPUT_H
#define KSCREEN_WAYLAND_OUTPUT_H

// libkscreen
#include "abstractbackend.h"
#include "config.h"
#include "output.h"

// Own
#include "waylandconfig.h"

#include <QScreen>
#include <QSize>
#include <QLoggingCategory>
#include <KWayland/Client/outputdevice.h>
#include <KWayland/Client/registry.h>


namespace KScreen
{

class WaylandOutput : public QObject
{
    Q_OBJECT

public:
    virtual ~WaylandOutput();

    KScreen::OutputPtr toKScreenOutput();
    void updateKScreenOutput(KScreen::OutputPtr &output);

    quint32 id() const;
    /**
     * Access to the Output's Edid object.
     */
    //KScreen::Edid edid();
    void setEdid(const QString &edidstring);

    bool enabled() const;

    KWayland::Client::OutputDevice* outputDevice() const;
    void bindOutputDevice(KWayland::Client::Registry* registry, KWayland::Client::OutputDevice* op, quint32 name, quint32 version);

    QString name() const;

    // translation methods
    KScreen::Output::Rotation toKScreenRotation(const KWayland::Client::OutputDevice::Transform transform) const;
    KWayland::Client::OutputDevice::Transform toKWaylandTransform(const KScreen::Output::Rotation rotation) const;

    QString toKScreenModeId(int kwaylandmodeid) const;
    int toKWaylandModeId(const QString &kscreenmodeid) const;

Q_SIGNALS:
    void complete();

    // only emitted after complete()
    void changed();

private:
    friend WaylandConfig;
    explicit WaylandOutput(quint32 id, WaylandConfig *parent = 0);
    void showOutput();
    QString modeName(const KWayland::Client::OutputDevice::Mode &m) const;

    mutable QSharedPointer<KScreen::Edid> m_edid;

    KWayland::Client::OutputDevice* m_output;
    KWayland::Client::Registry* m_registry;
    quint32 m_protocolName;
    quint32 m_protocolVersion;
    quint32 m_id;

    QMap<KWayland::Client::OutputDevice::Transform, KScreen::Output::Rotation> m_rotationMap;
    QMap<QString, int> m_modeIdMap; // left-hand-side: KScreen::Mode, right-hand-side: kwayland's mode.id
};

} // namespace

KSCREEN_EXPORT QDebug operator<<(QDebug dbg, const KScreen::WaylandOutput *output);

#endif
