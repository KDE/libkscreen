/*************************************************************************************
 *  Copyright (C) 2012 by Alejandro Fiestas Olivares <afiestas@kde.org>              *
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
        static void qvariant2qobject(const QVariantMap& variant, QObject* object);
        static KScreen::Screen* screenFromJson(const QMap<QString, QVariant>& data);
        static KScreen::Output* outputFromJson(const QVariantMap& map);
        static KScreen::Mode* modeFromJson(const QVariant& data);
        static QSize sizeFromJson(const QVariant& data);
        static QRect rectFromJson(const QVariant& data);
        static QPoint pointFromJson(const QVariant& data);
};

#endif //PARSER_H
