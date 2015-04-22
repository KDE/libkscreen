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

#include "output.h"
#include "config.h"
#include <QDebug>

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QFile>
#include <QFileInfo>

using namespace KScreen;

bool WaylandConfigWriter::write(const ConfigPtr& config, const QString& configfile)
{
    QJsonArray plugins;


    foreach (auto output, config->outputs()) {
                qDebug() << " _____________________ Output: " << output;
                qDebug() << "   output name: " << output->name();
                qDebug() << "   output modes: " << output->modes().count() << output->modes();
                qDebug() << "   output enabled: " << output->isEnabled();
                qDebug() << "   output connect: " << output->isConnected();
                qDebug() << "   output sizeMm : " << output->sizeMm();
//         QVERIFY(!output->name().isEmpty());
//         QVERIFY(output->id() > -1);
//         QVERIFY(output->isConnected());
//         QVERIFY(output->isEnabled());
//         QVERIFY(output->geometry() != QRectF(1,1,1,1));
//         QVERIFY(output->geometry() != QRectF());
//         QVERIFY(output->sizeMm() != QSize());
//         QVERIFY(output->edid() != 0);
//         QCOMPARE(output->rotation(), Output::None);
//         QVERIFY(!ids.contains(output->id()));
//         ids << output->id();

//         int currentModeId = outputConfig["currentModeId"].toInt();
//         QVariantList preferredModes = outputConfig["preferredModes"].toList();
//
//         Q_FOREACH(const QVariant &_mode, outputConfig["modes"].toList()) {
//             const QVariantMap &mode = _mode.toMap();
//             const QSize _size = sizeFromJson(mode["size"]);
//             int refresh = 60000;
//
//             if (outputConfig.keys().contains("refreshRate")) {
//                 refresh = qRound(outputConfig["refreshRate"].toReal() * 1000);
//             }
//             bool isCurrent = currentModeId == mode["id"].toInt();
//             bool isPreferred = preferredModes.contains(mode["id"]);
//         }
        QJsonObject o;
        o["name"] = output->name();
        o["id"] = output->id();
        o["connected"] = output->isConnected();
        o["enabled"] = output->isEnabled();

        plugins << o;
    }

    QJsonDocument jdoc;
    QJsonObject jo;
    jo["outputs"] = plugins;
    jdoc.setObject(jo);

    QString destfile = configfile;
    const QFileInfo fi(configfile);
    if (!fi.isAbsolute()) {
        //destfile = dir + '/' + dest;
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

QVariantMap WaylandConfigWriter::pointToJson(const QPoint& point)
{
    QVariantMap m;
    m["x"] = point.x();
    m["y"] = point.y();
    return m;
}

QVariantMap WaylandConfigWriter::rectToJson(const QRect& rect)
{
    QVariantMap m;
    m["x"] = rect.x();
    m["y"] = rect.y();
    m["width"] = rect.width();
    m["height"] = rect.height();
    return m;
}

QVariantMap WaylandConfigWriter::sizeToJson(const QSize& size)
{
    QVariantMap m;
    m["width"] = size.width();
    m["height"] = size.height();
    return m;
}

