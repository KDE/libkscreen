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

#include <QRect>

Q_DECLARE_METATYPE(QList<int>)

XRandROutput::XRandROutput(int id, XRandRConfig *config)
    : QObject(config)
    , m_id(id)
{
    XRROutputInfo *outputInfo = XRandR::XRROutput(m_id);
    updateOutput(outputInfo);

    /* Init modes */
    XRandRMode::Map modes;
    XRRModeInfo* modeInfo;
    XRRScreenResources *resources = XRandR::screenResources();
    for (int i = 0; i < outputInfo->nmode; ++i)
    {
        modeInfo = &resources->modes[i];
        XRandRMode *mode = new XRandRMode(modeInfo, this);
        modes.insert(modeInfo->id, mode);
    }
    setOutputProperty(XRandROutput::PropertyModes, QVariant::fromValue(modes));

    XRRFreeOutputInfo(outputInfo);
}


XRandROutput::~XRandROutput()
{
}

void XRandROutput::setOutputProperty(XRandROutput::Property id, const QVariant &value)
{
    m_properties.insert(id, value);
}

QVariant XRandROutput::outputProperty(XRandROutput::Property id)
{
    return m_properties.value(id);
}

void XRandROutput::update()
{
    XRROutputInfo *outputInfo = XRandR::XRROutput(m_id);

    updateOutput(outputInfo);

    /* FIXME: Can modes change? */

    XRRFreeOutputInfo(outputInfo);
}

void XRandROutput::updateOutput(const XRROutputInfo *outputInfo)
{
    setOutputProperty(XRandROutput::PropertyId, (int) m_id);
    setOutputProperty(XRandROutput::PropertyName, outputInfo->name);
    setOutputProperty(XRandROutput::PropertyConnected, outputInfo->connection == RR_Connected);
    setOutputProperty(XRandROutput::PropertyEnabled, outputInfo->crtc != None);
    setOutputProperty(XRandROutput::PropertyType, "unknown");
    setOutputProperty(XRandROutput::PropertyRotation, (int) KScreen::Output::None);

    if (outputInfo->crtc) {
        XRRCrtcInfo* crtcInfo = XRandR::XRRCrtc(outputInfo->crtc);
        QRect rect;
        rect.setRect(crtcInfo->x, crtcInfo->y, crtcInfo->width, crtcInfo->height);
        setOutputProperty(XRandROutput::PropertyPos, rect.topLeft());

        if (crtcInfo->mode) {
            setOutputProperty(XRandROutput::PropertyCurrentMode, (int) crtcInfo->mode);
        }
    }
}



KScreen::Output *XRandROutput::toKScreenOutput(KScreen::Config *parent) const
{
    KScreen::Output *kscreenOutput = new KScreen::Output(parent);

    kscreenOutput->setId(m_properties.value(PropertyId).toInt());
    kscreenOutput->setName(m_properties.value(PropertyName).toString());
    kscreenOutput->setType(m_properties.value(PropertyType).toString());
    kscreenOutput->setIcon(m_properties.value(PropertyIcon).toString());
    kscreenOutput->setPos(m_properties.value(PropertyPos).toPoint());
    kscreenOutput->setRotation((KScreen::Output::Rotation) m_properties.value(PropertyRotation).toInt());
    kscreenOutput->setCurrentMode(m_properties.value(PropertyCurrentMode).toInt());
    kscreenOutput->setConnected(m_properties.value(PropertyConnected).toBool());
    kscreenOutput->setEnabled(m_properties.value(PropertyEnabled).toBool());
    kscreenOutput->setPrimary(m_properties.value(PropertyPrimary).toBool());
    kscreenOutput->setClones(m_properties.value(PropertyClones).value< QList<int> >());

    XRandRMode::Map modes = m_properties.value(PropertyModes).value<XRandRMode::Map>();
    KScreen::ModeList kscreenModes;
    XRandRMode::Map::ConstIterator iter;
    for (iter = modes.constBegin(); iter != modes.constEnd(); iter++) {
        XRandRMode *mode = iter.value();
        KScreen::Mode *kscreenMode = mode->toKScreenMode(kscreenOutput);
        kscreenModes.insert(iter.key(), kscreenMode);
    }
    kscreenOutput->setModes(kscreenModes);

    return kscreenOutput;
}


#include "xrandroutput.moc"
