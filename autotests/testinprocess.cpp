/*
 *  SPDX-FileCopyrightText: 2015 Sebastian Kügler <sebas@kde.org>
 *
 *  SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <QCoreApplication>
#include <QElapsedTimer>
#include <QLoggingCategory>
#include <QObject>
#include <QSignalSpy>
#include <QTest>

#include "../src/backendmanager_p.h"
#include "../src/config.h"
#include "../src/configmonitor.h"
#include "../src/getconfigoperation.h"
#include "../src/mode.h"
#include "../src/output.h"
#include "../src/setconfigoperation.h"

Q_LOGGING_CATEGORY(KSCREEN, "kscreen")

using namespace KScreen;

class TestInProcess : public QObject
{
    Q_OBJECT

public:
    explicit TestInProcess(QObject *parent = nullptr);

private Q_SLOTS:
    void initTestCase();

    void init();
    void cleanup();

    void loadConfig();

    void testCreateJob();
    void testBackendCaching();

    void testConfigApply();
    void testConfigMonitor();

private:
    ConfigPtr m_config;
};

TestInProcess::TestInProcess(QObject *parent)
    : QObject(parent)
    , m_config(nullptr)
{
}

void TestInProcess::initTestCase()
{
}

void TestInProcess::init()
{
    // Use Fake backend with one of the json configs
    qputenv("KSCREEN_BACKEND", "Fake");

    KScreen::BackendManager::instance()->shutdownBackend();
    KScreen::BackendManager::instance()->setBackendArgs({{QStringLiteral("TEST_DATA"), TEST_DATA "multipleoutput.json"}});
}

void TestInProcess::cleanup()
{
    KScreen::BackendManager::instance()->shutdownBackend();
}

void TestInProcess::loadConfig()
{
    qputenv("KSCREEN_BACKEND_INPROCESS", "1");

    auto *op = new GetConfigOperation();
    QVERIFY(op->exec());
    m_config = op->config();
    QVERIFY(m_config);
    QVERIFY(m_config->isValid());
}

void TestInProcess::testBackendCaching()
{
    KScreen::BackendManager::instance()->shutdownBackend();
    qputenv("KSCREEN_BACKEND", "Fake");
    QElapsedTimer t;
    int t_cold;
    int t_warm;

    {
        t.start();
        auto cp = new GetConfigOperation();
        cp->exec();
        auto cc = cp->config();
        t_cold = t.nsecsElapsed();
        QVERIFY(cc != nullptr);
        QVERIFY(cc->isValid());
        QVERIFY(cc->outputs().count());
    }
    {
        // KScreen::BackendManager::instance()->shutdownBackend();
        t.start();
        auto cp = new GetConfigOperation();
        cp->exec();
        auto cc = cp->config();
        t_warm = t.nsecsElapsed();
        QVERIFY(cc != nullptr);
        QVERIFY(cc->isValid());
        QVERIFY(cc->outputs().count());
    }
    {
        auto cp = new GetConfigOperation();
        cp->exec();
        auto cc = cp->config();
        QVERIFY(cc != nullptr);
        QVERIFY(cc->isValid());
        QVERIFY(cc->outputs().count());
    }
    // Check if all our configs are still valid after the backend is gone
    KScreen::BackendManager::instance()->shutdownBackend();
}

void TestInProcess::testCreateJob()
{
    KScreen::BackendManager::instance()->shutdownBackend();
    {
        auto op = new GetConfigOperation();
        auto _op = qobject_cast<GetConfigOperation *>(op);
        QVERIFY(_op != nullptr);
        QVERIFY(op->exec());
        auto cc = op->config();
        QVERIFY(cc != nullptr);
        QVERIFY(cc->isValid());
    }
    KScreen::BackendManager::instance()->shutdownBackend();
}

void TestInProcess::testConfigApply()
{
    qputenv("KSCREEN_BACKEND", "Fake");
    KScreen::BackendManager::instance()->shutdownBackend();
    auto op = new GetConfigOperation();
    op->exec();
    auto config = op->config();
    //     qDebug() << "op:" << config->outputs().count();
    auto output = config->outputs().first();
    //     qDebug() << "res:" << output->geometry();
    //     qDebug() << "modes:" << output->modes();
    auto m0 = output->modes().first();
    // qDebug() << "m0:" << m0->id() << m0;
    output->setCurrentModeId(m0->id());
    QVERIFY(Config::canBeApplied(config));

    // expected to fail, SetConfigOperation is out-of-process only
    auto setop = new SetConfigOperation(config);
    QVERIFY(!setop->hasError());
    QVERIFY(setop->exec());

    QVERIFY(!setop->hasError());
}

void TestInProcess::testConfigMonitor()
{
    qputenv("KSCREEN_BACKEND", "Fake");

    KScreen::BackendManager::instance()->shutdownBackend();
    auto op = new GetConfigOperation();
    op->exec();
    auto config = op->config();
    //     qDebug() << "op:" << config->outputs().count();
    auto output = config->outputs().first();
    //     qDebug() << "res:" << output->geometry();
    //     qDebug() << "modes:" << output->modes();
    auto m0 = output->modes().first();
    // qDebug() << "m0:" << m0->id() << m0;
    output->setCurrentModeId(m0->id());
    QVERIFY(Config::canBeApplied(config));

    QSignalSpy monitorSpy(ConfigMonitor::instance(), &ConfigMonitor::configurationChanged);
    qDebug() << "MOnitorspy connencted.";
    ConfigMonitor::instance()->addConfig(config);

    auto setop = new SetConfigOperation(config);
    QVERIFY(!setop->hasError());
    // do not cal setop->exec(), this must not block as the signalspy already blocks
    QVERIFY(monitorSpy.wait(500));
}

QTEST_GUILESS_MAIN(TestInProcess)

#include "testinprocess.moc"
