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

#include "xlibandxrandr.h"

class XRandRConfig;
namespace KScreen
{
class Config;
class Output;
}

class XRandROutput : public QObject
{
    Q_OBJECT
    Q_FLAGS(Property Properties)

public:
    typedef QMap<int, XRandROutput*> Map;

    enum Property {
        PropertyId              = 1 << 0,
        PropertyName            = 1 << 1,
        PropertyType            = 1 << 2,
        PropertyIcon            = 1 << 3,
        PropertyModes           = 1 << 4,
        PropertyPos             = 1 << 5,
        PropertyRotation        = 1 << 6,
        PropertyCurrentMode     = 1 << 7,
        PropertyConnected       = 1 << 8,
        PropertyEnabled         = 1 << 9,
        PropertyPrimary         = 1 << 10,
        PropertyClones          = 1 << 11,
        PropertyEdid            = 1 << 12
    };
    Q_DECLARE_FLAGS(Properties, Property)

    explicit XRandROutput(int id, XRROutputInfo *outputInfo, XRandRConfig *config = 0);
    virtual ~XRandROutput();

    void setOutputProperty(XRandROutput::Property id, const QVariant &value);
    QVariant outputProperty(XRandROutput::Property id);

    KScreen::Output* toKScreenOutput(KScreen::Config *parent) const;
private:
    QMap<Property, QVariant> m_properties;
};

Q_DECLARE_METATYPE(XRandROutput::Map)

#endif // XRANDROUTPUT_H
