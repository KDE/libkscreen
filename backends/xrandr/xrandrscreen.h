/*************************************************************************************
 *  Copyright (C) 2012 by Dan Vrátil <dvratil@redhat.com>                            *
 *                                                                                   *
 *  This program is free software; you can redistribute it and/or                    *
 *  modify it under the terms of the GNU General Public License                      *
 *  as published by the Free Software Foundation; either version 2                   *
 *  of the License, or (at your option) any later version.                           *
 *                                                                                   *
 *  This program is distributed in the hope that it will be useful,                  *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of                   *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                    *
 *  GNU General Public License for more details.                                     *
 *                                                                                   *
 *  You should have received a copy of the GNU General Public License                *
 *  along with this program; if not, write to the Free Software                      *
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA   *
 *************************************************************************************/

#ifndef XRANDRSCREEN_H
#define XRANDRSCREEN_H

#include <QObject>
#include <QSize>

class XRandRConfig;
namespace KScreen
{
class Screen;
class Config;
}

class XRandRScreen : public QObject
{
    Q_OBJECT

public:
    explicit XRandRScreen(XRandRConfig *config = 0);
    virtual ~XRandRScreen();

    KScreen::Screen *toKScreenScreen(KScreen::Config *parent) const;
    void updateKScreenScreen(KScreen::Screen *screen) const;

    void update();
    QSize currentSize();

private:
    int m_id;
    QSize m_minSize;
    QSize m_maxSize;
    QSize m_currentSize;
};

#endif // XRANDRSCREEN_H
