/*
 *  SPDX-FileCopyrightText: 2016 Sebastian Kügler <sebas@kde.org>
 *
 *  SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <QCoreApplication>
#include <QObject>
#include <QSignalSpy>
#include <QtTest>

#include <KWayland/Client/connection_thread.h>
#include <KWayland/Client/dpms.h>
#include <KWayland/Client/registry.h>

#include "waylandtestserver.h"

static const QString s_socketName = QStringLiteral("libkscreen-test-wayland-backend-0");
// static const QString s_socketName = QStringLiteral("wayland-0");

Q_LOGGING_CATEGORY(KSCREEN, "kscreen")

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
    QThread *m_thread;
    Registry *m_registry;

    KScreen::WaylandTestServer *m_server;
};

TestDpmsClient::TestDpmsClient(QObject *parent)
    : QObject(parent)
    , m_server(nullptr)
{
    setenv("WAYLAND_DISPLAY", s_socketName.toLocal8Bit().constData(), true);
    m_server = new KScreen::WaylandTestServer(this);
    m_server->start();
}

void TestDpmsClient::initTestCase()
{
    // setup connection
    m_connection = new KWayland::Client::ConnectionThread;
    m_connection->setSocketName(s_socketName);
    QSignalSpy connectedSpy(m_connection, SIGNAL(connected()));
    m_connection->setSocketName(s_socketName);

    m_thread = new QThread(this);
    m_connection->moveToThread(m_thread);
    m_thread->start();

    m_connection->initConnection();
    QVERIFY(connectedSpy.wait());

    QSignalSpy dpmsSpy(this, &TestDpmsClient::dpmsAnnounced);

    m_connection->initConnection();
    QVERIFY(connectedSpy.wait(100));

    m_registry = new KWayland::Client::Registry;
    m_registry->create(m_connection);
    QObject::connect(m_registry, &Registry::interfacesAnnounced, this, [this] {
        const bool hasDpms = m_registry->hasInterface(Registry::Interface::Dpms);
        if (hasDpms) {
            qDebug() << QStringLiteral("Compositor provides a DpmsManager");
        } else {
            qDebug() << QStringLiteral("Compositor does not provid a DpmsManager");
        }
        Q_EMIT  this->dpmsAnnounced();
    });
    m_registry->setup();

    QVERIFY(dpmsSpy.wait(100));
}

void TestDpmsClient::cleanupTestCase()
{
    m_thread->exit();
    m_thread->wait();
    delete m_registry;
    delete m_thread;
    delete m_connection;
}

void TestDpmsClient::testDpmsConnect()
{
    QVERIFY(m_registry->isValid());
}

QTEST_GUILESS_MAIN(TestDpmsClient)

#include "testkwaylanddpms.moc"
