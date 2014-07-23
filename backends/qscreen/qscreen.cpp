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

#include "qscreen.h"
#include <configmonitor.h>

#include <QtCore/QFile>
#include <QtCore/qplugin.h>
#include <QtCore/QRect>
#include <QAbstractEventDispatcher>

#include <QX11Info>
#include <QGuiApplication>



using namespace KScreen;

Q_LOGGING_CATEGORY(KSCREEN_QSCREEN, "kscreen.qscreen");

QScreen::QScreen(QObject* parent)
    : QObject(parent)
    , m_isValid(false)
{
    QLoggingCategory::setFilterRules(QLatin1Literal("kscreen.xrandr.debug = true"));

    m_isValid = true; // We're cheating for now.
}

QScreen::~QScreen()
{


}

QString QScreen::name() const
{
    return QString("qscreen");
}

void QScreen::updateConfig()
{
    //s_internalConfig->update();
    KScreen::ConfigMonitor::instance()->notifyUpdate();
}

void QScreen::outputRemovedSlot()
{
    KScreen::ConfigMonitor::instance()->notifyUpdate();
}

Config* QScreen::config() const
{
    return 0;
}

void QScreen::setConfig(Config* config) const
{
    if (!config) {
        return;
    }

}

Edid *QScreen::edid(int outputId) const
{
    return 0;
    //return output->edid();
}

bool QScreen::isValid() const
{
    return m_isValid;
}

void QScreen::updateConfig(Config *config) const
{
    //Q_ASSERT(config != 0);

}


#include "qscreen.moc"
