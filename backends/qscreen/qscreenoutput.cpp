/*************************************************************************************
 *  Copyright 2014 Sebastian Kügler <sebas@kde.org>                                  *
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

#include "qscreenoutput.h"
#include <mode.h>
#include <edid.h>

#include <QtCore/QRect>

#include <QGuiApplication>
#include <QScreen>



using namespace KScreen;

static int s_kscreenqscreenbackendOutputId = -1;

int getOutputId()
{
    s_kscreenqscreenbackendOutputId++;
    return s_kscreenqscreenbackendOutputId;
}


QScreenOutput::QScreenOutput(const QScreen *qscreen, QObject* parent)
    : Output(parent)
    , m_qscreen(qscreen)
    , m_edid(0)

{
    updateFromQScreen(m_qscreen);
}

QScreenOutput::~QScreenOutput()
{
}

void QScreenOutput::updateFromQScreen(const QScreen *qscreen)
{

    // Initialize primary output
    setId(getOutputId());
    setName(qscreen->name());
    setEnabled(true);
    setConnected(true);
    setPrimary(QGuiApplication::primaryScreen() == qscreen);

    // FIXME: Rotation

    // Physical size
    QSize mm;
    qreal physicalWidth;
    physicalWidth = qscreen->size().width() / (qscreen->physicalDotsPerInchX() / 25.4);
    mm.setWidth(qRound(physicalWidth));
    qreal physicalHeight;
    physicalHeight = qscreen->size().height() / (qscreen->physicalDotsPerInchY() / 25.4);
    mm.setHeight(qRound(physicalHeight));
    setSizeMm(mm);

    // Modes: we create a single default mode and go with that
    Mode* mode = new Mode(this);
    const QString modeid = QStringLiteral("defaultmode");
    mode->setId(modeid);
    mode->setRefreshRate(qscreen->refreshRate());
    mode->setSize(qscreen->size());

    setCurrentModeId(modeid);

    const QString modename = QString::number(qscreen->size().width()) + QStringLiteral("x") + QString::number(qscreen->size().height()) + QStringLiteral("@") + QString::number(qscreen->refreshRate());
    mode->setName(modename);

    ModeList modes;
    modes[modeid] = mode;
    setModes(modes);
}

KScreen::Edid* QScreenOutput::edid() const
{
    qCDebug(KSCREEN_QSCREEN) << "NEWING";
    if (!m_edid) {
        qCDebug(KSCREEN_QSCREEN) << "NEWING FOR REALZ";
        m_edid = new KScreen::Edid(0, 0, 0);
    }

    return m_edid;
}




#include "qscreenoutput.moc"
