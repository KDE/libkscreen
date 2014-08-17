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

#include <QtCore/QRect>
#include <QGuiApplication>
#include <QScreen>

using namespace KScreen;

QScreenConfig::QScreenConfig(QObject *parent)
    : QObject(parent)
    , m_screen(new QScreenScreen(this))
    , m_blockSignals(true)
{
    foreach(const QScreen * qscreen, QGuiApplication::screens()) {
        screenAdded(qscreen);
    }
    m_blockSignals = false;
    connect(qApp, &QGuiApplication::screenAdded, this, &QScreenConfig::screenAdded);
}

QScreenConfig::~QScreenConfig()
{
    foreach (auto output, m_outputMap.values()) {
        delete output;
    }
}

Config* QScreenConfig::toKScreenConfig() const
{
    Config *config = new Config();
    config->setScreen(m_screen->toKScreenScreen(config));
    updateKScreenConfig(config);
    return config;
}

int QScreenConfig::outputId(const QScreen* qscreen)
{
    QList<int> ids;
    foreach (auto output, m_outputMap.values()) {
        if (qscreen == output->qscreen()) {
            return output->id();
        }
    }
    m_lastOutputId++;
    return m_lastOutputId;
}

void QScreenConfig::screenAdded(const QScreen* qscreen)
{
    qCDebug(KSCREEN_QSCREEN) << "Screen added" << qscreen << qscreen->name();
    QScreenOutput *qscreenoutput = new QScreenOutput(qscreen, this);
    qscreenoutput->setId(outputId(qscreen));
    m_outputMap.insert(qscreenoutput->id(), qscreenoutput);

    connect(qscreen, &QObject::destroyed, this, &QScreenConfig::screenDestroyed);

    if (!m_blockSignals) {
        KScreen::ConfigMonitor::instance()->notifyUpdate();
    }
}

void QScreenConfig::screenDestroyed(QObject* qscreen)
{
    qCDebug(KSCREEN_QSCREEN) << "Screen removed" << qscreen << QGuiApplication::screens().count();
    // Find output matching the QScreen object and remove it
    int removedOutputId = -1;
    foreach (auto output, m_outputMap.values()) {
        if (output->qscreen() == qscreen) {
            removedOutputId = output->id();
            m_outputMap.remove(removedOutputId);
            delete output;
        }
    }
    KScreen::ConfigMonitor::instance()->notifyUpdate();
}

void QScreenConfig::updateKScreenConfig(Config* config) const
{
    m_screen->updateKScreenScreen(config->screen());

    //Removing removed outputs
    KScreen::OutputList outputs = config->outputs();
    Q_FOREACH(KScreen::Output *output, outputs) {
        if (!m_outputMap.keys().contains(output->id())) {
            config->removeOutput(output->id());
        }
    }

    // Add KScreen::Outputs that aren't in the list yet, handle primaryOutput
    foreach(auto output, m_outputMap.values()) {

        KScreen::Output *kscreenOutput = config->output(output->id());

        if (!kscreenOutput) {
            kscreenOutput = output->toKScreenOutput(config);
            config->addOutput(kscreenOutput);
        }
        output->updateKScreenOutput(kscreenOutput);
        if (QGuiApplication::primaryScreen() == output->qscreen()) {
            config->setPrimaryOutput(kscreenOutput);
        }
    }
}

QMap< int, QScreenOutput * > QScreenConfig::outputMap() const
{
    return m_outputMap;
}

#include "qscreenconfig.moc"
