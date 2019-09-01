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
#include "xrandrmode.h"

#include "mode.h"
#include "output.h"
#include "xrandroutput.h"

XRandRMode::XRandRMode(const xcb_randr_mode_info_t &modeInfo, XRandROutput *output)
    : QObject(output)
{
    m_id = modeInfo.id;
    // FIXME XCB
    //m_name = QString::fromUtf8(modeInfo->name);
    m_size = QSize(modeInfo.width, modeInfo.height);
    m_refreshRate = (float) modeInfo.dot_clock
                    / ((float) modeInfo.htotal * (float) modeInfo.vtotal);
}

XRandRMode::~XRandRMode()
{
}

KScreen::ModePtr XRandRMode::toKScreenMode()
{
    KScreen::ModePtr kscreenMode(new KScreen::Mode);

    kscreenMode->setId(QString::number(m_id));
    kscreenMode->setName(m_name);
    kscreenMode->setSize(m_size);
    kscreenMode->setRefreshRate(m_refreshRate);

    return kscreenMode;
}

xcb_randr_mode_t XRandRMode::id() const
{
    return m_id;
}

QSize XRandRMode::size() const
{
    return m_size;
}

float XRandRMode::refreshRate() const
{
    return m_refreshRate;
}

QString XRandRMode::name() const
{
    return m_name;
}
