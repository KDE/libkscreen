/*
 *  SPDX-FileCopyrightText: 2014-2015 Sebastian Kügler <sebas@kde.org>
 *  SPDX-FileCopyrightText: 2021 Méven Car <meven.car@enioka.com>
 *
 *  SPDX-License-Identifier: LGPL-2.1-or-later
 */
#include "waylandoutputmanagement.h"
#include <QDebug>

namespace KScreen
{
WaylandOutputManagement::WaylandOutputManagement(int version)
    : QWaylandClientExtensionTemplate<WaylandOutputManagement>(version)
{
    connect(this, &WaylandOutputManagement::activeChanged, this, [this]() {
        if (!isActive()) {
            kde_output_management_v2_destroy(object());
        }
    });
    initialize();
}

WaylandOutputManagement::~WaylandOutputManagement()
{
    if (isActive()) {
        kde_output_management_v2_destroy(object());
    }
}

WaylandOutputConfiguration *WaylandOutputManagement::createConfiguration()
{
    if (isActive()) {
        return new WaylandOutputConfiguration(create_configuration());
    } else {
        return nullptr;
    }
}

WaylandOutputConfiguration::WaylandOutputConfiguration(struct ::kde_output_configuration_v2 *object)
    : QObject()
    , QtWayland::kde_output_configuration_v2()
{
    init(object);
}

void WaylandOutputConfiguration::kde_output_configuration_v2_applied()
{
    Q_EMIT applied();
}

void WaylandOutputConfiguration::kde_output_configuration_v2_failed()
{
    Q_EMIT failed(m_errorMessage);
}

void WaylandOutputConfiguration::kde_output_configuration_v2_failure_reason(const QString &reason)
{
    m_errorMessage = reason;
}

WaylandOutputOrder::WaylandOutputOrder(struct ::wl_registry *registry, int id, int version)
    : QtWayland::kde_output_order_v1(registry, id, version)
{
}

WaylandOutputOrder::~WaylandOutputOrder()
{
    destroy();
}

QList<QString> WaylandOutputOrder::order() const
{
    return m_outputOrder;
}

void WaylandOutputOrder::kde_output_order_v1_output(const QString &output_name)
{
    m_pendingOutputOrder.push_back(output_name);
}

void WaylandOutputOrder::kde_output_order_v1_done()
{
    m_outputOrder = m_pendingOutputOrder;
    Q_EMIT outputOrderChanged(m_pendingOutputOrder);
    m_pendingOutputOrder.clear();
}
}

#include "moc_waylandoutputmanagement.cpp"
