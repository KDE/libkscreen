/*************************************************************************************
 *  Copyright 2016 by Sebastian Kügler <sebas@kde.org>                           *
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

Q_LOGGING_CATEGORY(KSCREEN, "kscreen");

using namespace KScreen;

class TestBackendLoader : public QObject
{
    Q_OBJECT

public:
    explicit TestBackendLoader(QObject *parent = nullptr);

private Q_SLOTS:

    void testPreferredBackend();
    void testEnv();
    void testEnv_data();
    void testLoadBackend();

};

TestBackendLoader::TestBackendLoader(QObject *parent)
    : QObject(parent)
{
}

void TestBackendLoader::testPreferredBackend()
{
    auto backends = BackendManager::instance()->listBackends();
    QVERIFY(!backends.isEmpty());
    auto preferred = BackendManager::instance()->preferredBackend();
    QVERIFY(preferred.exists());
}

void TestBackendLoader::testEnv_data()
{
    // save this to restore it later and avoid surprises
    auto _b = qgetenv("KSCREEN_BACKEND");

    QTest::addColumn<QString>("var");
    QTest::addColumn<QString>("backend");

    QTest::newRow("all lower") << "kwayland" << "KSC_KWayland";
    QTest::newRow("camel case") << "KWayland" << "KSC_KWayland";
    QTest::newRow("all upper") << "KWAYLAND" << "KSC_KWayland";
    QTest::newRow("mixed") << "kwAYlaND" << "KSC_KWayland";

    QTest::newRow("xrandr 1.1") << "xrandr11" << "KSC_XRandR11";
    QTest::newRow("qscreen") << "qscreen" << "KSC_QScreen";
    QTest::newRow("mixed") << "fake" << "KSC_Fake";

    // set to original value
    qputenv("KSCREEN_BACKEND", _b);
}

void TestBackendLoader::testEnv()
{
    // We want to be pretty liberal, so this should work
    QFETCH(QString, var);
    QFETCH(QString, backend);
    qputenv("KSCREEN_BACKEND", var.toLocal8Bit());
    auto preferred = BackendManager::instance()->preferredBackend();
    QVERIFY(preferred.fileName().startsWith(backend));
}

void TestBackendLoader::testLoadBackend()
{

}


QTEST_GUILESS_MAIN(TestBackendLoader)

#include "testbackendloader.moc"
