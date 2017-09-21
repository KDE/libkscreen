/*************************************************************************************
 *  Copyright 2017 Sebastian Kügler <sebas@kde.org>                                  *
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

#ifndef KSCREEN_TSTOOL_H
#define KSCREEN_TSTOOL_H

#include <QCommandLineParser>
#include <QObject>

#include "xtouchscreen.h"

namespace KScreen
{

class TsTool : public QObject
{
    Q_OBJECT

public:
    explicit TsTool(QObject *parent = 0);
    virtual ~TsTool();

    void setOptionList(const QStringList &positionalArgs);
    void start(QCommandLineParser *m_parser);

    void listTouchscreens();


private:
    void rotateTouchscreen(int ts_id, const QString& rotation_string);
    int parseInt(const QString &str, bool &ok) const;
    QCommandLineParser* m_parser;
    QStringList m_positionalArgs;

    XTouchscreenList m_touchscreens;
};

} // namespace

#endif // KSCREEN_TSTOOL_H
