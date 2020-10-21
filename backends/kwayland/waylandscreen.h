/*************************************************************************************
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
#pragma once

#include "abstractbackend.h"
#include "config.h"
#include "screen.h"

#include <QObject>
#include <QSize>

namespace KScreen
{
class WaylandConfig;
class WaylandOutput;

class WaylandScreen : public QObject
{
    Q_OBJECT

public:
    explicit WaylandScreen(WaylandConfig *config);
    ~WaylandScreen() override = default;

    KScreen::ScreenPtr toKScreenScreen(KScreen::ConfigPtr &parent) const;
    void updateKScreenScreen(KScreen::ScreenPtr &screen) const;
    void setOutputs(const QList<WaylandOutput*> &outputs);

    void setOutputCount(int count);

private:
    QSize m_size;
    int m_outputCount;
};

}
