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
    m_id = modeInfo->id;
    m_name = QString::fromUtf8(modeInfo->name);
    m_size = QSize(modeInfo->width, modeInfo->height);
    m_refreshRate = ((float) modeInfo->dotClock / ((float) modeInfo->hTotal * (float) modeInfo->vTotal));
}


XRandRMode::~XRandRMode()
{
}

KScreen::Mode *XRandRMode::toKScreenMode(KScreen::Output *parent)
{
    KScreen::Mode *kscreenMode = new KScreen::Mode(parent);

    kscreenMode->setId(QString::number(m_id));
    kscreenMode->setName(m_name);
    kscreenMode->setSize(m_size);
    kscreenMode->setRefreshRate(m_refreshRate);

    return kscreenMode;
}

QSize XRandRMode::size() const
{
    return m_size;
}

#include "xrandrmode.moc"
