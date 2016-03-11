/*************************************************************************************
 *  Copyright (C) 2012 by Alejandro Fiestas Olivares <afiestas@kde.org>              *
 *  Copyright (C) 2012, 2013 by Daniel Vrátil <dvratil@redhat.com>                   *
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

#ifndef KWAYLAND_BACKEND_H
#define KWAYLAND_BACKEND_H

#include "abstractbackend.h"

#include <QLoggingCategory>

namespace KScreen
{
class WaylandConfig;

class WaylandBackend : public KScreen::AbstractBackend
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.kf5.kscreen.backends.kwayland")

public:
    explicit WaylandBackend();
    virtual ~WaylandBackend() = default;

    QString name() const Q_DECL_OVERRIDE;
    QString serviceName() const Q_DECL_OVERRIDE;
    KScreen::ConfigPtr config() const Q_DECL_OVERRIDE;
    void setConfig(const KScreen::ConfigPtr &config) Q_DECL_OVERRIDE;
    bool isValid() const Q_DECL_OVERRIDE;
    QByteArray edid(int outputId) const Q_DECL_OVERRIDE;

    void updateConfig(KScreen::ConfigPtr &config);

private:
    bool m_isValid;
    KScreen::ConfigPtr m_config;
    WaylandConfig *m_internalConfig;
    void emitConfigChanged(const KScreen::ConfigPtr &cfg);
};

} // namespace

Q_DECLARE_LOGGING_CATEGORY(KSCREEN_WAYLAND)

#endif //KWAYLAND_BACKEND_H
