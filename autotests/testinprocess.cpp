/*************************************************************************************
 *  Copyright 2015 by Sebastian Kügler <sebas@kde.org>                           *
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

#include <QCoreApplication>
#include <QtTest>
#include <QObject>
#include <QSignalSpy>

#include "../src/backendmanager_p.h"
#include "../src/inprocessconfigoperation.h"
#include "../src/getconfigoperation.h"
#include "../src/config.h"
#include "../src/configmonitor.h"
#include "../src/output.h"
#include "../src/mode.h"
#include "../src/edid.h"

Q_LOGGING_CATEGORY(KSCREEN, "kscreen");

using namespace KScreen;

class TestInProcess : public QObject
{
    Q_OBJECT

public:
    explicit TestInProcess(QObject *parent = nullptr);

private Q_SLOTS:
    void init();
    void cleanup();

    void loadConfig();

    void testModeSwitching();
    void testBackendCaching();

private:

    ConfigPtr m_config;

};

TestInProcess::TestInProcess(QObject *parent)
    : QObject(parent)
    , m_config(nullptr)
{
}

void TestInProcess::init()
{
    // Make sure we do everything in-process
    setenv("KSCREEN_BACKEND_INPROCESS", "1", 1);
    // Use Fake backend with one of the json configs
    setenv("KSCREEN_BACKEND", "Fake", 1);
    qputenv("KSCREEN_BACKEND_ARGS", "TEST_DATA=" TEST_DATA "multipleoutput.json");

    KScreen::BackendManager::instance()->shutdownBackend();
}

void TestInProcess::cleanup()
{
    KScreen::BackendManager::instance()->shutdownBackend();
}

void TestInProcess::loadConfig()
{
    InProcessConfigOperation *op = new InProcessConfigOperation();
    op->exec();
    m_config = op->config();
    QVERIFY(m_config);
    QVERIFY(m_config->isValid());
}

void TestInProcess::testModeSwitching()
{
    // Load QScreen backend in-process
//     qDebug() << "TT qscreen in-process";
    setenv("KSCREEN_BACKEND", "QScreen", 1);
    auto op = new InProcessConfigOperation();
    op->exec();
    auto oc = op->config();
    QVERIFY(oc != nullptr);
    QVERIFY(oc->isValid());

//     qDebug() << "TT fake in-process";
    // Load the Fake backend in-process
    setenv("KSCREEN_BACKEND", "Fake", 1);
    auto ip = new InProcessConfigOperation();
    ip->exec();
    auto ic = ip->config();
    QVERIFY(ic != nullptr);
    QVERIFY(ic->isValid());
    QVERIFY(ic->outputs().count());

    qDebug() << "TT xrandr out-of-process";
    // Load the xrandr backend out-of-process
    setenv("KSCREEN_BACKEND", "XRandR", 1);
    setenv("KSCREEN_BACKEND_INPROCESS", "0", 1);
    BackendManager::instance()->setMode(BackendManager::OutOfProcess);
    auto xp = new GetConfigOperation();
    QCOMPARE(BackendManager::instance()->mode(), BackendManager::OutOfProcess);
    xp->exec();
    auto xc = xp->config();
    QVERIFY(xc != nullptr);
    QVERIFY(xc->isValid());
    QVERIFY(xc->outputs().count());
        qDebug() << "TT fake in-process";

    setenv("KSCREEN_BACKEND_INPROCESS", "1", 1);
    BackendManager::instance()->setMode(BackendManager::InProcess);
    // Load the Fake backend in-process
    setenv("KSCREEN_BACKEND", "Fake", 1);
    auto fp = new InProcessConfigOperation();
    QCOMPARE(BackendManager::instance()->mode(), BackendManager::InProcess);
    fp->exec();
    auto fc = fp->config();
    QVERIFY(fc != nullptr);
    QVERIFY(fc->isValid());
    QVERIFY(fc->outputs().count());

    QVERIFY(oc->isValid());
    QVERIFY(ic->isValid());
    QVERIFY(xc->isValid());
    QVERIFY(fc->isValid());
}
void TestInProcess::testBackendCaching()
{
    setenv("KSCREEN_BACKEND", "Fake", 1);
    BackendManager::instance()->setMode(BackendManager::InProcess);
    {
        auto cp = new InProcessConfigOperation();
        QCOMPARE(BackendManager::instance()->mode(), BackendManager::InProcess);
        cp->exec();
        auto cc = cp->config();
        QVERIFY(cc != nullptr);
        QVERIFY(cc->isValid());
        QVERIFY(cc->outputs().count());
    }
    {
        auto cp = new InProcessConfigOperation();
        QCOMPARE(BackendManager::instance()->mode(), BackendManager::InProcess);
        cp->exec();
        auto cc = cp->config();
        QVERIFY(cc != nullptr);
        QVERIFY(cc->isValid());
        QVERIFY(cc->outputs().count());
    }
    {
        auto cp = new InProcessConfigOperation();
        QCOMPARE(BackendManager::instance()->mode(), BackendManager::InProcess);
        cp->exec();
        auto cc = cp->config();
        QVERIFY(cc != nullptr);
        QVERIFY(cc->isValid());
        QVERIFY(cc->outputs().count());
    }
    // Check if all our configs are still valid after the backend is gone
    KScreen::BackendManager::instance()->shutdownBackend();
}



QTEST_GUILESS_MAIN(TestInProcess)

#include "testinprocess.moc"
