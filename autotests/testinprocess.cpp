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
#include "../src/setconfigoperation.h"
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

    void testCreateJob();
    void testModeSwitching();
    void testBackendCaching();

    void testConfigApply();

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
    KScreen::BackendManager::instance()->shutdownBackend();
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

    //qDebug() << "TT xrandr out-of-process";
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
    //qDebug() << "TT fake in-process";

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
    KScreen::BackendManager::instance()->shutdownBackend();
    setenv("KSCREEN_BACKEND", "Fake", 1);
    QElapsedTimer t;
    BackendManager::instance()->setMode(BackendManager::InProcess);
    QCOMPARE(BackendManager::instance()->mode(), BackendManager::InProcess);
    int t_cold;
    int t_warm;

    {
        t.start();
        auto cp = new InProcessConfigOperation();
        cp->exec();
        auto cc = cp->config();
        t_cold = t.nsecsElapsed();
        QVERIFY(cc != nullptr);
        QVERIFY(cc->isValid());
        QVERIFY(cc->outputs().count());
    }
    {
        //KScreen::BackendManager::instance()->shutdownBackend();
        QCOMPARE(BackendManager::instance()->mode(), BackendManager::InProcess);
        t.start();
        auto cp = new InProcessConfigOperation();
        cp->exec();
        auto cc = cp->config();
        t_warm = t.nsecsElapsed();
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

    //setenv("KSCREEN_BACKEND", "QScreen", 1);
    setenv("KSCREEN_BACKEND_INPROCESS", "0", 1);
    BackendManager::instance()->setMode(BackendManager::OutOfProcess);
    QCOMPARE(BackendManager::instance()->mode(), BackendManager::OutOfProcess);
    int t_x_cold;

    {
        t.start();
        auto xp = new GetConfigOperation();
        xp->exec();
        t_x_cold = t.nsecsElapsed();
        auto xc = xp->config();
        QVERIFY(xc != nullptr);
    }
    t.start();
    auto xp = new GetConfigOperation();
    xp->exec();
    int t_x_warm = t.nsecsElapsed();
    auto xc = xp->config();
    QVERIFY(xc != nullptr);

    // Make sure in-process is faster
    QVERIFY(t_cold > t_warm);
    QVERIFY(t_x_cold > t_x_warm);
    QVERIFY(t_x_cold > t_cold);
    return;
    qDebug() << "ip  speedup for cached access:" << (qreal)((qreal)t_cold / (qreal)t_warm);
    qDebug() << "oop speedup for cached access:" << (qreal)((qreal)t_x_cold / (qreal)t_x_warm);
    qDebug() << "out-of vs. in-process speedup:" << (qreal)((qreal)t_x_warm / (qreal)t_warm);
    qDebug() << "cold oop:   " << ((qreal)t_x_cold / 1000000);
    qDebug() << "cached oop: " << ((qreal)t_x_warm / 1000000);
    qDebug() << "cold in process:   " << ((qreal)t_cold / 1000000);
    qDebug() << "cached in process: " << ((qreal)t_warm / 1000000);
}

void TestInProcess::testCreateJob()
{
    KScreen::BackendManager::instance()->shutdownBackend();
    {
        BackendManager::instance()->setMode(BackendManager::InProcess);
        auto op = ConfigOperation::create();
        auto _op = qobject_cast<InProcessConfigOperation*>(op);
        QVERIFY(_op != nullptr);
        QCOMPARE(BackendManager::instance()->mode(), BackendManager::InProcess);
        op->exec();
        auto cc = op->config();
        QVERIFY(cc != nullptr);
        QVERIFY(cc->isValid());
    }
    {
        BackendManager::instance()->setMode(BackendManager::OutOfProcess);
        auto op = ConfigOperation::create();
        auto _op = qobject_cast<GetConfigOperation*>(op);
        QVERIFY(_op != nullptr);
        QCOMPARE(BackendManager::instance()->mode(), BackendManager::OutOfProcess);
        op->exec();
        auto cc = op->config();
        QVERIFY(cc != nullptr);
        QVERIFY(cc->isValid());
    }
    KScreen::BackendManager::instance()->shutdownBackend();
    BackendManager::instance()->setMode(BackendManager::InProcess);
}

void TestInProcess::testConfigApply()
{
    setenv("KSCREEN_BACKEND", "Fake", 1);
    KScreen::BackendManager::instance()->shutdownBackend();
    BackendManager::instance()->setMode(BackendManager::InProcess);
    auto op = ConfigOperation::create();
    op->exec();
    auto config = op->config();
//     qDebug() << "op:" << config->outputs().count();
    auto output = config->outputs().first();
//     qDebug() << "res:" << output->geometry();
//     qDebug() << "modes:" << output->modes();
    auto m0 = output->modes().first();
    //qDebug() << "m0:" << m0->id() << m0;
    output->setCurrentModeId(m0->id());
    QVERIFY(Config::canBeApplied(config));

    // expected to fail, SetConfigOperation is out-of-process only
    auto setfail = new SetConfigOperation(config);
    QVERIFY(!setfail->hasError());
    setfail->exec();
    QVERIFY(setfail->hasError());
    qDebug() << "----------------------------------------------------------";
    auto setop = ConfigOperation::setOperation(config);
    QVERIFY(!setop->hasError());
    setop->exec();

    QVERIFY(!setop->hasError());
}

QTEST_GUILESS_MAIN(TestInProcess)

#include "testinprocess.moc"
