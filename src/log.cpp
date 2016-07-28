/*************************************************************************************
 *  Copyright 2016 by Sebastian Kügler <sebas@kde.org>                               *
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

#include "log.h"

#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QStandardPaths>

namespace KScreen {

Log* Log::sInstance = nullptr;
QtMessageHandler sDefaultMessageHandler = nullptr;

void kscreenLogOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    QByteArray localMsg = msg.toLocal8Bit();
    if (QString::fromLocal8Bit(context.category).startsWith(QLatin1String("kscreen"))) {
        Log::log(localMsg.constData(), context.category);
    }
    sDefaultMessageHandler(type, context, msg);
}

void log(const QString& msg)
{
    Log::log(msg);
}

Log* Log::instance()
{
    if (!sInstance) {
        sInstance = new Log();
    }

    return sInstance;
}

using namespace KScreen;
class Log::Private
{
  public:
      QString context;
      bool enabled = true;
      QString logFile;

};

Log::Log() :
   d(new Private)
{
    const char* logging_env = "KSCREEN_LOGGING";
    const char* logfile_env = "KSCREEN_LOGFILE";

    if (qEnvironmentVariableIsSet(logging_env)) {
        const QString logging_env_value = qgetenv(logging_env).constData();
        if (logging_env_value == QStringLiteral("0") || logging_env_value.toLower() == QStringLiteral("false")) {
            d->enabled = false;
        }
    }
    if (qEnvironmentVariableIsSet(logfile_env)) {
        const auto logfile_env_value = qgetenv(logfile_env).constData();
        // todo : checks path exists, writable
        d->logFile = QString::fromLocal8Bit(logfile_env_value);
    } else {
        d->logFile = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + "/kscreen/kscreen.log";
    }
    if (!d->enabled) {
        return;
    }
    QLoggingCategory::setFilterRules("kscreen.*=true");
    // todo : create path if necessary
    QFileInfo fi(d->logFile);
    if (!QDir().mkpath(fi.absolutePath())) {
        qWarning() << "Failed to create logging dir" << fi.absolutePath();
    }

    if (!sDefaultMessageHandler) {
        sDefaultMessageHandler = qInstallMessageHandler(kscreenLogOutput);
        qDebug() << "installed message handler, logging to " << d->logFile;
    }
}

Log::Log(Log::Private *dd) :
   d(dd)
{
}

Log::~Log()
{
    delete d;
    delete sInstance;
    sInstance = nullptr;
}

QString Log::context() const
{
    return d->context;
}

void Log::setContext(const QString& context)
{
    d->context = context;
}

bool Log::enabled() const
{
    return d->enabled;
}

QString Log::logFile() const
{
    return d->logFile;
}

void Log::log(const QString &msg, const QString &category)
{
    auto _cat = category;
    _cat.remove("kscreen.");
    const QString timestamp = QString::number(QDateTime::currentDateTime().toMSecsSinceEpoch());
    QString logMessage = QString("\n%1 ; %2 ; %3 : %4").arg(timestamp, _cat, instance()->context(), msg);
    QFile file(instance()->logFile());
    if (!file.open(QIODevice::Append | QIODevice::Text)) {
        return;
    }
    file.write(logMessage.toUtf8());
}

} // ns
