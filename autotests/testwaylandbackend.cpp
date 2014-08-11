/*************************************************************************************
 *  Copyright 2014 by Sebastian Kügler <sebas@kde.org>                           *
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

#define QT_GUI_LIB

#include <QtTest/QtTest>
#include <QtCore/QObject>

#include "../src/config.h"
#include "../src/output.h"
#include "../src/mode.h"
#include "../src/edid.h"

//Q_LOGGING_CATEGORY(KSCREEN_QSCREEN, "kscreen.qscreen");

using namespace KScreen;

class testWaylandBackend : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void verifyConfig();
    void cleanupTestCase();

private:
    QProcess m_process;
    Config *m_config;
    QString m_backend;
};

void testWaylandBackend::initTestCase()
{
   setenv("KSCREEN_BACKEND", "wayland", 1);
//     setenv("KSCREEN_BACKEND", "xrandr", 1);
    m_backend = qgetenv("KSCREEN_BACKEND").constData();

    m_config = Config::current();
}

void testWaylandBackend::verifyConfig()
{
    QVERIFY(m_config != 0);
    if (!m_config) {
        QSKIP("Wayland backend invalid", SkipAll);
    }
}

void testWaylandBackend::cleanupTestCase()
{
    delete m_config;
    qApp->exit(0);
}



QTEST_MAIN(testWaylandBackend)

#include "testwaylandbackend.moc"
