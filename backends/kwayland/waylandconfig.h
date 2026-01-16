/*
 *  SPDX-FileCopyrightText: 2014-2015 Sebastian Kügler <sebas@kde.org>
 *
 *  SPDX-License-Identifier: LGPL-2.1-or-later
 */
#pragma once

#include "abstractbackend.h"

#include <QFuture>
#include <QObject>

struct kde_output_device_v2;
struct wl_registry;
struct wl_fixes;

namespace KScreen
{
class WaylandOutputConfiguration;
class WaylandOutputDevice;
class WaylandOutputManagement;

struct WaylandConfigApplyOperation {
    KScreen::ConfigPtr config;
    std::unique_ptr<WaylandOutputConfiguration> request;
    QPromise<SetConfigResult> promise;
};

class WaylandConfig : public QObject
{
    Q_OBJECT

public:
    explicit WaylandConfig(QObject *parent = nullptr);
    ~WaylandConfig();

    KScreen::ConfigPtr currentConfig();
    QMap<int, WaylandOutputDevice *> outputMap() const;

    QFuture<SetConfigResult> applyConfig(const KScreen::ConfigPtr &newConfig);
    WaylandOutputDevice *findOutputDevice(struct ::kde_output_device_v2 *outputdevice) const;

    bool isValid() const;

Q_SIGNALS:
    void configChanged();

private:
    void setupRegistry();
    void destroyRegistry();
    void handleActiveChanged();

    void initKWinTabletMode();

    void addOutput(quint32 name, quint32 version);
    void removeOutput(quint32 name);

    QFuture<SetConfigResult> apply(std::unique_ptr<WaylandConfigApplyOperation> &&operation);
    void tryPendingConfig();

    wl_registry *m_registry = nullptr;
    wl_fixes *m_fixes = nullptr;

    std::unique_ptr<WaylandOutputManagement> m_outputManagement;

    // KWayland names as keys
    QMap<int, WaylandOutputDevice *> m_outputMap;

    // KWayland names
    QList<WaylandOutputDevice *> m_initializingOutputs;

    KScreen::ConfigPtr m_config;
    std::unique_ptr<WaylandConfigApplyOperation> m_currentOperation;
    std::unique_ptr<WaylandConfigApplyOperation> m_pendingOperation;

    bool m_tabletModeAvailable = false;
    bool m_tabletModeEngaged = false;
};

}
