/*************************************************************************************
 *  Copyright (C) 2012 by Dan Vrátil <dvratil@redhat.com>                            *
 *                                                                                   *
 *  This program is free software; you can redistribute it and/or                    *
 *  modify it under the terms of the GNU General Public License                      *
 *  as published by the Free Software Foundation; either version 2                   *
 *  of the License, or (at your option) any later version.                           *
 *                                                                                   *
 *  This program is distributed in the hope that it will be useful,                  *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of                   *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                    *
 *  GNU General Public License for more details.                                     *
 *                                                                                   *
 *  You should have received a copy of the GNU General Public License                *
 *  along with this program; if not, write to the Free Software                      *
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA   *
 *************************************************************************************/


#ifndef XRANDROUTPUT_H
#define XRANDROUTPUT_H

#include <QObject>
#include <QMap>
#include <QVariant>
#include <QPointer>

#include "xlibandxrandr.h"
#include "xrandrmode.h"
#include "output.h"

class XRandRConfig;
namespace KScreen
{
class Config;
class Output;
class Edid;
}

class XRandROutput : public QObject
{
    Q_OBJECT
    Q_FLAGS(Property Properties)

public:
    typedef QMap<int, XRandROutput*> Map;

    enum Property {
        PropertyNone            = 1 << 0,
        PropertyId              = 1 << 1,
        PropertyName            = 1 << 2,
        PropertyIcon            = 1 << 3,
        PropertyModes           = 1 << 4,
        PropertyPos             = 1 << 5,
        PropertyRotation        = 1 << 6,
        PropertyCurrentMode     = 1 << 7,
        PropertyConnected       = 1 << 8,
        PropertyEnabled         = 1 << 9,
        PropertyPrimary         = 1 << 10,
        PropertyClones          = 1 << 11,
        PropertyEdid            = 1 << 12,
        PropertyPreferredMode   = 1 << 13
    };

    enum PrimaryChange {
        NoChange = 0,
        SetPrimary = 1,
        UnsetPrimary = 2,
    };

    Q_DECLARE_FLAGS(Properties, Property)

    explicit XRandROutput(int id, bool primary, XRandRConfig *config = 0);
    virtual ~XRandROutput();

    void update(PrimaryChange primary = NoChange);

    int id() const;
    bool isEnabled() const;
    bool isConnected() const;
    bool isPrimary() const;
    QPoint position() const;
    QString currentModeId() const;
    XRandRMode* currentMode() const;
    KScreen::Output::Rotation rotation() const;
    inline bool isHorizontal() const { return ((m_rotation == KScreen::Output::None) || (m_rotation == KScreen::Output::Inverted)); }
    KScreen::Edid* edid() const;

    KScreen::Output* toKScreenOutput(KScreen::Config *parent) const;
    void updateKScreenOutput(KScreen::Output *output) const;
private:
    void updateOutput(const XRROutputInfo *outputInfo);
    void updateModes(const XRROutputInfo *outputInfo);
    void fetchType();
    KScreen::Output::Type typeFromName();
    QByteArray typeFromProperty() const;

    int m_id;
    QString m_name;
    KScreen::Output::Type m_type;
    QString m_icon;
    XRandRMode::Map m_modes;
    QPoint m_position;
    KScreen::Output::Rotation m_rotation;
    QString m_currentMode;
    QStringList m_preferredModes;
    int m_connected : 1;
    int m_enabled : 1;
    int m_primary : 1;
    QList<int> m_clones;
    mutable QPointer<KScreen::Edid> m_edid;

    mutable int m_changedProperties;
};

Q_DECLARE_METATYPE(XRandROutput::Map)

#endif // XRANDROUTPUT_H
