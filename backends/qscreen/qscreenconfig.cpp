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

#include "qscreenconfig.h"
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


QScreenConfig::QScreenConfig(QObject* parent)
    : Config(parent)

{
    QLoggingCategory::setFilterRules(QLatin1Literal("kscreen.xrandr.debug = true"));
    updateConfig();
}

QScreenConfig::~QScreenConfig()
{
}

void QScreenConfig::updateConfig()
{
    Screen* screen = new Screen(this);
    screen->setId(0001); // FIXME

    auto primary = QGuiApplication::primaryScreen();
    QSize _s = primary->availableVirtualGeometry().size();
    screen->setMinSize(_s);
    screen->setMaxSize(_s);
    screen->setCurrentSize(_s);
    screen->setMaxActiveOutputsCount(QGuiApplication::screens().count());

    OutputList outputList;

    foreach (const QScreen *qscreen, QGuiApplication::screens()) {

        qCDebug(KSCREEN_QSCREEN) << "New Output: " << qscreen->name();

        QScreenOutput *output = new QScreenOutput(qscreen);

        if (output->isPrimary()) {
            setPrimaryOutput(output);
        }

        outputList.insert(output->id(), output);
        m_outputMap.insert(output->id(), output);
    }


    setScreen(screen);
    setOutputs(outputList);
}


QMap< int, QScreenOutput* > QScreenConfig::outputMap() const
{
    return m_outputMap;
}


#include "qscreenconfig.moc"













