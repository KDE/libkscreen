/*************************************************************************************
 *  Copyright 2014-2015 by Sebastian Kügler <sebas@kde.org>                          *
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

#include "doctor.h"

#include <QCoreApplication>
#include <QCommandLineParser>

#include <QDebug>


int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);

    KScreen::Doctor server;

    QCommandLineOption config = QCommandLineOption(QStringList() << QStringLiteral("c") << "config",
                                                  QStringLiteral("Config file"), "config");
    QCommandLineOption json = QCommandLineOption(QStringList() << QStringLiteral("j") << "json", QStringLiteral("Json output"), "jsonfile");
    QCommandLineParser parser;
    parser.addHelpOption();
    parser.addOption(config);
    parser.addOption(json);
    parser.process(app);

    server.start(&parser);
    return app.exec();
}
