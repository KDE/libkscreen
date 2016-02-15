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

/** Usage example:
 * kscreen-doctor --set output.0.disable output.1.mode.1 output.1.enable"
 */

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);

    KScreen::Doctor server;

    QCommandLineOption outputs = QCommandLineOption(QStringList() << QStringLiteral("o") << "outputs",
                                                  QStringLiteral("Show outputs"));
    QCommandLineOption json = QCommandLineOption(QStringList() << QStringLiteral("j") << "json",
                                                 QStringLiteral("Show configuration in JSON format"));
    QCommandLineOption enable = QCommandLineOption(QStringList() << QStringLiteral("e") << "enable",
                                                 QStringLiteral("Output id"), "output_id");
    QCommandLineOption disable = QCommandLineOption(QStringList() << QStringLiteral("d") << "disable",
                                                 QStringLiteral("Output id"), "output_id");
    QCommandLineParser parser;
    parser.addHelpOption();
    parser.addOption(outputs);
    parser.addOption(json);
    parser.addOption(enable);
    parser.addOption(disable);
    parser.process(app);

    if (!parser.positionalArguments().isEmpty()) {
        server.setOptionList(parser.positionalArguments());
    }

    server.start(&parser);
    return app.exec();
}
