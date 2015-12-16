/*************************************************************************************
 *  Copyright (C) 2012 by Alejandro Fiestas Olivares <afiestas@kde.org>              *
 *  Copyright (C) 2012, 2013 by Daniel Vrátil <dvratil@redhat.com>                   *
 *  Copyright 2014 Sebastian Kügler <sebas@kde.org>                                  *
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

#ifndef WAYLAND_BACKEND_H
#define WAYLAND_BACKEND_H

#include "abstractbackend.h"

#include <QSize>
#include <QLoggingCategory>

namespace KScreen
{
class Output;
class WaylandConfig;

class WaylandBackend : public KScreen::AbstractBackend
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.kf5.kscreen.backends.wayland")

public:
    explicit WaylandBackend();
    virtual ~WaylandBackend();

    static WaylandConfig *internalConfig();

    virtual QString name() const;
    virtual QString serviceName() const;
    virtual KScreen::ConfigPtr config() const;
    virtual void setConfig(const KScreen::ConfigPtr &config);
    virtual bool isValid() const;
    virtual QByteArray edid(int outputId) const;
    virtual void updateConfig(KScreen::ConfigPtr &config);

private:
    bool m_isValid;
    static WaylandConfig *s_internalConfig;
    KScreen::ConfigPtr m_config;
    void emitConfigChanged(const KScreen::ConfigPtr cfg);
};


} // namespace

Q_DECLARE_LOGGING_CATEGORY(KSCREEN_WAYLAND)

#endif //WAYLAND_BACKEND_H
