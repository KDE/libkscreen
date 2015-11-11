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

#include <KSharedConfig>
#include <KConfigGroup>

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

    void concurrentOperation();

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
}

void TestInProcess::concurrentOperation()
{
    // Load QScreen backend in-process
    setenv("KSCREEN_BACKEND", "QScreen", 1);
    auto op = new InProcessConfigOperation();
    op->exec();
    QVERIFY(op->config() != nullptr);
    QVERIFY(op->config()->isValid());

    // Load the Fake backend in-process
    setenv("KSCREEN_BACKEND", "Fake", 1);
    auto ip = new InProcessConfigOperation();
    ip->exec();
    QVERIFY(ip->config() != nullptr);
    QVERIFY(ip->config()->isValid());
}



QTEST_GUILESS_MAIN(TestInProcess)

#include "testinprocess.moc"
