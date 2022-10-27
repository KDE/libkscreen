/*
 *  SPDX-FileCopyrightText: 2016 Sebastian Kügler <sebas@kde.org>
 *
 *  SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef KSCREEN_DPMSCLIENT_H
#define KSCREEN_DPMSCLIENT_H

#include "../config.h"
#include <QObject>
#include <QRect>

#include <KWayland/Client/registry.h>
#include <memory>

class QThread;

namespace KWayland
{
namespace Client
{
class ConnectionThread;
}
}

class Dpms;
class DpmsManager;

namespace KScreen
{
class DpmsClient : public QObject
{
    Q_OBJECT

public:
    explicit DpmsClient(QObject *parent = nullptr);
    ~DpmsClient() override;

    void setExcludedOutputNames(const QStringList &excluded)
    {
        m_excludedOutputNames = excluded;
    }

    void connect();
    void off();
    void on();

Q_SIGNALS:
    void ready();
    void finished();

private Q_SLOTS:
    void modeChanged();

private:
    enum Mode {
        On = 0,
        Standby = 1,
        Suspend = 2,
        Off = 3,
    };

    void changeMode(Mode mode);
    KWayland::Client::ConnectionThread *m_connection = nullptr;
    KWayland::Client::Registry *m_registry = nullptr;
    bool m_setOff = true;
    bool m_setOn = false;

    bool m_supportedOututCount = 0;
    int m_modeChanges = 0;
    QStringList m_excludedOutputNames;
    std::unique_ptr<DpmsManager> m_manager;
};

} // namespace

#endif // KSCREEN_DPSMCLIENT_H
