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

#include <QtTest>
#include <QtCore/QObject>

class testScreenConfig : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void singleOutput();
};

void testScreenConfig::singleOutput()
{
//     KScreen *kscreen = KScreen::self();
//
//     QVERIFY2(kscreen, ScreenManagement::error() + ScreenManagement::errorString());
//
//     ScreenConfig *config = kscreen->config();
//
//     QVERIFY2(config->backend().isEmpty());
//
//     QCOMPARE(config->outputs().count(), 1);
//
//     ScreenOutput *output = config->outputs().first();
//
//     QCOMPARE(output->name(), "FAKE1");
//     QCOMPARE(output->modes(), 10);
//     QCOMPARE(output->currentMode(), 9);
//     QCOMPARE(output->currentMode(), 9);
//     QCOMPARE(output->pos(), QPoint(0, 0));
//     QCOMPARE(output->size(), QPoint(1280, 800));
//     QCOMPARE(output->rotation(), ScreenManagement::Normal);
//     QCOMPARE(output->isConnected(), true);
//     QCOMPARE(output->isEnabled(), true);
//     QCOMPARE(output->isPrimary(), true);
//     QCOMPARE(output->clones(), 0);

}


QTEST_MAIN(testScreenConfig)

#include "config.moc"