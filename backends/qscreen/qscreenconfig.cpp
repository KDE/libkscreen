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

#include "qscreenconfig.h"
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
    screen->setId(0001);

    auto primary = QGuiApplication::primaryScreen();
    QSize _s = primary->availableVirtualGeometry().size();
    screen->setMinSize(_s);
    screen->setMaxSize(_s);
    screen->setCurrentSize(_s);
    screen->setMaxActiveOutputsCount(QGuiApplication::screens().count());

    OutputList outputList;

    foreach (const QScreen *qscreen, QGuiApplication::screens()) {

        qCDebug(KSCREEN_QSCREEN) << "New Output: " << qscreen->name();

        Output *output = new Output;
        qScreenToOutput(qscreen, output);


        if (output->isPrimary()) {
            setPrimaryOutput(output);
        }

        outputList.insert(output->id(), output);
    }


    setScreen(screen);
    setOutputs(outputList);
}

void QScreenConfig::qScreenToOutput(const QScreen *qscreen, Output* output) const
{

    // Initialize primary output
    output->setId(getId());
    output->setName(qscreen->name());
    output->setSizeMm(qscreen->size());
    output->setEnabled(true);
    output->setConnected(true);
    output->setPrimary(QGuiApplication::primaryScreen() == qscreen);

    // Rotation

    Mode* mode = new Mode;
    const QString modeid = QStringLiteral("14");
    mode->setId(modeid);
    mode->setRefreshRate(qscreen->refreshRate());
    mode->setSize(qscreen->size());

    output->setCurrentModeId(modeid);

    const QString modename = QString::number(qscreen->size().width()) + QStringLiteral("x") + QString::number(qscreen->size().height()) + QStringLiteral("@") + QString::number(qscreen->refreshRate());
    mode->setName(modename);

    ModeList modes;
    modes[modeid] = mode;
    output->setModes(modes);


    qCDebug(KSCREEN_QSCREEN) << "   Output.setCurrentSize: " << output->sizeMm() << modename;

}


#include "qscreenconfig.moc"













