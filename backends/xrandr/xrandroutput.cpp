/*************************************************************************************
 *  Copyright (C) 2012 by Alejandro Fiestas Olivares <afiestas@kde.org>              *
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

#include "xrandroutput.h"
#include "xrandrmode.h"
#include "xrandrconfig.h"
#include "xrandr.h"
#include "output.h"
#include "config.h"
#include "edid.h"

#include <QRect>

Q_DECLARE_METATYPE(QList<int>)

XRandROutput::XRandROutput(int id, bool primary, XRandRConfig *config)
    : QObject(config)
    , m_id(id)
    , m_type("unknown")
    , m_rotation(KScreen::Output::None)
    , m_currentMode(0)
    , m_edid(0)
    , m_changedProperties(0)
{
    XRROutputInfo *outputInfo = XRandR::XRROutput(m_id);
    updateOutput(outputInfo);
    updateModes(outputInfo);
    m_primary = primary;

    XRRFreeOutputInfo(outputInfo);
}


XRandROutput::~XRandROutput()
{
    delete m_edid;
}

int XRandROutput::id() const
{
    return m_id;
}

bool XRandROutput::isConnected() const
{
    return m_connected;
}

bool XRandROutput::isEnabled() const
{
    return m_enabled;
}

bool XRandROutput::isPrimary() const
{
    return m_primary;
}

QPoint XRandROutput::position() const
{
    return m_position;
}

int XRandROutput::currentModeId() const
{
    return m_currentMode;
}

KScreen::Output::Rotation XRandROutput::rotation() const
{
    return m_rotation;
}

KScreen::Edid *XRandROutput::edid() const
{
    if (!m_edid) {
        size_t len;
        quint8 *data = XRandR::outputEdid(m_id, len);
        if (data) {
            m_edid = new KScreen::Edid(data, len, 0);
            delete data;
        } else {
            m_edid = new KScreen::Edid(0, 0, 0);
        }
    }

    return m_edid;
}

void XRandROutput::update(PrimaryChange primary)
{
    XRROutputInfo *outputInfo = XRandR::XRROutput(m_id);

    m_changedProperties = 0;
    updateOutput(outputInfo);

    if (primary != NoChange) {
        bool setPrimary = (primary == SetPrimary);
        if (m_primary != setPrimary) {
            m_primary = setPrimary;
            m_changedProperties |= PropertyPrimary;
        }
    }

    if (m_changedProperties == 0) {
        m_changedProperties = PropertyNone;
    }

    XRRFreeOutputInfo(outputInfo);
}

void XRandROutput::updateOutput(const XRROutputInfo *outputInfo)
{
    bool isConnected = (outputInfo->connection == RR_Connected);

    if (m_name != outputInfo->name) {
        m_name = outputInfo->name;
        m_changedProperties |= PropertyName;
    }

    if (m_enabled != (outputInfo->crtc != None)) {
        m_enabled = outputInfo->crtc != None;
        m_changedProperties |= PropertyEnabled;
    }

    QList<int> clones;
    for (int i = 0; i < outputInfo->nclone; i++) {
        clones << outputInfo->clones[i];
    }
    if (isConnected && (m_clones != clones)) {
        m_clones = clones;
        m_changedProperties |= PropertyClones;
    }

    /* Don't update modes on disconnected output */
    if (isConnected && (outputInfo->crtc)) {
        XRRCrtcInfo* crtcInfo = XRandR::XRRCrtc(outputInfo->crtc);
        QRect rect;
        rect.setRect(crtcInfo->x, crtcInfo->y, crtcInfo->width, crtcInfo->height);
        if (m_position != rect.topLeft()) {
            m_position = rect.topLeft();
            m_changedProperties |= PropertyPos;
        }

        if (crtcInfo->mode) {
            if (m_currentMode != (int) crtcInfo->mode) {
                m_currentMode = crtcInfo->mode;
                m_changedProperties |= PropertyCurrentMode;
            }

            if (m_rotation != crtcInfo->rotation) {
                m_rotation = (KScreen::Output::Rotation) crtcInfo->rotation;
                m_changedProperties |= PropertyRotation;
            }
        }
        XRRFreeCrtcInfo(crtcInfo);
    }

    /* When an output is disconnected then force reset most properties */
    if (m_connected != isConnected) {
        m_connected = isConnected;
        if (!m_connected) {
            m_preferredModes.clear();
            qDeleteAll(m_modes);
            m_modes.clear();
            delete m_edid;
            m_changedProperties |= PropertyConnected | PropertyModes | PropertyEdid | PropertyPreferredMode;
        } else {
            updateModes(outputInfo);
            m_changedProperties |= PropertyConnected | PropertyModes | PropertyPreferredMode;
        }
    }
}

void XRandROutput::updateModes(const XRROutputInfo *outputInfo)
{
    /* Init modes */
    XRRModeInfo* modeInfo;
    XRRScreenResources *resources = XRandR::screenResources();

    m_preferredModes.clear();
    for (int i = 0; i < outputInfo->nmode; ++i)
    {
        /* Resources->modes contains all possible modes, we are only interested
         * in those listed in outputInfo->modes. */
        for (int j = 0; j < resources->nmode; ++j) {
            modeInfo = &resources->modes[j];
            if (modeInfo->id != outputInfo->modes[i]) {
                continue;
            }

            XRandRMode *mode = new XRandRMode(modeInfo, this);
            m_modes.insert(modeInfo->id, mode);

            if (i < outputInfo->npreferred) {
                m_preferredModes.append((int) modeInfo->id);
            }
        }
    }
    XRRFreeScreenResources(resources);
}


KScreen::Output *XRandROutput::toKScreenOutput(KScreen::Config *parent) const
{
    KScreen::Output *kscreenOutput = new KScreen::Output(parent);

    m_changedProperties = 0;
    kscreenOutput->setId(m_id);
    updateKScreenOutput(kscreenOutput);

    return kscreenOutput;
}

void XRandROutput::updateKScreenOutput(KScreen::Output *output) const
{
    if (!m_changedProperties || (m_changedProperties & PropertyName)) {
        output->setName(m_name);
    }

    if (!m_changedProperties || (m_changedProperties & PropertyType)) {
        output->setType(m_type);
    }

    if (!m_changedProperties || (m_changedProperties & PropertyIcon)) {
        output->setIcon(m_icon);
    }

    if (!m_changedProperties || (m_changedProperties & PropertyPos)) {
        output->setPos(m_position);
    }

    if (!m_changedProperties || (m_changedProperties & PropertyRotation)) {
        output->setRotation(m_rotation);
    }

    if (!m_changedProperties || (m_changedProperties & PropertyCurrentMode)) {
        output->setCurrentModeId(m_currentMode);
    }

    if (!m_changedProperties || (m_changedProperties & PropertyPreferredMode)) {
        output->setPreferredModes(m_preferredModes);
    }

    if (!m_changedProperties || (m_changedProperties & PropertyModes)) {
        output->setModes(KScreen::ModeList());
    }

    if (!m_changedProperties || (m_changedProperties & PropertyConnected)) {
        output->setConnected(m_connected);
    }

    if (!m_changedProperties || (m_changedProperties & PropertyEnabled)) {
        output->setEnabled(m_enabled);
    }

    if (!m_changedProperties || (m_changedProperties & PropertyPrimary)) {
        output->setPrimary(m_primary);
    }

    if (!m_changedProperties || (m_changedProperties & PropertyClones)) {
        output->setClones(m_clones);
    }

    if (!m_changedProperties || (m_changedProperties & PropertyModes)) {
        XRandRMode::Map::ConstIterator iter;
        KScreen::ModeList kscreenModes;
        for (iter = m_modes.constBegin(); iter != m_modes.constEnd(); ++iter) {
            XRandRMode *mode = iter.value();
            KScreen::Mode *kscreenMode = mode->toKScreenMode(output);
            kscreenModes.insert(iter.key(), kscreenMode);
        }
        output->setModes(kscreenModes);
    }
}

#include "xrandroutput.moc"
