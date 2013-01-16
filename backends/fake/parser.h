/*************************************************************************************
 *  Copyright (C) 2012 by Alejandro Fiestas Olivares <afiestas@kde.org>              *
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

#ifndef PARSER_H
#define PARSER_H

#include <QtCore/QByteArray>
#include <QtCore/QString>
#include <QtCore/QVariant>
#include <QtCore/QSize>
#include <QtCore/QRect>
#include <QtCore/QPoint>

namespace KScreen {
    class Config;
    class Screen;
    class Output;
    class Mode;
}
class Parser
{
    public:
        static KScreen::Config* fromJson(const QByteArray &data);
        static KScreen::Config* fromJson(const QString &path);
        static bool validate(const QByteArray &data);
        static bool validate(const QString &data);

    private:
        static KScreen::Screen* screenFromJson(const QMap<QString, QVariant>& data);
        static KScreen::Output* outputFromJson(const QVariant& data);
        static KScreen::Mode* modeFromJson(const QVariant& data);
        static QSize sizeFromJson(const QVariant& data);
        static QRect rectFromJson(const QVariant& data);
        static QPoint pointFromJson(const QVariant& data);
};

#endif //PARSER_H
