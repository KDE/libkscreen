/*
 *  SPDX-FileCopyrightText: 2012 Alejandro Fiestas Olivares <afiestas@kde.org>
 *  SPDX-FileCopyrightText: 2012, 2013 Daniel Vrátil <dvratil@redhat.com>
 *  SPDX-FileCopyrightText: 2014-2015 Sebastian Kügler <sebas@kde.org>
 *  SPDX-FileCopyrightText: 2021 Méven Car <meven.car@enioka.com>
 *
 *  SPDX-License-Identifier: LGPL-2.1-or-later
 */
#include "waylandbackend.h"

#include "waylandconfig.h"
#include "waylandoutputdevice.h"

#include "kscreen_kwayland_logging.h"

#include <configmonitor.h>
#include <mode.h>
#include <output.h>

#include <QProcess>
#include <QSettings>
#include <QStandardPaths>

using namespace KScreen;

// This class scopes the connections to the internal config changes signals to the matching request
// TODO it would be better if WaylandConfig::applyConfig handled the QFuture itself.
class SetConfigJob : public QObject
{
    Q_OBJECT
public:
    explicit SetConfigJob(QObject *parent = nullptr)
        : QObject(parent)
    {
        m_pendingResult.start();
    }
    void fail(const QString &error)
    {
        deleteLater();
        m_pendingResult.addResult(std::unexpected(error));
        m_pendingResult.finish();
    }
    void finish()
    {
        deleteLater();
        m_pendingResult.addResult(SetConfigResult());
        m_pendingResult.finish();
    }
    QFuture<SetConfigResult> future()
    {
        return m_pendingResult.future();
    }

private:
    QPromise<SetConfigResult> m_pendingResult;
};

WaylandBackend::WaylandBackend()
    : KScreen::AbstractBackend()
    , m_internalConfig(new WaylandConfig(this))
{
    qCDebug(KSCREEN_WAYLAND) << "Loading Wayland backend.";
    connect(m_internalConfig, &WaylandConfig::configChanged, this, [this] {
        Q_EMIT configChanged(m_internalConfig->currentConfig());
    });
}

QString WaylandBackend::name() const
{
    return QStringLiteral("kwayland");
}

QString WaylandBackend::serviceName() const
{
    return QStringLiteral("org.kde.KScreen.Backend.KWayland");
}

ConfigPtr WaylandBackend::config() const
{
    // Note: This should ONLY be called from GetConfigOperation!
    return m_internalConfig->currentConfig();
}

QFuture<SetConfigResult> WaylandBackend::setConfig(const KScreen::ConfigPtr &newconfig)
{
    if (!newconfig) {
        return QtFuture::makeReadyFuture<SetConfigResult>(std::unexpected(QStringLiteral("config is nullptr!")));
    }

    SetConfigJob *job = new SetConfigJob(this);
    connect(m_internalConfig, &WaylandConfig::configChanged, job, &SetConfigJob::finish);
    connect(m_internalConfig, &WaylandConfig::configFailed, job, &SetConfigJob::fail);
    if (!m_internalConfig->applyConfig(newconfig)) {
        // nothing changed
        job->finish();
    }
    return job->future();
}

QByteArray WaylandBackend::edid(int outputId) const
{
    WaylandOutputDevice *output = m_internalConfig->outputMap().value(outputId);
    if (!output) {
        return QByteArray();
    }
    return output->edid();
}

bool WaylandBackend::isValid() const
{
    return m_internalConfig->isReady();
}

#include "moc_waylandbackend.cpp"
#include "waylandbackend.moc"
