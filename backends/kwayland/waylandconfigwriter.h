/*************************************************************************************
 *  Copyright 2015 Sebastian Kügler <sebas@kde.org>                                  *
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

#ifndef KSCREEN_WAYLAND_CONFIGWRITER_H
#define KSCREEN_WAYLAND_CONFIGWRITER_H

#include <QObject>
#include <QRect>

// libkscreen
#include "screen.h"

// KWayland
#include <KWayland/Server/display.h>
#include <KWayland/Server/output_interface.h>

namespace KScreen
{

class WaylandConfigWriter
{

public:
    static bool writeJson(const KScreen::ConfigPtr& config, const QString& configfile);
    static bool writeConfig(const KScreen::ConfigPtr& config, const QString& configfile);
};

} // namespace

#endif // KSCREEN_WAYLAND_CONFIGWRITER_H
