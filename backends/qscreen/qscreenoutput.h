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

#ifndef QSCREEN_OUTPUT_H
#define QSCREEN_OUTPUT_H

#include "qscreenconfig.h"

#include "config.h"
#include "output.h"
#include "edid.h"

#include <QScreen>

namespace KScreen
{

class QScreenOutput : public QObject
{
    Q_OBJECT

public:
    explicit QScreenOutput(const QScreen *qscreen, QObject *parent = nullptr);
    ~QScreenOutput() override;

    KScreen::OutputPtr toKScreenOutput() const;
    void updateKScreenOutput(KScreen::OutputPtr &output) const;

    int id() const;
    void setId(const int newId);

    const QScreen *qscreen() const;

private:
    const QScreen *m_qscreen;
    int m_id;
};

} // namespace

#endif
