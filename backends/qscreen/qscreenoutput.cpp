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
#include "qscreenbackend.h"

#include <mode.h>
#include <edid.h>

#include <QtCore/QRect>

#include <QGuiApplication>
#include <QScreen>

using namespace KScreen;

// Book-keeping for ids, make sure we map the id to the right QScreen, and only once.
static int s_kscreenqscreenbackendOutputId = -1;
static QHash<const QScreen*, int> s_kscreenqscreenbackendOutputIdTable;

int getOutputId(const QScreen *qscreen)
{
    if (!s_kscreenqscreenbackendOutputIdTable.contains(qscreen)) {
        s_kscreenqscreenbackendOutputId++;
        s_kscreenqscreenbackendOutputIdTable.insert(qscreen, s_kscreenqscreenbackendOutputId);
    }
    return s_kscreenqscreenbackendOutputIdTable.value(qscreen);
}


QScreenOutput::QScreenOutput(const QScreen *qscreen, QObject *parent)
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

}

KScreen::Edid *QScreenOutput::fakeEdid()
{
    qCDebug(KSCREEN_QSCREEN) << "NEWING";
    if (!m_edid) {
        qCDebug(KSCREEN_QSCREEN) << "NEWING FOR REALZ";
        m_edid = new KScreen::Edid(0, 0, this);
    }

    return m_edid;
}

const QScreen* QScreenOutput::qscreen() const
{
    return m_qscreen;
}

Output* QScreenOutput::toKScreenOutput(Config* parent) const
{
    Output *output = new Output(parent);
    output->setId(getOutputId(m_qscreen));
    output->setName(m_qscreen->name());
    updateKScreenOutput(output);
    return output;
}

void QScreenOutput::updateKScreenOutput(Output* output) const
{
    // Initialize primary output
    output->setEnabled(true);
    output->setConnected(true);
    output->setPrimary(QGuiApplication::primaryScreen() == m_qscreen);

    // FIXME: Rotation

    // Physical size
    QSize mm;
    qreal physicalWidth;
    physicalWidth = m_qscreen->size().width() / (m_qscreen->physicalDotsPerInchX() / 25.4);
    mm.setWidth(qRound(physicalWidth));
    qreal physicalHeight;
    physicalHeight = m_qscreen->size().height() / (m_qscreen->physicalDotsPerInchY() / 25.4);
    mm.setHeight(qRound(physicalHeight));
    output->setSizeMm(mm);
    qCDebug(KSCREEN_QSCREEN) << "  ####### setSizeMm: " << mm;
    qCDebug(KSCREEN_QSCREEN) << "  ####### availableGeometry: " << m_qscreen->availableGeometry();
    output->setPos(m_qscreen->availableGeometry().topLeft());

    // Modes: we create a single default mode and go with that
    Mode *mode = new Mode(output);
    const QString modeid = QStringLiteral("defaultmode");
    mode->setId(modeid);
    mode->setRefreshRate(m_qscreen->refreshRate());
    mode->setSize(m_qscreen->size());


    const QString modename = QString::number(m_qscreen->size().width()) + QStringLiteral("x") + QString::number(m_qscreen->size().height()) + QStringLiteral("@") + QString::number(m_qscreen->refreshRate());
    mode->setName(modename);

    ModeList modes;
    modes[modeid] = mode;
    output->setModes(modes);
    output->setCurrentModeId(modeid);

}


#include "qscreenoutput.moc"
