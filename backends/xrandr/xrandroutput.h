/*************************************************************************************
 *  Copyright 2012, 2013  Daniel Vrátil <dvratil@redhat.com>                         *
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

#ifndef XRANDROUTPUT_H
#define XRANDROUTPUT_H

#include <QObject>
#include <QMap>
#include <QVariant>
#include <QPointer>

#include "xrandrmode.h"
#include "output.h"
#include "../xcbwrapper.h"

class XRandRConfig;
class XRandRCrtc;
namespace KScreen
{
class Config;
class Output;
}

class XRandROutput : public QObject
{
    Q_OBJECT

public:
    typedef QMap<xcb_randr_output_t, XRandROutput*> Map;

    explicit XRandROutput(xcb_randr_output_t id, XRandRConfig *config);
    virtual ~XRandROutput();

    void disabled();
    void disconnected();

    void update();
    void update(xcb_randr_crtc_t crtc, xcb_randr_mode_t mode, xcb_randr_connection_t conn, bool primary);

    xcb_randr_output_t id() const;
    bool isEnabled() const;
    bool isConnected() const;
    bool isPrimary() const;
    QPoint position() const;
    QString currentModeId() const;
    XRandRMode::Map modes() const;
    XRandRMode* currentMode() const;
    KScreen::Output::Rotation rotation() const;
    bool isHorizontal() const;
    QByteArray edid() const;
    XRandRCrtc* crtc() const;

    KScreen::OutputPtr toKScreenOutput() const;

Q_SIGNALS:
    void outputRemoved(xcb_randr_output_t id);

private:
    void init();
    void updateModes(const XCB::OutputInfo &outputInfo);

    static KScreen::Output::Type fetchOutputType(xcb_randr_output_t outputId, const QString &name);
    static KScreen::Output::Type typeFromName(const QString &name);
    static QByteArray typeFromProperty(xcb_randr_output_t outputId);

    XRandRConfig *m_config;
    xcb_randr_output_t m_id;
    QString m_name;
    xcb_randr_connection_t m_connected;
    KScreen::Output::Type m_type;
    QString m_icon;
    XRandRMode::Map m_modes;
    QStringList m_preferredModes;
    bool m_primary;
    QList<xcb_randr_output_t> m_clones;
    mutable QByteArray m_edid;
    unsigned int m_widthMm;
    unsigned int m_heightMm;
    XRandRCrtc *m_crtc;
};

Q_DECLARE_METATYPE(XRandROutput::Map)

#endif // XRANDROUTPUT_H
