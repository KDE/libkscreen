/*
 *  SPDX-FileCopyrightText: 2014-2015 Sebastian Kügler <sebas@kde.org>
 *  SPDX-FileCopyrightText: 2021 Méven Car <meven.car@enioka.com>
 *
 *  SPDX-License-Identifier: LGPL-2.1-or-later
 */
#pragma once

#include "qwayland-kde-output-management-v2.h"

#include <QObject>
#include <QSize>
#include <QWaylandClientExtension>

namespace KScreen
{
class WaylandConfig;
class WaylandOutputDevice;

class WaylandOutputConfiguration : public QObject, public QtWayland::kde_output_configuration_v2
{
    Q_OBJECT
public:
    WaylandOutputConfiguration(struct ::kde_output_configuration_v2 *object);
    ~WaylandOutputConfiguration() override;

Q_SIGNALS:
    void applied();
    void failed(const QString &errorMessage);

protected:
    void kde_output_configuration_v2_applied() override;
    void kde_output_configuration_v2_failed() override;
    void kde_output_configuration_v2_failure_reason(const QString &reason) override;

    QString m_errorMessage;
};

class WaylandOutputManagement : public QWaylandClientExtensionTemplate<WaylandOutputManagement>, public QtWayland::kde_output_management_v2
{
    Q_OBJECT
public:
    explicit WaylandOutputManagement(int version);
    ~WaylandOutputManagement() override;

    std::unique_ptr<WaylandOutputConfiguration> createConfiguration();
};
}
