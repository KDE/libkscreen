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

#define QT_GUI_LIB

#include <QtTest/QtTest>
#include <QtCore/QObject>
#include <QLoggingCategory>

#include "../src/log.h"

Q_DECLARE_LOGGING_CATEGORY(KSCREEN_TESTLOG)

Q_LOGGING_CATEGORY(KSCREEN_TESTLOG, "kscreen.testlog")

using namespace KScreen;

auto KSCREEN_LOGGING = "KSCREEN_LOGGING";
auto KSCREEN_LOGFILE = "KSCREEN_LOGFILE";



class TestLog : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void init();
    void initTestCase();
    void cleanupTestCase();
    void testContext();
    void testEnabled();
    void testLogFile();
    void testLog();

private:
    QString m_defaultLogFile;
};

void TestLog::init()
{
    QLoggingCategory::setFilterRules(QStringLiteral("kscreen.*=true"));
    QStandardPaths::setTestModeEnabled(true);
    m_defaultLogFile = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + "/kscreen/kscreen.log";
}

void TestLog::initTestCase()
{

}

void TestLog::cleanupTestCase()
{
    qunsetenv(KSCREEN_LOGGING);
    qunsetenv(KSCREEN_LOGFILE);
}

void TestLog::testContext()
{
    auto log = new KScreen::Log;
    QString ctx("context text");
    QVERIFY(log != nullptr);
    log->setContext(ctx);
    QCOMPARE(log->context(), ctx);

    delete log;
}

void TestLog::testEnabled()
{
    auto logfile = "/tmp/kscreenlogfile.log";
    qputenv(KSCREEN_LOGFILE, logfile);
    qputenv(KSCREEN_LOGGING, QByteArray("faLSe"));

    auto log = new KScreen::Log;
    QCOMPARE(log->enabled(), false);
    QCOMPARE(log->logFile(), QString::fromLocal8Bit(logfile));

    delete log;
    qunsetenv(KSCREEN_LOGGING);
    qunsetenv(KSCREEN_LOGFILE);

    log = new KScreen::Log;
    QCOMPARE(log->enabled(), true);

    delete log;
}

void TestLog::testLogFile()
{
    auto logfile = "/tmp/kscreenlogfile.log";
    qputenv(KSCREEN_LOGFILE, logfile);

    auto log = new KScreen::Log;
    QCOMPARE(log->enabled(), true);
    QCOMPARE(log->logFile(), QString::fromLocal8Bit(logfile));

    delete log;
    qunsetenv(KSCREEN_LOGFILE);

    log = new KScreen::Log;
    QCOMPARE(log->logFile(), m_defaultLogFile);
    delete log;
}

void TestLog::testLog()
{
    QFile lf(m_defaultLogFile);
    lf.remove();
    QVERIFY(!lf.exists());

    QString logmsg("This is a log message. ♥");
    Log::log(logmsg);

    QVERIFY(lf.exists());
    QVERIFY(lf.remove());

    qCDebug(KSCREEN_TESTLOG) << "qCDebug message from testlog";
    QVERIFY(lf.exists());
    QVERIFY(lf.remove());
}


QTEST_MAIN(TestLog)

#include "testlog.moc"
