/*************************************************************************************
 *  Copyright 2016 Sebastian Kügler <sebas@kde.org>                                  *
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

#include "dpmsclient.h"

#include <QCommandLineParser>
#include <QLoggingCategory>
#include <QRect>
#include <QStandardPaths>
#include <QThread>

#include <KWayland/Client/connection_thread.h>
#include <KWayland/Client/dpms.h>
#include <KWayland/Client/registry.h>
#include <KWayland/Client/output.h>

//static const QString s_socketName = QStringLiteral("libkscreen-test-wayland-backend-0");
static const QString s_socketName = QStringLiteral("wayland-0");

Q_LOGGING_CATEGORY(KSCREEN_DPMS, "kscreen.dpms")

using namespace KScreen;

using namespace KWayland::Client;

DpmsClient::DpmsClient(QObject *parent)
    : QObject(parent)
    , m_thread(nullptr)
    , m_connection(nullptr)
    , m_dpmsManager(nullptr)
{

}

DpmsClient::~DpmsClient()
{
    m_thread->exit();
    m_thread->wait();
    delete m_thread;
    delete m_connection;

}

void DpmsClient::connect()
{
    // setup connection
    m_connection = new ConnectionThread;
    m_connection->setSocketName(s_socketName);
    QObject::connect(m_connection, &ConnectionThread::connected, this, &DpmsClient::connected);
    QObject::connect(m_connection, &ConnectionThread::failed, this, [=]() {
        qCDebug(KSCREEN_DPMS) << "Connection failed";
    });

    m_thread = new QThread(this);
    m_connection->moveToThread(m_thread);
    m_thread->start();

    m_connection->initConnection();

    qDebug() << "init";
}

void DpmsClient::connected()
{
    qDebug() << "Connected!";
    m_registry.create(m_connection);
    QObject::connect(&m_registry, &Registry::interfacesAnnounced, this,
        [this] {
            const bool hasDpms = m_registry.hasInterface(Registry::Interface::Dpms);
           // QLabel *hasDpmsLabel = new QLabel(&window);
            if (hasDpms) {
                qDebug() << QStringLiteral("Compositor provides a DpmsManager");
            } else {
                qDebug() << QStringLiteral("Compositor does not provid a DpmsManager");
            }

            if (hasDpms) {
                const auto dpmsData = m_registry.interface(Registry::Interface::Dpms);
                m_dpmsManager = m_registry.createDpmsManager(dpmsData.name, dpmsData.version);
            }


            emit this->ready();
        });
    m_registry.setup();

    //QVERIFY(dpmsSpy.wait(100));

}

void KScreen::DpmsClient::changeMode(KWayland::Client::Dpms::Mode mode)
{
    const auto outputs = m_registry.interfaces(Registry::Interface::Output);
    for (auto outputInterface : outputs) {

        KWayland::Client::Output *output = m_registry.createOutput(outputInterface.name, outputInterface.version, &m_registry);
        qDebug() << "OUTPUT!" << output->model() << output->manufacturer() << output->geometry();

        Dpms *dpms = nullptr;
        if (m_dpmsManager) {
            dpms = m_dpmsManager->getDpms(output, output);
        }

        if (dpms) {
            QObject::connect(dpms, &Dpms::supportedChanged, this,
                [dpms, mode, this] {
                    if (dpms->isSupported()) {
                        QObject::connect(dpms, &Dpms::modeChanged, this,
                            &DpmsClient::modeChanged, Qt::QueuedConnection);
                        qDebug() << "Switching " << (mode == Dpms::Mode::On ? "on" : "off");
                        m_modeChanges++;
                        dpms->requestMode(mode);
                    }

                }, Qt::QueuedConnection
            );
        }

    qDebug() << "dpms->isSupported()" << dpms->isSupported();
    }
}

void DpmsClient::modeChanged()
{
    m_modeChanges = m_modeChanges - 1;
    if (m_modeChanges <= 0) {
        emit finished();
        m_modeChanges = 0;
    }
}

void DpmsClient::on()
{
    changeMode(Dpms::Mode::On);
    //emit finished();
}

void KScreen::DpmsClient::off()
{
    changeMode(Dpms::Mode::Off);
    //emit finished();
}
