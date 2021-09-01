/*
 *  SPDX-FileCopyrightText: 2014-2015 Sebastian Kügler <sebas@kde.org>
 *  SPDX-FileCopyrightText: 2021 Méven Car <meven.car@enioka.com>
 *
 *  SPDX-License-Identifier: LGPL-2.1-or-later
 */
#ifndef WAYLANDOUTPUTMANAGEMENT_H
#define WAYLANDOUTPUTMANAGEMENT_H

#include "qwayland-kde-output-management-v2.h"

#include <QObject>
#include <QSize>

namespace KScreen
{
class WaylandOutputConfiguration : public QObject, public QtWayland::kde_output_configuration_v2
{
    Q_OBJECT
public:
    WaylandOutputConfiguration(struct ::kde_output_configuration_v2 *object);

Q_SIGNALS:
    void applied();
    void failed();

protected:
    void kde_output_configuration_v2_applied() override;
    void kde_output_configuration_v2_failed() override;
};

class WaylandOutputManagement : public QObject, public QtWayland::kde_output_management_v2
{
    Q_OBJECT
public:
    WaylandOutputManagement(struct ::wl_registry *registry, int id, int version);

    WaylandOutputConfiguration *createConfiguration();
};

}

#endif // WAYLANDOUTPUTMANAGEMENT_H
