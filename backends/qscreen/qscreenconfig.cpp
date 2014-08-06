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
#include "qscreenscreen.h"
#include "qscreenbackend.h"

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

QScreenConfig::QScreenConfig(QObject *parent)
    : QObject(parent)
    , m_screen(new QScreenScreen(this))

{
    updateOutputsInternal();
}

QScreenConfig::~QScreenConfig()
{
}

Config* QScreenConfig::toKScreenConfig() const
{
    Config *config = new Config();
    // FIXME: set outputs
    updateKScreenConfig(config);
    config->setScreen(m_screen->toKScreenScreen(config));
    // FIXME: set primary
    return config;
}

void QScreenConfig::updateOutputsInternal()
{
    foreach (auto output, m_outputMap.values()) {
        delete output;
    }
    m_outputMap.clear();

    OutputList outputList;
    foreach(const QScreen * qscreen, QGuiApplication::screens()) {

        qCDebug(KSCREEN_QSCREEN) << "New Output: " << qscreen->name();

        QScreenOutput *qscreenoutput = new QScreenOutput(qscreen, this);
        m_outputMap.insert(qscreenoutput->id(), qscreenoutput);
    }

}


void QScreenConfig::updateKScreenConfig(Config* config) const
{
//     foreach (auto output, m_outputMap.values()) {
//         delete output;
//     }
//     m_outputMap.clear();

    OutputList outputList;
    foreach(auto qscreenoutput, m_outputMap.values()) {

        //qCDebug(KSCREEN_QSCREEN) << "New Output: " << qscreen->name();

        Output *output = qscreenoutput->toKScreenOutput(config);

        if (QGuiApplication::primaryScreen() == qscreenoutput->qscreen()) {
            config->setPrimaryOutput(output);

        }

        outputList.insert(output->id(), output);
    }
    config->setOutputs(outputList);
}

QMap< int, QScreenOutput * > QScreenConfig::outputMap() const
{
    return m_screen->outputMap();
}

#include "qscreenconfig.moc"

