/*************************************************************************************
 *  Copyright 2018   Daniel Vrátil <dvratil@kde.org>                                 *
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

#include "utils.h"

#include <QVector>

KScreen::Output::Type Utils::guessOutputType(const QString &type, const QString &name)
{
    static const auto embedded = { QLatin1String("LVDS"),
                                   QLatin1String("IDP"),
                                   QLatin1String("EDP"),
                                   QLatin1String("LCD") };

    for (const QLatin1String &pre : embedded) {
        if (name.toUpper().startsWith(pre)) {
            return KScreen::Output::Panel;
        }
    }

    if (type.contains("VGA")) {
        return KScreen::Output::VGA;
    } else if (type.contains("DVI")) {
        return KScreen::Output::DVI;
    } else if (type.contains("DVI-I")) {
        return KScreen::Output::DVII;
    } else if (type.contains("DVI-A")) {
        return KScreen::Output::DVIA;
    } else if (type.contains("DVI-D")) {
        return KScreen::Output::DVID;
    } else if (type.contains("HDMI")) {
        return KScreen::Output::HDMI;
    } else if (type.contains("Panel")) {
        return KScreen::Output::Panel;
    } else if (type.contains("TV-Composite")) {
        return KScreen::Output::TVComposite;
    } else if (type.contains("TV-SVideo")) {
        return KScreen::Output::TVSVideo;
    } else if (type.contains("TV-Component")) {
        return KScreen::Output::TVComponent;
    } else if (type.contains("TV-SCART")) {
        return KScreen::Output::TVSCART;
    } else if (type.contains("TV-C4")) {
        return KScreen::Output::TVC4;
    } else if (type.contains("TV")) {
        return KScreen::Output::TV;
    } else if (type.contains("DisplayPort") || type.startsWith("DP")) {
        return KScreen::Output::DisplayPort;
    } else if (type.contains("unknown")) {
        return KScreen::Output::Unknown;
    } else {
        return KScreen::Output::Unknown;
    }
}

