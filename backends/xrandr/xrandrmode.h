/*
 * Copyright 2012  Dan Vrátil <dvratil@redhat.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef XRANDRMODE_H
#define XRANDRMODE_H

#include <QObject>
#include <QMap>
#include <QVariant>

#include "xlibandxrandr.h"

class XRandROutput;
namespace KScreen
{
class Output;
class Mode;
}

class XRandRMode : public QObject
{
    Q_OBJECT
    Q_FLAGS(Property Properties)

public:
    typedef QMap<int, XRandRMode*> Map;

    enum Property {
        PropertyId          = 1 << 0,
        PropertyName        = 1 << 1,
        PropertySize        = 1 << 2,
        PropertyRefreshRate = 1 << 3
    };
    Q_DECLARE_FLAGS(Properties, Property);

    explicit XRandRMode(XRRModeInfo* modeInfo, XRandROutput *output);
    virtual ~XRandRMode();

    void setModeProperty(Property id, const QVariant &value);
    QVariant modeProperty(Property id) const;

    KScreen::Mode* toKScreenMode(KScreen::Output *parent);

private:
    QMap<Property, QVariant> m_properties;
};

Q_DECLARE_METATYPE(XRandRMode::Map)

#endif // XRANDRMODE_H
