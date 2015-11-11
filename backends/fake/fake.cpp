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

#include "fake.h"
#include "parser.h"

#include "config.h"
#include "edid.h"
#include <output.h>

#include <stdlib.h>

#include <QFile>
#include <QTimer>

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

#include <QDBusConnection>

#include "fakebackendadaptor.h"

using namespace KScreen;

Q_LOGGING_CATEGORY(KSCREEN_FAKE, "kscreen.fake")

Fake::Fake()
    : KScreen::AbstractBackend()
{
    QLoggingCategory::setFilterRules(QStringLiteral("kscreen.fake.debug = true"));


    if (qgetenv("KSCREEN_BACKEND_INPROCESS") != QByteArray("1")) {
        QTimer::singleShot(0, this, SLOT(delayedInit()));
    }
}

void Fake::init(const QVariantMap &arguments)
{
    if (!mConfig.isNull()) {
        mConfig.clear();
    }

    mConfigFile = arguments[QStringLiteral("TEST_DATA")].toString();
    qCDebug(KSCREEN_FAKE) << "Fake profile file:" << mConfigFile;

}

void Fake::delayedInit()
{
    new FakeBackendAdaptor(this);
    QDBusConnection::sessionBus().registerObject(QLatin1String("/fake"), this);
}

Fake::~Fake()
{
}

QString Fake::name() const
{
    return QString("Fake");
}

QString Fake::serviceName() const
{
    return QLatin1Literal("org.kde.KScreen.Backend.Fake");
}

ConfigPtr Fake::config() const
{
    if (mConfig.isNull()) {
        mConfig = Parser::fromJson(mConfigFile);
    }

    return mConfig;
}

void Fake::setConfig(const ConfigPtr &config)
{
    Q_UNUSED(config)
}

bool Fake::isValid() const
{
    return true;
}

QByteArray Fake::edid(int outputId) const
{
    Q_UNUSED(outputId);
    QFile file(mConfigFile);
    file.open(QIODevice::ReadOnly);

    const QJsonDocument jsonDoc = QJsonDocument::fromJson(file.readAll());
    const QJsonObject json = jsonDoc.object();

    const QJsonArray outputs = json["outputs"].toArray();
    Q_FOREACH(const QJsonValue &value, outputs) {
        const QVariantMap output = value.toObject().toVariantMap();
        if (output["id"].toInt() != outputId) {
            continue;
        }

        return QByteArray::fromBase64(output["edid"].toByteArray());
    }
    return QByteArray();
}

void Fake::setConnected(int outputId, bool connected)
{
    KScreen::OutputPtr output = config()->output(outputId);
    if (output->isConnected() == connected) {
        return;
    }

    output->setConnected(connected);
    Q_EMIT configChanged(mConfig);
}

void Fake::setEnabled(int outputId, bool enabled)
{
    KScreen::OutputPtr output = config()->output(outputId);
    if (output->isEnabled() == enabled) {
        return;
    }

    output->setEnabled(enabled);
    Q_EMIT configChanged(mConfig);
}

void Fake::setPrimary(int outputId, bool primary)
{
    KScreen::OutputPtr output = config()->output(outputId);
    if (output->isPrimary() == primary) {
        return;
    }

    Q_FOREACH (KScreen::OutputPtr output, config()->outputs()) {
        if (output->id() == outputId) {
            output->setPrimary(primary);
        } else {
            output->setPrimary(false);
        }
    }
    Q_EMIT configChanged(mConfig);
}

void Fake::setCurrentModeId(int outputId, const QString &modeId)
{
    KScreen::OutputPtr output = config()->output(outputId);
    if (output->currentModeId() == modeId) {
        return;
    }

    output->setCurrentModeId(modeId);
    Q_EMIT configChanged(mConfig);
}

void Fake::setRotation(int outputId, int rotation)
{
    KScreen::OutputPtr output = config()->output(outputId);
    const KScreen::Output::Rotation rot = static_cast<KScreen::Output::Rotation>(rotation);
    if (output->rotation() == rot) {
        return;
    }

    output->setRotation(rot);
    Q_EMIT configChanged(mConfig);
}

void Fake::addOutput(int outputId, const QString &name)
{
    KScreen::OutputPtr output(new KScreen::Output);
    output->setId(outputId);
    output->setName(name);
    mConfig->addOutput(output);
    Q_EMIT configChanged(mConfig);
}

void Fake::removeOutput(int outputId)
{
    mConfig->removeOutput(outputId);
    Q_EMIT configChanged(mConfig);
}
