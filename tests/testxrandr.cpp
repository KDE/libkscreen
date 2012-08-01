/*************************************************************************************
 *  Copyright (C) 2012 by Alejandro Fiestas Olivares <afiestas@kde.org>              *
 *                                                                                   *
 *  This program is free software; you can redistribute it and/or                    *
 *  modify it under the terms of the GNU General Public License                      *
 *  as published by the Free Software Foundation; either version 2                   *
 *  of the License, or (at your option) any later version.                           *
 *                                                                                   *
 *  This program is distributed in the hope that it will be useful,                  *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of                   *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                    *
 *  GNU General Public License for more details.                                     *
 *                                                                                   *
 *  You should have received a copy of the GNU General Public License                *
 *  along with this program; if not, write to the Free Software                      *
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA   *
 *************************************************************************************/

#define QT_GUI_LIB

#include <QtTest>
#include <QtCore/QObject>

#include "../src/kscreen.h"
#include "../src/config.h"
#include "../src/output.h"
#include "../src/mode.h"

class testXRandR : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void singleOutput();

private:
    QProcess m_process;
};

void testXRandR::initTestCase()
{
}

void testXRandR::singleOutput()
{
    setenv("KSCREEN_BACKEND", "XRandR", 1);

    KScreen *kscreen = KScreen::self();

    QVERIFY2(kscreen, KScreen::errorString().toLatin1());

    QVERIFY2(!kscreen->backend().isEmpty(), "No backend loaded");

    Config *config = kscreen->config();

    QCOMPARE(config->outputs().count(), 1);

    Output *output = config->outputs().take(1);

    QCOMPARE(output->name(), QString("default"));
    QCOMPARE(output->type(), QString("unknown"));
    QCOMPARE(output->modes().count(), 15);
    QCOMPARE(output->pos(), QPoint(0, 0));
    QCOMPARE(output->currentMode(), 338);
    QCOMPARE(output->rotation(), Output::None);
    QCOMPARE(output->isConnected(), true);
    QCOMPARE(output->isEnabled(), true);
    QCOMPARE(output->isPrimary(), false);
    QVERIFY2(output->clones().isEmpty(), "In singleOutput is impossible to have clones");
}

QTEST_MAIN(testXRandR)

#include "testxrandr.moc"