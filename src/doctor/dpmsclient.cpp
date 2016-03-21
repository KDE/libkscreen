/*************************************************************************************
 *  Copyright 2014-2015 Sebastian Kügler <sebas@kde.org>                             *
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

Q_LOGGING_CATEGORY(KSCREEN_DPMS, "kscreen.dpms");

using namespace KScreen;

using namespace KWayland::Client;

DpmsClient::DpmsClient(QObject *parent)
    : QObject(parent)
    , m_thread(nullptr)
    , m_connection(nullptr)
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

            DpmsManager *dpmsManager = nullptr;
            if (hasDpms) {
                const auto dpmsData = m_registry.interface(Registry::Interface::Dpms);
                dpmsManager = m_registry.createDpmsManager(dpmsData.name, dpmsData.version);
            }

            // get all Outputs
            const auto outputs = m_registry.interfaces(Registry::Interface::Output);
            for (auto outputInterface : outputs) {

                KWayland::Client::Output *output = m_registry.createOutput(outputInterface.name, outputInterface.version, &m_registry);
                qDebug() << "OUTPUT!" << output->model() << output->manufacturer() << output->geometry();
                //QLabel *label = new QLabel(output->model());
//                 QObject::connect(output, &KWayland::Client::Output::changed, label,
//                     [label, output] {
//                         //label->setText(output->model());
//                     }, Qt::QueuedConnection
//                 );

                Dpms *dpms = nullptr;
                if (dpmsManager) {
                    dpms = dpmsManager->getDpms(output, output);
                }

//                 QFormLayout *dpmsForm = new QFormLayout;
                bool supported = dpms ? dpms->isSupported() : false;
//                 QLabel *supportedLabel = new QLabel(supportedToString(supported));
//                 qDebug() << "OUTPUT!";
//                 dpmsForm->addRow(QStringLiteral("Supported:"), supportedLabel);
// //                 QLabel *modeLabel = new QLabel(modeToString(dpms ? dpms->mode() : Dpms::Mode::On));
//                 dpmsForm->addRow(QStringLiteral("Mode:"), modeLabel);

//                 QPushButton *standbyButton = new QPushButton(QStringLiteral("Standby"));
//                 QPushButton *suspendButton = new QPushButton(QStringLiteral("Suspend"));
//                 QPushButton *offButton = new QPushButton(QStringLiteral("Off"));
//                 standbyButton->setEnabled(supported);
//                 suspendButton->setEnabled(supported);
//                 offButton->setEnabled(supported);
//                 QDialogButtonBox *bg = new QDialogButtonBox;
//                 bg->addButton(standbyButton, QDialogButtonBox::ActionRole);
//                 bg->addButton(suspendButton, QDialogButtonBox::ActionRole);
//                 bg->addButton(offButton, QDialogButtonBox::ActionRole);

                if (dpms) {
                    QObject::connect(dpms, &Dpms::supportedChanged, this,
                        [this, dpms] {
                            const bool supported = dpms->isSupported();
//                             supportedLabel->setText(supportedToString(supported));
//                             standbyButton->setEnabled(supported);
//                             suspendButton->setEnabled(supported);
//                             offButton->setEnabled(supported);
                        }, Qt::QueuedConnection
                    );
                    QObject::connect(dpms, &Dpms::modeChanged, this,
                        [dpms] {
//                             modeLabel->setText(modeToString(dpms->mode()));
                        }, Qt::QueuedConnection
                    );
//                     QObject::connect(standbyButton, &QPushButton::clicked, dpms, [dpms] { dpms->requestMode(Dpms::Mode::Standby);});
//                     QObject::connect(suspendButton, &QPushButton::clicked, dpms, [dpms] { dpms->requestMode(Dpms::Mode::Suspend);});
//                     QObject::connect(offButton, &QPushButton::clicked, dpms, [dpms] { dpms->requestMode(Dpms::Mode::Off);});
                    if (m_setOff) {
                        qDebug() << "Switching off";
                        dpms->requestMode(Dpms::Mode::Off);
                    }
                }





                //                 layout->addLayout(setupOutput(o, &registry, dpmsManager));
//                 QFrame *hline = new QFrame;
//                 hline->setFrameShape(QFrame::HLine);
//                 layout->addWidget(hline);
            }

            emit this->ready();
        });
    m_registry.setup();

    //QVERIFY(dpmsSpy.wait(100));

}

void DpmsClient::on()
{

}

void KScreen::DpmsClient::off()
{

}

void KScreen::DpmsClient::setTimeout(int msec)
{

}
