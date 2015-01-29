/*************************************************************************************
 *  Copyright (C) 2012 by Alejandro Fiestas Olivares <afiestas@kde.org>              *
 *  Copyright (C) 2012, 2013 by Daniel Vrátil <dvratil@redhat.com>                   *
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

#include "xrandroutput.h"
#include "xrandrmode.h"
#include "xrandrconfig.h"
#include "xrandr.h"
#include "output.h"
#include "config.h"

#include <QRect>

Q_DECLARE_METATYPE(QList<int>)

XRandROutput::XRandROutput(RROutput id, XRandRConfig *config)
    : QObject(config)
    , m_config(config)
    , m_id(id)
    , m_type(KScreen::Output::Unknown)
    , m_primary(0)
    , m_crtc(0)
{
    init();
}

XRandROutput::~XRandROutput()
{
}

RROutput XRandROutput::id() const
{
    return m_id;
}

bool XRandROutput::isConnected() const
{
    return m_connected == RR_Connected;
}

bool XRandROutput::isEnabled() const
{
    return m_crtc != Q_NULLPTR && m_crtc->mode() != None;
}

bool XRandROutput::isPrimary() const
{
    return m_primary;
}

QPoint XRandROutput::position() const
{
    return m_crtc ? m_crtc->geometry().topLeft() : QPoint();
}

XRandRMode::Map XRandROutput::modes() const
{
    return m_modes;
}

QString XRandROutput::currentModeId() const
{
    return m_crtc ? QString::number(m_crtc->mode()) : QString();
}

XRandRMode* XRandROutput::currentMode() const
{
    if (!m_crtc) {
        return Q_NULLPTR;
    }
    int modeId = m_crtc->mode();
    if (!m_modes.contains(modeId)) {
        return 0;
    }

    return m_modes[modeId];
}

KScreen::Output::Rotation XRandROutput::rotation() const
{
    return static_cast<KScreen::Output::Rotation>(m_crtc ? m_crtc->rotation() : RR_Rotate_0);
}

QByteArray XRandROutput::edid() const
{
    if (m_edid.isNull()) {
        size_t len;
        quint8 *data = XRandR::outputEdid(m_id, len);
        if (data) {
            m_edid = QByteArray((char *) data, len);
            delete[] data;
        } else {
            m_edid = QByteArray();
        }
    }

    return m_edid;
}

XRandRCrtc* XRandROutput::crtc() const
{
    return m_crtc;
}

void XRandROutput::update()
{
    init();
}

void XRandROutput::update(RRCrtc crtc, RRMode mode, Connection conn, bool primary)
{
    qCDebug(KSCREEN_XRANDR) << "XRandROutput" << m_id << "update";
    qCDebug(KSCREEN_XRANDR) << "\tm_connected:" << m_connected;
    qCDebug(KSCREEN_XRANDR) << "\tm_crtc" << m_crtc;
    qCDebug(KSCREEN_XRANDR) << "\tCRTC:" << crtc;
    qCDebug(KSCREEN_XRANDR) << "\tMODE:" << mode;
    qCDebug(KSCREEN_XRANDR) << "\tConnection:" << conn;

    // Connected or disconnected
    if (isConnected() != (conn == RR_Connected)) {
        if (conn == RR_Connected) {
            // New monitor has been connected, refresh everything
          init();
        } else {
            // Disconnected
            m_connected = conn;
            m_clones.clear();
            m_heightMm = 0;
            m_widthMm = 0;
            m_type = KScreen::Output::Unknown;
            qDeleteAll(m_modes);
            m_modes.clear();
            m_preferredModes.clear();
        }
    }

    // A monitor has been enabled or disabled
    // We don't use isEnabled(), because it checks for crtc && crtc->mode(), however
    // crtc->mode may already be unset due to RRCrtcChangeNotify coming before
    // RROutputChangeNotify and reseting the CRTC mode

    if ((m_crtc == Q_NULLPTR) != (crtc == None)) {
        if (crtc == None && mode == None) {
            // Monitor has been disabled
            m_crtc->disconectOutput(m_id);
            m_crtc = 0;
        } else {
            m_crtc = m_config->crtc(crtc);
            m_crtc->connectOutput(m_id);
        }
    }

    // Primary has changed
    m_primary = primary;
}



void XRandROutput::init()
{
    XRROutputInfo *outputInfo = XRandR::XRROutput(m_id);
    Q_ASSERT(outputInfo);
    if (!outputInfo) {
        return;
    }

    RROutput primary = XRRGetOutputPrimary(XRandR::display(), XRandR::rootWindow());

    m_name = QString::fromUtf8(outputInfo->name, outputInfo->nameLen);
    m_type = fetchOutputType(m_id, m_name);
    m_icon = QString();
    m_connected = outputInfo->connection;
    m_primary = (primary == m_id);
    for (int i = 0; i < outputInfo->nclone; ++i) {
        m_clones.append(outputInfo->clones[i]);
    }
    m_widthMm = outputInfo->mm_width;
    m_heightMm = outputInfo->mm_height;
    m_crtc = m_config->crtc(outputInfo->crtc);
    if (m_crtc) {
        m_crtc->connectOutput(m_id);
    }

    updateModes(outputInfo);

    XRRFreeOutputInfo(outputInfo);
}

void XRandROutput::updateModes(const XRROutputInfo *outputInfo)
{
    /* Init modes */
    XRRScreenResources *resources = XRandR::screenResources();
    Q_ASSERT(resources);
    if (!resources) {
        return;
    }

    m_preferredModes.clear();
    qDeleteAll(m_modes);
    m_modes.clear();
    for (int i = 0; i < outputInfo->nmode; ++i) {
        /* Resources->modes contains all possible modes, we are only interested
         * in those listed in outputInfo->modes. */
        for (int j = 0; j < resources->nmode; ++j) {
            XRRModeInfo *modeInfo = &resources->modes[j];
            if (modeInfo->id != outputInfo->modes[i]) {
                continue;
            }

            XRandRMode *mode = new XRandRMode(modeInfo, this);
            m_modes.insert(modeInfo->id, mode);

            if (i < outputInfo->npreferred) {
                m_preferredModes.append(QString::number(modeInfo->id));
            }
        }
    }
    XRRFreeScreenResources(resources);
}

KScreen::Output::Type XRandROutput::fetchOutputType(RROutput outputId, const QString &name)
{
    const QByteArray type = typeFromProperty(outputId);
    if (type.isEmpty()) {
        return typeFromName(name);
    }

    if (type.contains("VGA")) {
        return KScreen::Output::VGA;
    } else if (type.contains("DVI")) {
        return KScreen::Output::DVI;
    } else if (type.contains("DVI-I")) {
        return KScreen::Output::DVII;
    } else if (type.contains("DVI-A")) {
        return KScreen::Output::DVIA;
    } else if (type.contains("DVI-D")) {
        return KScreen::Output::DVID;
    } else if (type.contains("HDMI")) {
        return KScreen::Output::HDMI;
    } else if (type.contains("Panel")) {
        return KScreen::Output::Panel;
    } else if (type.contains("TV")) {
        return KScreen::Output::TV;
    } else if (type.contains("TV-Composite")) {
        return KScreen::Output::TVComposite;
    } else if (type.contains("TV-SVideo")) {
        return KScreen::Output::TVSVideo;
    } else if (type.contains("TV-Component")) {
        return KScreen::Output::TVComponent;
    } else if (type.contains("TV-SCART")) {
        return KScreen::Output::TVSCART;
    } else if (type.contains("TV-C4")) {
        return KScreen::Output::TVC4;
    } else if (type.contains("DisplayPort")) {
        return KScreen::Output::DisplayPort;
    } else if (type.contains("unknown")) {
        return KScreen::Output::Unknown;
    } else {
//         qCDebug(KSCREEN_XRANDR) << "Output Type not translated:" << type;
    }

    return KScreen::Output::Unknown;

}

KScreen::Output::Type XRandROutput::typeFromName(const QString &name)
{
    static const QStringList embedded = QStringList() << QLatin1String("LVDS")
                                                      << QLatin1String("IDP")
                                                      << QLatin1String("EDP")
                                                      << QLatin1String("LCD");

    Q_FOREACH(const QString &pre, embedded) {
        if (name.toUpper().startsWith(pre)) {
            return KScreen::Output::Panel;
        }
    }

    return KScreen::Output::Unknown;
}

QByteArray XRandROutput::typeFromProperty(RROutput outputId)
{
    QByteArray type;

    const Atom atomType = XInternAtom(XRandR::display(), RR_PROPERTY_CONNECTOR_TYPE, True);
    if (atomType == None) {
        return type;
    }

    unsigned char *prop;
    int actualFormat;
    unsigned long nitems, bytes_after;
    Atom actualType;
    char *connectorType;

    if (XRRGetOutputProperty(XRandR::display(), outputId, atomType, 0, 100, False,
            False, AnyPropertyType, &actualType, &actualFormat, &nitems,
            &bytes_after, &prop) != Success) {

        return type;
    }

    if (!(actualType == XA_ATOM && actualFormat == 32 && nitems == 1)) {
        return type;
    }

    connectorType = XGetAtomName(XRandR::display(), *((Atom *) prop));
    if (!connectorType) {
        return type;
    }

    type = connectorType;
    XFree(connectorType);


    return type;
}

KScreen::OutputPtr XRandROutput::toKScreenOutput() const
{
    KScreen::OutputPtr kscreenOutput(new KScreen::Output);

    const bool signalsBlocked = kscreenOutput->signalsBlocked();
    kscreenOutput->blockSignals(true);
    kscreenOutput->setId(m_id);
    kscreenOutput->setType(m_type);
    kscreenOutput->setSizeMm(QSize(m_widthMm, m_heightMm));
    kscreenOutput->setName(m_name);
    kscreenOutput->setIcon(m_icon);

    kscreenOutput->setConnected(isConnected());
    if (isConnected()) {
        KScreen::ModeList kscreenModes;
        for (auto iter = m_modes.constBegin(), end = m_modes.constEnd(); iter != end; ++iter) {
            XRandRMode *mode = iter.value();
            kscreenModes.insert(QString::number(iter.key()), mode->toKScreenMode());
        }
        kscreenOutput->setModes(kscreenModes);
        kscreenOutput->setPreferredModes(m_preferredModes);
        kscreenOutput->setPrimary(m_primary);
        kscreenOutput->setClones(m_clones);

        kscreenOutput->setEnabled(isEnabled());
        if (isEnabled()) {
            kscreenOutput->setPos(position());
            kscreenOutput->setRotation(rotation());
            kscreenOutput->setCurrentModeId(currentModeId());
        }
    }


    kscreenOutput->blockSignals(signalsBlocked);
    return kscreenOutput;
}
