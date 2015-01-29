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

#include "xlibandxrandr.h"
#include "xrandrmode.h"
#include "output.h"

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
    typedef QMap<int, XRandROutput*> Map;

    explicit XRandROutput(RROutput id, XRandRConfig *config);
    virtual ~XRandROutput();

    void disabled();
    void disconnected();

    void update();
    void update(RRCrtc crtc, RRMode mode, Connection conn, bool primary);

    RROutput id() const;
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
    void outputRemoved(int id);

private:
    void init();
    void updateModes(const XRROutputInfo *outputInfo);

    static KScreen::Output::Type fetchOutputType(RROutput outputId, const QString &name);
    static KScreen::Output::Type typeFromName(const QString &name);
    static QByteArray typeFromProperty(RROutput outputId);

    XRandRConfig *m_config;
    RROutput m_id;
    QString m_name;
    Connection m_connected;
    KScreen::Output::Type m_type;
    QString m_icon;
    XRandRMode::Map m_modes;
    QStringList m_preferredModes;
    bool m_primary;
    QList<int> m_clones;
    mutable QByteArray m_edid;
    unsigned int m_widthMm;
    unsigned int m_heightMm;
    XRandRCrtc *m_crtc;
};

Q_DECLARE_METATYPE(XRandROutput::Map)

#endif // XRANDROUTPUT_H
