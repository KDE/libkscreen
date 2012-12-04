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


#include "xrandrmode.h"
#include "xrandroutput.h"
#include "mode.h"
#include "output.h"

XRandRMode::XRandRMode(XRRModeInfo *modeInfo, XRandROutput *output)
    : QObject(output)
{
    setModeProperty(PropertyId, (int) modeInfo->id);
    setModeProperty(PropertyName, QString::fromUtf8(modeInfo->name));
    setModeProperty(PropertySize, QSize(modeInfo->width, modeInfo->height));
    setModeProperty(PropertyRefreshRate, ((float) modeInfo->dotClock / ((float) modeInfo->hTotal * (float) modeInfo->vTotal)));
}


XRandRMode::~XRandRMode()
{
}

void XRandRMode::setModeProperty(XRandRMode::Property id, const QVariant &value)
{
    m_properties.insert(id, value);
}

QVariant XRandRMode::modeProperty(XRandRMode::Property id) const
{
    return m_properties.value(id);
}

KScreen::Mode *XRandRMode::toKScreenMode(KScreen::Output *parent)
{
    KScreen::Mode *kscreenMode = new KScreen::Mode(parent);

    kscreenMode->setId(m_properties.value(PropertyId).toInt());
    kscreenMode->setName(m_properties.value(PropertyName).toString());
    kscreenMode->setSize(m_properties.value(PropertySize).toSize());
    kscreenMode->setRefreshRate(m_properties.value(PropertyRefreshRate).toFloat());

    return kscreenMode;
}

#include "xrandrmode.moc"
