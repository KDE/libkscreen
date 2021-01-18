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
#pragma once

#include "output.h"

#include "../xcbwrapper.h"
#include "xrandrmode.h"

#include <QMap>
#include <QObject>
#include <QVariant>

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
    typedef QMap<xcb_randr_output_t, XRandROutput *> Map;

    explicit XRandROutput(xcb_randr_output_t id, XRandRConfig *config);
    ~XRandROutput() override;

    void disabled();
    void disconnected();

    void update();
    void update(xcb_randr_crtc_t crtc, xcb_randr_mode_t mode, xcb_randr_connection_t conn, bool primary);

    void setIsPrimary(bool primary);

    xcb_randr_output_t id() const;

    bool isEnabled() const;
    bool isConnected() const;
    bool isPrimary() const;

    QPoint position() const;
    QSize size() const;
    QSizeF logicalSize() const;

    QString currentModeId() const;
    XRandRMode::Map modes() const;
    XRandRMode *currentMode() const;

    KScreen::Output::Rotation rotation() const;
    bool isHorizontal() const;

    QByteArray edid() const;
    XRandRCrtc *crtc() const;

    KScreen::OutputPtr toKScreenOutput() const;

    void updateLogicalSize(const KScreen::OutputPtr &output, XRandRCrtc *crtc = nullptr);

private:
    void init();
    void updateModes(const XCB::OutputInfo &outputInfo);

    static KScreen::Output::Type fetchOutputType(xcb_randr_output_t outputId, const QString &name);
    static QByteArray typeFromProperty(xcb_randr_output_t outputId);

    xcb_render_transform_t currentTransform() const;

    XRandRConfig *m_config;
    xcb_randr_output_t m_id;
    QString m_name;
    QString m_icon;
    mutable QByteArray m_edid;

    xcb_randr_connection_t m_connected;
    bool m_primary;
    KScreen::Output::Type m_type;

    XRandRMode::Map m_modes;
    QStringList m_preferredModes;

    QList<xcb_randr_output_t> m_clones;

    unsigned int m_widthMm;
    unsigned int m_heightMm;

    bool m_hotplugModeUpdate = false;
    XRandRCrtc *m_crtc;
};

Q_DECLARE_METATYPE(XRandROutput::Map)
