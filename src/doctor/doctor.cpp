/*************************************************************************************
 *  Copyright 2014-2015 Sebastian Kügler <sebas@kde.org>                             *
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

#include "doctor.h"

#include <QDebug>

#include <QCommandLineParser>
#include <QDateTime>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QFile>
#include <QStandardPaths>
#include "../config.h"
#include "../configoperation.h"
#include "../getconfigoperation.h"
#include "../edid.h"

static QTextStream cout(stdout);
static QTextStream cerr(stderr);

namespace KScreen
{
namespace ConfigSerializer
{
// Exported private symbol in configserializer_p.h in KScreen
extern QJsonObject serializeConfig(const KScreen::ConfigPtr &config);
}
}


using namespace KScreen;

Doctor::Doctor(QObject *parent)
    : QObject(parent)
    , m_config(nullptr)
{
    cout << "*********************";
}

Doctor::~Doctor()
{
}

void Doctor::start(QCommandLineParser *parser)
{
    m_parser = parser;
    qDebug() << "START: Requesting Config";
    cout << "$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$";
    KScreen::GetConfigOperation *op = new KScreen::GetConfigOperation();
    QObject::connect(op, &KScreen::GetConfigOperation::finished, this,
                     [&](KScreen::ConfigOperation *op) {
                        qDebug() << "cfg returns";
                        configReceived(op);
                      });
}

void Doctor::configReceived(KScreen::ConfigOperation *op)
{
    cout << "Config received";
    m_config = op->config();

    if (m_parser->isSet("json")) {
        showJson();

    }
    qApp->quit();
}



int Doctor::outputCount() const
{
    return 0;
}

void Doctor::showOutputs()
{
    cout << "show outputs";
}

void Doctor::showJson()
{
    QJsonDocument doc(KScreen::ConfigSerializer::serializeConfig(m_config));
    cout << doc.toJson(QJsonDocument::Indented);

}

/*
QString Doctor::modeString(KWayland::Server::OutputDeviceInterface* outputdevice, int mid)
{
    QString s;
    QString ids;
    int _i = 0;
    Q_FOREACH (const auto &_m, outputdevice->modes()) {
        _i++;
        if (_i < 6) {
            ids.append(QString::number(_m.id) + ", ");
        } else {
            ids.append(".");
        }
        if (_m.id == mid) {
            s = QString("%1x%2 @%3").arg(QString::number(_m.size.width()), \
            QString::number(_m.size.height()), QString::number(_m.refreshRate));
        }
    }
    return QString("[%1] %2 (%4 modes: %3)").arg(QString::number(mid), s, ids, QString::number(outputdevice->modes().count()));

}
*/
