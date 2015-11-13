/*************************************************************************************
 *  Copyright (C) 2012 by Alejandro Fiestas Olivares <afiestas@kde.org>              *
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

#include <QtTest/QtTest>
#include <QtCore/QObject>

#include "../src/screen.h"
#include "../src/config.h"
#include "../src/output.h"
#include "../src/mode.h"
#include "../src/configoperation.h"
#include "../src/backendmanager_p.h"

using namespace KScreen;

class testScreenConfig : public QObject
{
    Q_OBJECT

private:
    KScreen::ConfigPtr getConfig();

private Q_SLOTS:
    void initTestCase();
    void singleOutput();
    void singleOutputWithoutPreferred();
    void multiOutput();
    void clonesOutput();
    void configCanBeApplied();
    void cleanupTestCase();
};

ConfigPtr testScreenConfig::getConfig()
{
    setenv("KSCREEN_BACKEND_INPROCESS", "0", 1);
    auto *op = ConfigOperation::create();
    if (!op->exec()) {
        qWarning("ConfigOperation error: %s", qPrintable(op->errorString()));
        BackendManager::instance()->shutdownBackend();
        return ConfigPtr();
    }

    BackendManager::instance()->shutdownBackend();

    return op->config();
}


void testScreenConfig::initTestCase()
{
    setenv("KSCREEN_BACKEND", "Fake", 1);
}

void testScreenConfig::cleanupTestCase()
{
    BackendManager::instance()->shutdownBackend();
}

void testScreenConfig::singleOutput()
{
    //json file for the fake backend
    qputenv("KSCREEN_BACKEND_ARGS", "TEST_DATA=" TEST_DATA "singleoutput.json");

//     QVERIFY2(kscreen, KScreen::errorString().toLatin1());

//     QVERIFY2(!kscreen->backend().isEmpty(), "No backend loaded");



    const ConfigPtr config = getConfig();
    QVERIFY(!config.isNull());
    const ScreenPtr screen = config->screen();
    QVERIFY(!screen.isNull());

    QCOMPARE(screen->minSize(), QSize(320, 200));
    QCOMPARE(screen->maxSize(), QSize(8192, 8192));
    QCOMPARE(screen->currentSize(), QSize(1280, 800));

    QCOMPARE(config->outputs().count(), 1);

    const OutputPtr output = config->outputs().take(1);
    QVERIFY(!output.isNull());

    QCOMPARE(output->name(), QString("LVDS1"));
    QCOMPARE(output->type(), Output::Panel);
    QCOMPARE(output->modes().count(), 3);
    QCOMPARE(output->pos(), QPoint(0, 0));
    QCOMPARE(output->geometry(), QRect(0,0, 1280, 800));
    QCOMPARE(output->currentModeId(), QLatin1String("3"));
    QCOMPARE(output->preferredModeId(), QLatin1String("3"));
    QCOMPARE(output->rotation(), Output::None);
    QCOMPARE(output->isConnected(), true);
    QCOMPARE(output->isEnabled(), true);
    QCOMPARE(output->isPrimary(), true);
    //QCOMPARE(output->isEmbedded(), true);
    QVERIFY2(output->clones().isEmpty(), "In singleOutput is impossible to have clones");

    const ModePtr mode = output->currentMode();
    QCOMPARE(mode->size(), QSize(1280, 800));
    QCOMPARE(mode->refreshRate(), (float)59.9);
}

void testScreenConfig::singleOutputWithoutPreferred()
{
    qputenv("KSCREEN_BACKEND_ARGS", "TEST_DATA=" TEST_DATA "singleOutputWithoutPreferred.json");

    const ConfigPtr config = getConfig();
    QVERIFY(!config.isNull());
    const OutputPtr output = config->outputs().take(1);
    QVERIFY(!output.isNull());

    QVERIFY(output->preferredModes().isEmpty());
    QCOMPARE(output->preferredModeId(), QLatin1String("3"));
}

void testScreenConfig::multiOutput()
{
    qputenv("KSCREEN_BACKEND_ARGS", "TEST_DATA=" TEST_DATA "multipleoutput.json");

    const ConfigPtr config = getConfig();
    QVERIFY(!config.isNull());
    const ScreenPtr screen = config->screen();
    QVERIFY(!screen.isNull());

    QCOMPARE(screen->minSize(), QSize(320, 200));
    QCOMPARE(screen->maxSize(), QSize(8192, 8192));
    QCOMPARE(screen->currentSize(), QSize(3200, 1880));

    QCOMPARE(config->outputs().count(), 2);

    const OutputPtr output = config->outputs().take(2);
    QVERIFY(!output.isNull());

    QCOMPARE(output->name(), QString("HDMI1"));
    QCOMPARE(output->type(), Output::HDMI);
    QCOMPARE(output->modes().count(), 4);
    QCOMPARE(output->pos(), QPoint(1280, 0));
    QCOMPARE(output->geometry(), QRect(1280, 0, 1920, 1080));
    QCOMPARE(output->currentModeId(), QLatin1String("4"));
    QCOMPARE(output->preferredModeId(), QLatin1String("4"));
    QCOMPARE(output->rotation(), Output::None);
    QCOMPARE(output->isConnected(), true);
    QCOMPARE(output->isEnabled(), true);
    QCOMPARE(output->isPrimary(), false);
    QVERIFY2(output->clones().isEmpty(), "This simulates extended output, no clones");

    const ModePtr mode = output->currentMode();
    QVERIFY(!mode.isNull());
    QCOMPARE(mode->size(), QSize(1920, 1080));
    QCOMPARE(mode->refreshRate(), (float)60.0);
}

void testScreenConfig::clonesOutput()
{
    qputenv("KSCREEN_BACKEND_ARGS", "TEST_DATA=" TEST_DATA "multipleclone.json");

    const ConfigPtr config = getConfig();
    QVERIFY(!config.isNull());
    const ScreenPtr screen = config->screen();
    QVERIFY(!screen.isNull());

    QCOMPARE(screen->minSize(), QSize(320, 200));
    QCOMPARE(screen->maxSize(), QSize(8192, 8192));
    QCOMPARE(screen->currentSize(), QSize(1024, 768));

    const OutputPtr one = config->outputs()[1];
    const OutputPtr two = config->outputs()[2];

    QCOMPARE(one->currentMode()->size(), two->currentMode()->size());
    QCOMPARE(one->clones().count(), 1);
    QCOMPARE(one->clones().first(), two->id());
    QVERIFY2(two->clones().isEmpty(), "Output two should have no clones");
}

void testScreenConfig::configCanBeApplied()
{
    qputenv("KSCREEN_BACKEND_ARGS", "TEST_DATA=" TEST_DATA "singleoutputBroken.json");
    const ConfigPtr brokenConfig = getConfig();

    qputenv("KSCREEN_BACKEND_ARGS", "TEST_DATA=" TEST_DATA "singleoutput.json");
    const ConfigPtr currentConfig = getConfig();
    QVERIFY(!currentConfig.isNull());
    const OutputPtr primaryBroken = brokenConfig->outputs()[2];
    QVERIFY(!primaryBroken.isNull());
    const OutputPtr currentPrimary = currentConfig->outputs()[1];
    QVERIFY(!currentPrimary.isNull());

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


    qputenv("KSCREEN_BACKEND_ARGS", "TEST_DATA=" TEST_DATA "tooManyOutputs.json");
    const ConfigPtr brokenConfig2 = getConfig();
    QVERIFY(!brokenConfig2.isNull());

    int enabledOutputsCount = 0;
    Q_FOREACH (const OutputPtr &output, brokenConfig2->outputs()) {
        if (output->isEnabled()) {
            ++enabledOutputsCount;
        }
    }
    QVERIFY(brokenConfig2->screen()->maxActiveOutputsCount() < enabledOutputsCount);
    QVERIFY(!Config::canBeApplied(brokenConfig2));
}

QTEST_MAIN(testScreenConfig)

#include "testscreenconfig.moc"
