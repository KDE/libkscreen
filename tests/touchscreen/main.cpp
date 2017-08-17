/*************************************************************************************
 *  Copyright 2017 by Sebastian Kügler <sebas@kde.org>                               *
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

#include "tstool.h"

#include <QGuiApplication>
#include <QCommandLineParser>

#include <QDebug>


int main(int argc, char **argv)
{
    const QString desc = "kscreen-touchscreen allows to manipulate touchscreens.\n";
    const QString syntax = "\n";

    QGuiApplication app(argc, argv);

    KScreen::TsTool tstool;

    QCommandLineOption list = QCommandLineOption(QStringList() << QStringLiteral("l") << "list",
                                                  QStringLiteral("List available touchscreen devices."));
    QCommandLineOption rotate = QCommandLineOption(QStringList() << QStringLiteral("r") << "rotate",
                                                  QStringLiteral("rotate touchscreen"), QStringLiteral("none"));

    QCommandLineParser parser;
    parser.setApplicationDescription(desc);
    parser.addPositionalArgument("config", syntax, QString());
    parser.addHelpOption();
    parser.addOption(list);
    parser.addOption(rotate);
    parser.process(app);

    if (!parser.positionalArguments().isEmpty()) {
        tstool.setOptionList(parser.positionalArguments());
    }

    tstool.start(&parser);
    return app.exec();
}
