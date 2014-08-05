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

#include "qscreenscreen.h"
#include "qscreenoutput.h"

#include <configmonitor.h>
#include <mode.h>

#include <QtCore/QFile>
#include <QtCore/qplugin.h>
#include <QtCore/QRect>
#include <QAbstractEventDispatcher>

#include <QX11Info>
#include <QGuiApplication>
#include <QScreen>

using namespace KScreen;

static int s_kscreenqscreenbackendScreenId = -1;

int getId()
{
    s_kscreenqscreenbackendScreenId++;
    return s_kscreenqscreenbackendScreenId;
}

QScreenScreen::QScreenScreen(Config *config)
    : Screen(config)
    , m_config(config)

{
    QLoggingCategory::setFilterRules(QLatin1Literal("kscreen.xrandr.debug = true"));
    updateConfig();
    connect(qApp, &QGuiApplication::screenAdded, this, &QScreenScreen::screenAdded);
}

QScreenScreen::~QScreenScreen()
{
}

void QScreenScreen::screenAdded(QScreen* qscreen)
{
    qCDebug(KSCREEN_QSCREEN) << "Screen added!!! Updating config..";
    updateConfig(); // FIXME: We want to be a bit smarter here.
}


void QScreenScreen::updateConfig()
{
    setId(getId());

    auto primary = QGuiApplication::primaryScreen();
    QSize _s = primary->availableVirtualGeometry().size();
    setMinSize(_s);
    setMaxSize(_s);
    setCurrentSize(_s);
    setMaxActiveOutputsCount(QGuiApplication::screens().count());

    OutputList outputList;

    foreach(const QScreen * qscreen, QGuiApplication::screens()) {

        qCDebug(KSCREEN_QSCREEN) << "New Output: " << qscreen->name();

        QScreenOutput *output = new QScreenOutput(qscreen);

        if (output->isPrimary()) {
            m_config->setPrimaryOutput(output);

        }

        outputList.insert(output->id(), output);
        m_outputMap.insert(output->id(), output);
    }

    m_config->setOutputs(outputList);
}

QMap< int, QScreenOutput * > QScreenScreen::outputMap() const
{
    return m_outputMap;
}

#include "qscreenscreen.moc"

