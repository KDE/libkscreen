/*
 *  SPDX-FileCopyrightText: 2016 Sebastian Kügler <sebas@kde.org>
 *
 *  SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "dpmsclient.h"

#include <QCommandLineParser>
#include <QLoggingCategory>
#include <QRect>
#include <QStandardPaths>
#include <QThread>

#include <KWayland/Client/connection_thread.h>
#include <KWayland/Client/output.h>
#include <KWayland/Client/registry.h>

#include "qwayland-dpms.h"
#include <QtWaylandClient/qwaylandclientextension.h>

Q_LOGGING_CATEGORY(KSCREEN_DPMS, "kscreen.dpms")

using namespace KScreen;

class DpmsManager : public QWaylandClientExtensionTemplate<DpmsManager>, public QtWayland::org_kde_kwin_dpms_manager
{
public:
    DpmsManager()
        : QWaylandClientExtensionTemplate<DpmsManager>(1)
    {
    }
};

class Dpms : public QObject, public QtWayland::org_kde_kwin_dpms
{
    Q_OBJECT
public:
    Dpms(struct ::org_kde_kwin_dpms *object, QObject *parent)
        : QObject(parent)
        , org_kde_kwin_dpms(object)
    {
    }

    bool isSupported() const
    {
        return m_supported;
    }

    Q_SIGNAL void supportedChanged();
    Q_SIGNAL void modeChanged();

private:
    bool m_supported = false;
    bool m_pendingSupported = false;

    void org_kde_kwin_dpms_supported(uint32_t supported) override
    {
        m_pendingSupported = supported;
    }

    void org_kde_kwin_dpms_mode(uint32_t mode) override
    {
        Q_UNUSED(mode);
    }

    void org_kde_kwin_dpms_done() override
    {
        m_supported = m_pendingSupported;
        Q_EMIT supportedChanged();
        Q_EMIT modeChanged();
    }
};

DpmsClient::DpmsClient(QObject *parent)
    : QObject(parent)
    , m_registry(new KWayland::Client::Registry)
{
}

DpmsClient::~DpmsClient()
{
    delete m_registry;
    delete m_connection;
}

void DpmsClient::connect()
{
    // setup connection
    m_connection = KWayland::Client::ConnectionThread::fromApplication(this);

    qDebug() << "Connected!";
    m_registry->create(m_connection);

    m_registry->setup();

    m_manager = new DpmsManager;

    QObject::connect(m_manager, &DpmsManager::activeChanged, this, [this] {
        const bool hasDpms = m_manager->isActive();
        if (hasDpms) {
            qDebug() << QStringLiteral("Compositor provides a DpmsManager");
        } else {
            qDebug() << QStringLiteral("Compositor does not provid a DpmsManager");
        }

        Q_EMIT this->ready();
    });

    // QVERIFY(dpmsSpy.wait(100));
}

void KScreen::DpmsClient::changeMode(Mode mode)
{
    const auto outputs = m_registry->interfaces(KWayland::Client::Registry::Interface::Output);
    for (auto outputInterface : outputs) {
        KWayland::Client::Output *output = m_registry->createOutput(outputInterface.name, outputInterface.version, m_registry);

        Dpms *dpms = new Dpms(m_manager->get(output->output()), output);

        QObject::connect(dpms, &Dpms::supportedChanged, this, [dpms, mode, output, this] {
            if (m_excludedOutputNames.contains(output->model())) {
                qDebug() << "Skipping" << output->model() << output->manufacturer();
                return;
            }

            if (dpms->isSupported()) {
                QObject::connect(dpms, &Dpms::modeChanged, this, &DpmsClient::modeChanged, Qt::QueuedConnection);
                qDebug() << "Switching" << output->model() << output->manufacturer() << (mode == Mode::On ? "on" : "off");
                m_modeChanges++;
                dpms->set(mode);
            }
        });
    }
}

void DpmsClient::modeChanged()
{
    m_modeChanges = m_modeChanges - 1;
    if (m_modeChanges <= 0) {
        Q_EMIT finished();
        m_modeChanges = 0;
    }
}

void DpmsClient::on()
{
    changeMode(Mode::On);
    // Q_EMIT finished();
}

void KScreen::DpmsClient::off()
{
    changeMode(Mode::Off);
    // Q_EMIT finished();
}

#include "dpmsclient.moc"
