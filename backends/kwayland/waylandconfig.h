/*
 *  SPDX-FileCopyrightText: 2014-2015 Sebastian Kügler <sebas@kde.org>
 *
 *  SPDX-License-Identifier: LGPL-2.1-or-later
 */
#pragma once

#include "abstractbackend.h"
#include "config.h"

#include <QDir>
#include <QEventLoop>
#include <QLoggingCategory>
#include <QScreen>
#include <QSize>
#include <QSocketNotifier>

namespace KWayland
{
namespace Client
{
class ConnectionThread;
class EventQueue;
class Registry;
class OutputManagement;
}
}

namespace KScreen
{
class Output;
class WaylandOutputDevice;
class WaylandScreen;
class WaylandOutputManagement;

/**
 * @class WaylandConfig
 *
 * This class holds the basic skeleton of the configuration and takes care of
 * fetching the information from the Wayland server and synchronizing the
 * configuration out to the "clients" that receive the config from the backend.
 * We initialize a wayland connection, using a threaded event queue when
 * querying the wayland server for data.
 * Initially, the creation of a WaylandConfig blocks until all data has been
 * received, signalled by the initialized() signal. This means that the
 * wayland client has received information about all interfaces, and that all
 * outputs are completely initialized. From then on, we properly notifyUpdate().
 */
class WaylandConfig : public QObject
{
    Q_OBJECT

public:
    explicit WaylandConfig(QObject *parent = nullptr);
    ~WaylandConfig() override;

    KScreen::ConfigPtr currentConfig();
    QMap<int, WaylandOutputDevice *> outputMap() const;

    void applyConfig(const KScreen::ConfigPtr &newConfig);

    bool isReady() const;

Q_SIGNALS:
    void configChanged();
    void initialized();

private:
    void setupRegistry();
    void checkInitialized();
    void disconnected();

    void initKWinTabletMode();
    void initConnection();

    void addOutput(quint32 name, quint32 version);
    void removeOutput(WaylandOutputDevice *output);

    void blockSignals();
    void unblockSignals();
    void tryPendingConfig();

    KWayland::Client::ConnectionThread *m_connection;

    KWayland::Client::Registry *m_registry;
    WaylandOutputManagement *m_outputManagement = nullptr;

    // KWayland names as keys
    QMap<int, WaylandOutputDevice *> m_outputMap;

    // KWayland names
    QList<WaylandOutputDevice *> m_initializingOutputs;
    int m_lastOutputId = -1;

    bool m_registryInitialized;
    bool m_blockSignals;
    QEventLoop m_syncLoop;
    KScreen::ConfigPtr m_kscreenConfig;
    KScreen::ConfigPtr m_kscreenPendingConfig;
    WaylandScreen *m_screen;

    bool m_tabletModeAvailable;
    bool m_tabletModeEngaged;
    bool m_initialized = false;
};

}
