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

namespace KScreen
{
Log *Log::sInstance = nullptr;
QtMessageHandler sDefaultMessageHandler = nullptr;

void kscreenLogOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    auto category = QString::fromLocal8Bit(context.category);
    if (category.startsWith(QLatin1String("kscreen"))) {
        Log::log(msg, category);
    }
    sDefaultMessageHandler(type, context, msg);
}

void log(const QString &msg)
{
    Log::log(msg);
}

Log *Log::instance()
{
    if (!sInstance) {
        sInstance = new Log();
    }

    return sInstance;
}

using namespace KScreen;
class Q_DECL_HIDDEN Log::Private
{
public:
    QString context;
    bool enabled = false;
    QString logFile;
};

Log::Log()
    : d(new Private)
{
    const char *logging_env = "KSCREEN_LOGGING";

    if (qEnvironmentVariableIsSet(logging_env)) {
        const QString logging_env_value = QString::fromUtf8(qgetenv(logging_env));
        if (logging_env_value != QLatin1Char('0') && logging_env_value.toLower() != QLatin1String("false")) {
            d->enabled = true;
        }
    }
    if (!d->enabled) {
        return;
    }
    d->logFile = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QLatin1String("/kscreen/kscreen.log");

    QLoggingCategory::setFilterRules(QStringLiteral("kscreen.*=true"));
    QFileInfo fi(d->logFile);
    if (!QDir().mkpath(fi.absolutePath())) {
        qWarning() << "Failed to create logging dir" << fi.absolutePath();
    }

    if (!sDefaultMessageHandler) {
        sDefaultMessageHandler = qInstallMessageHandler(kscreenLogOutput);
    }
}

Log::Log(Log::Private *dd)
    : d(dd)
{
}

Log::~Log()
{
    delete d;
    sInstance = nullptr;
}

QString Log::context() const
{
    return d->context;
}

void Log::setContext(const QString &context)
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
    if (!instance()->enabled()) {
        return;
    }
    auto _cat = category;
    _cat.remove(QStringLiteral("kscreen."));
    const QString timestamp = QDateTime::currentDateTime().toString(QStringLiteral("dd.MM.yyyy hh:mm:ss.zzz"));
    QString logMessage = QStringLiteral("\n%1 ; %2 ; %3 : %4").arg(timestamp, _cat, instance()->context(), msg);
    QFile file(instance()->logFile());
    if (!file.open(QIODevice::Append | QIODevice::Text)) {
        return;
    }
    file.write(logMessage.toUtf8());
}

} // ns
