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

#include <QtTest/QtTest>
#include <QtCore/QObject>

#include "../src/screen.h"
#include "../src/config.h"
#include "../src/output.h"
#include "../src/mode.h"

using namespace KScreen;

class testScreenConfig : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void singleOutput();
    void singleOutputWithoutPreferred();
    void multiOutput();
    void clonesOutput();
    void configCanBeApplied();
};

void testScreenConfig::initTestCase()
{
    setenv("KSCREEN_BACKEND", "Fake", 1);
}

void testScreenConfig::singleOutput()
{
    //json file for the fake backend
    QByteArray path(TEST_DATA);
    path.append("/singleoutput.json");
    setenv("TEST_DATA", path, 1);

//     QVERIFY2(kscreen, KScreen::errorString().toLatin1());

//     QVERIFY2(!kscreen->backend().isEmpty(), "No backend loaded");

    Config *config = Config::current();

    Screen* screen = config->screen();

    QCOMPARE(screen->minSize(), QSize(320, 200));
    QCOMPARE(screen->maxSize(), QSize(8192, 8192));
    QCOMPARE(screen->currentSize(), QSize(1280, 800));

    QCOMPARE(config->outputs().count(), 1);

    Output *output = config->outputs().take(1);

    QCOMPARE(output->name(), QString("LVDS1"));
    QCOMPARE(output->type(), Output::Panel);
    QCOMPARE(output->modes().count(), 3);
    QCOMPARE(output->pos(), QPoint(0, 0));
    QCOMPARE(output->currentModeId(), QLatin1String("3"));
    QCOMPARE(output->preferredModeId(), QLatin1String("3"));
    QCOMPARE(output->rotation(), Output::None);
    QCOMPARE(output->isConnected(), true);
    QCOMPARE(output->isEnabled(), true);
    QCOMPARE(output->isPrimary(), true);
    //QCOMPARE(output->isEmbedded(), true);
    QVERIFY2(output->clones().isEmpty(), "In singleOutput is impossible to have clones");

    Mode* mode = output->currentMode();
    QCOMPARE(mode->size(), QSize(1280, 800));
    QCOMPARE(mode->refreshRate(), (float)59.9);
}

void testScreenConfig::singleOutputWithoutPreferred()
{
    QByteArray path(TEST_DATA);
    path.append("/singleOutputWithoutPreferred.json");
    setenv("TEST_DATA", path, 1);

    Config* config = Config::current();
    Output* output = config->outputs().take(1);

    QVERIFY(output->preferredModes().isEmpty());
    QCOMPARE(output->preferredModeId(), QLatin1String("3"));
}

void testScreenConfig::multiOutput()
{
    QByteArray path(TEST_DATA);
    path.append("/multipleoutput.json");
    setenv("TEST_DATA", path, 1);

    Config *config = Config::current();

    Screen* screen = config->screen();

    QCOMPARE(screen->minSize(), QSize(320, 200));
    QCOMPARE(screen->maxSize(), QSize(8192, 8192));
    QCOMPARE(screen->currentSize(), QSize(3200, 1880));

    QCOMPARE(config->outputs().count(), 2);

    Output *output = config->outputs().take(2);

    QCOMPARE(output->name(), QString("HDMI1"));
    QCOMPARE(output->type(), Output::HDMI);
    QCOMPARE(output->modes().count(), 4);
    QCOMPARE(output->pos(), QPoint(0, 0));
    QCOMPARE(output->currentModeId(), QLatin1String("4"));
    QCOMPARE(output->preferredModeId(), QLatin1String("4"));
    QCOMPARE(output->rotation(), Output::None);
    QCOMPARE(output->isConnected(), true);
    QCOMPARE(output->isEnabled(), true);
    QCOMPARE(output->isPrimary(), false);
    QVERIFY2(output->clones().isEmpty(), "This simulates extended output, no clones");

    Mode* mode = output->currentMode();
    QCOMPARE(mode->size(), QSize(1920, 1080));
    QCOMPARE(mode->refreshRate(), (float)60.0);
}

void testScreenConfig::clonesOutput()
{
    QByteArray path(TEST_DATA);
    path.append("/multipleclone.json");
    setenv("TEST_DATA", path, 1);

    Config *config = Config::current();
    Screen* screen = config->screen();

    QCOMPARE(screen->minSize(), QSize(320, 200));
    QCOMPARE(screen->maxSize(), QSize(8192, 8192));
    QCOMPARE(screen->currentSize(), QSize(1024, 768));

    Output* one = config->outputs()[1];
    Output* two = config->outputs()[2];

    QCOMPARE(one->currentMode()->size(), two->currentMode()->size());
    QCOMPARE(one->clones().count(), 1);
    QCOMPARE(one->clones().first(), two->id());
    QVERIFY2(two->clones().isEmpty(), "Output two should have no clones");
}

void testScreenConfig::configCanBeApplied()
{
    QByteArray path(TEST_DATA);
    path.append("/singleoutputBroken.json");
    setenv("TEST_DATA", path, 1);
    Config* brokenConfig = Config::current();

    path = TEST_DATA;
    path.append("/singleoutput.json");
    setenv("TEST_DATA", path, 1);
    Config* currentConfig = Config::current();

    Output* primaryBroken = brokenConfig->outputs()[2];
    Output* currentPrimary = currentConfig->outputs()[1];

    QVERIFY(!Config::canBeApplied(brokenConfig));
    primaryBroken->setId(currentPrimary->id());
    QVERIFY(!Config::canBeApplied(brokenConfig));
    primaryBroken->setConnected(currentPrimary->isConnected());
    QVERIFY(!Config::canBeApplied(brokenConfig));
    primaryBroken->setCurrentModeId(QLatin1String("42"));
    QVERIFY(!Config::canBeApplied(brokenConfig));
    primaryBroken->setCurrentModeId(currentPrimary->currentModeId());
    QVERIFY(!Config::canBeApplied(brokenConfig));
    primaryBroken->mode(QLatin1String("3"))->setSize(QSize(1280, 800));
    QVERIFY(Config::canBeApplied(brokenConfig));
}

QTEST_MAIN(testScreenConfig)

#include "testscreenconfig.moc"
