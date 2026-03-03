/*
 *  SPDX-FileCopyrightText: 2014-2015 Sebastian Kügler <sebas@kde.org>
 *
 *  SPDX-License-Identifier: LGPL-2.1-or-later
 */
#pragma once

#include "abstractbackend.h"
#include "config.h"

#include <QDir>
#include <QLoggingCategory>
#include <QScreen>
#include <QSize>
#include <QSocketNotifier>

namespace KScreen
{
class Output;
class WaylandOutputDevice;
class WaylandScreen;
class WaylandOutputDeviceRegistry;
class WaylandOutputManagement;

class WaylandConfig : public QObject
{
    Q_OBJECT

public:
    explicit WaylandConfig(QObject *parent = nullptr);
    ~WaylandConfig();

    KScreen::ConfigPtr currentConfig();
    QMap<int, WaylandOutputDevice *> outputMap() const;

    bool applyConfig(const KScreen::ConfigPtr &newConfig);

    bool isValid() const;

Q_SIGNALS:
    void configChanged();
    void globalRemoved(uint32_t name);
    void configFailed(const QString &reason);

private:
    void handleActiveChanged();

    void initKWinTabletMode();

    void addOutput(WaylandOutputDevice *output);
    void removeOutput(WaylandOutputDevice *output);

    void blockSignals();
    void unblockSignals();
    void tryPendingConfig();

    std::unique_ptr<WaylandOutputManagement> m_outputManagement;
    std::unique_ptr<WaylandOutputDeviceRegistry> m_outputRegistry;

    // KWayland names as keys
    QMap<int, WaylandOutputDevice *> m_outputMap;

    // KWayland names
    QList<WaylandOutputDevice *> m_initializingOutputs;
    int m_lastOutputId = -1;

    bool m_blockSignals;
    KScreen::ConfigPtr m_kscreenConfig;
    KScreen::ConfigPtr m_kscreenPendingConfig;
    WaylandScreen *m_screen;

    bool m_tabletModeAvailable;
    bool m_tabletModeEngaged;
};

}
