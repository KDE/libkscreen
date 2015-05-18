/*************************************************************************************
 *  Copyright 2015 Sebastian Kügler <sebas@kde.org>                                  *
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

#include "waylandconfigwriter.h"
#include <configserializer_p.h>

#include "output.h"
#include "config.h"
#include <QDebug>

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QFile>
#include <QFileInfo>
#include <QStandardPaths>

using namespace KScreen;
bool WaylandConfigWriter::writeConfig(const ConfigPtr& config, const QString& configfile)
{
    QString _all;
    for (auto output: config->outputs()) {
        if (!output->isEnabled() || !output->isConnected()) {
            continue;
        }
        QString _o;
        _all.append(QString("[%1]\n").arg(output->name()));
        const int x = output->pos().x();
        const int y = output->pos().y();
        int width = -1;
        int height = -1;
        float refreshRate = -1;
        for (auto mode: output->modes()) {
            if (mode->id() == output->currentModeId()) {
                width = mode->size().width();
                height = mode->size().height();
                refreshRate = mode->refreshRate();
            }
        }
        int refresh = qRound(refreshRate * 1000); // this is what KWayland deals with
        _all.append(QString("x=%1\n").arg(QString::number(x)));
        _all.append(QString("y=%1\n").arg(QString::number(y)));
        _all.append(QString("width=%1\n").arg(QString::number(width)));
        _all.append(QString("height=%1\n").arg(QString::number(height)));
        _all.append(QString("refreshRate=%1\n").arg(QString::number(refreshRate)));
        _all.append("\n");
    }
    qDebug() << "CONFIG" << _all;

    QString destfile = configfile;
    const QFileInfo fi(configfile);
    if (!fi.isAbsolute()) {
        destfile = QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation) + QLatin1Char('/') + configfile;

    }
    QFile file(destfile);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qWarning() << "Failed to open " << destfile;
        return false;
    }
    qDebug() << "destfile" << destfile;
    file.write(_all.toLocal8Bit());
    return true;
}
/* We could use QSettings, but this produces "unclean" output files
    QSettings settings(configfile, QSettings::IniFormat);

    qDebug() << "writing config to " << configfile;
    foreach (auto output, config->outputs()) {
        settings.beginGroup(output->name());
        settings.setValue("ID", output->id());
        settings.setValue(QStringLiteral("id"), output->id());
        settings.setValue(QStringLiteral("width"), "1920");
        settings.setValue(QStringLiteral("height"), "1080");
        settings.setValue(QStringLiteral("x"), "4");
        settings.setValue(QStringLiteral("y"), "7");
    }
*/

bool WaylandConfigWriter::writeJson(const ConfigPtr& config, const QString& configfile)
{
    QJsonDocument jdoc;
    QJsonObject jo = KScreen::ConfigSerializer::serializeConfigMinimal(config);
    jdoc.setObject(jo);

    QString destfile = configfile;
    const QFileInfo fi(configfile);
    if (!fi.isAbsolute()) {
        destfile = QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation) + QLatin1Char('/') + configfile;

    }
    QFile file(destfile);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qWarning() << "Failed to open " << destfile;
        return false;
    }

    file.write(jdoc.toJson());
    //file.write(jdoc.toBinaryData());
    qDebug() << "JSON: " << jdoc.toJson();
    qWarning() << "Generated " << destfile;
    return true;
}
