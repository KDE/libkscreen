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

#include <KWayland/Client/connection_thread.h>
#include <KWayland/Client/dpms.h>
#include <KWayland/Client/registry.h>

#include "../src/backendmanager_p.h"

static const QString s_socketName = QStringLiteral("libkscreen-test-wayland-backend-0");

Q_LOGGING_CATEGORY(KSCREEN, "kscreen");

using namespace KScreen;

using namespace KWayland::Client;

class TestDpmsClient : public QObject
{
    Q_OBJECT

public:
    explicit TestDpmsClient(QObject *parent = nullptr);

Q_SIGNALS:
    void dpmsAnnounced();


private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();

    void testDpmsConnect();

private:
    ConnectionThread *m_connection;
    Registry m_registry;
};

TestDpmsClient::TestDpmsClient(QObject *parent)
    : QObject(parent)
{
}

void TestDpmsClient::initTestCase()
{
    m_connection = new ConnectionThread;
    m_connection->setSocketName(s_socketName);

    QSignalSpy announced(&m_registry, &KWayland::Client::Registry::outputDeviceAnnounced);
    //QVERIFY(m_registry.isValid());
    //m_registry.setup();
    //wl_display_flush(m_connection->display());
    QThread *thread = new QThread;
    m_connection->moveToThread(thread);
    thread->start();

    m_connection->initConnection();
    m_registry.create(m_connection->display());
    m_registry.setup();
    QVERIFY(announced.wait(1000));
    qDebug() << "init";
}

void TestDpmsClient::cleanupTestCase()
{
    delete m_connection;
}

void TestDpmsClient::testDpmsConnect()
{
    QSignalSpy connectedSpy(m_connection, SIGNAL(connected()));
    //m_connection->setSocketName(s_socketName);

    m_connection->initConnection();
    QVERIFY(connectedSpy.wait());

    qDebug() << "Conenct";
    QObject::connect(&m_registry, &Registry::interfacesAnnounced, this, [this] {
            const bool hasDpms = m_registry.hasInterface(Registry::Interface::Dpms);
            if (hasDpms) {
                qDebug() << "Compositor provides a DpmsManager";
            } else {
                qDebug() << "Compositor does not provide a DpmsManager";
            }
            emit this->dpmsAnnounced();
        });

}

QTEST_GUILESS_MAIN(TestDpmsClient)

#include "testkwaylanddpms.moc"
