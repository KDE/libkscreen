/*************************************************************************************
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

#ifndef QSCREEN_SCREEN_H
#define QSCREEN_SCREEN_H

#include "config.h"
#include "screen.h"

#include <QScreen>
#include <QSize>
#include <QLoggingCategory>

namespace KScreen
{
class Output;
class QScreenOutput;

class QScreenScreen : public QObject
{
    Q_OBJECT

public:
    explicit QScreenScreen(QScreenConfig *config);
    ~QScreenScreen() override;

    KScreen::ScreenPtr toKScreenScreen() const;
    void updateKScreenScreen(KScreen::ScreenPtr &screen) const;
};

} // namespace

#endif // QSCREEN_SCREEN_H
