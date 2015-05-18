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

#ifndef KSCREEN_DRM_BACKEND_H
#define KSCREEN_DRM_BACKEND_H

#include <QObject>


namespace KScreen
{
class WaylandConfig;
class WaylandOutput;

class DrmBackend : public QObject
{
    Q_OBJECT

public:
    explicit DrmBackend(QObject *parent = 0);
    virtual ~DrmBackend();

    void start();
};

} // namespace

#endif // KSCREEN_DRM_BACKEND_H
