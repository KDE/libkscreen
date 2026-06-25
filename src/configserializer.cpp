/*
 * SPDX-FileCopyrightText: 2014 Daniel Vratil <dvratil@redhat.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 */

#include "configserializer_p.h"

#include "config.h"
#include "mode.h"
#include "output.h"
#include "screen.h"

#include <QDBusArgument>
#include <QFile>
#include <QJsonDocument>
#include <QRect>

using namespace Qt::StringLiterals;
using namespace KScreen;

QJsonObject ConfigSerializer::serializePoint(const QPoint &point)
{
    QJsonObject obj;
    obj[QLatin1String("x")] = point.x();
    obj[QLatin1String("y")] = point.y();
    return obj;
}

QJsonObject ConfigSerializer::serializeSize(const QSize &size)
{
    QJsonObject obj;
    obj[QLatin1String("width")] = size.width();
    obj[QLatin1String("height")] = size.height();
    return obj;
}

QJsonObject ConfigSerializer::serializeConfig(const ConfigPtr &config)
{
    QJsonObject obj;

    if (!config) {
        return obj;
    }

    QJsonArray outputs;
    for (const OutputPtr &output : config->outputs()) {
        outputs.append(serializeOutput(output));
    }
    obj[QLatin1String("outputs")] = outputs;
    if (config->screen()) {
        obj[QLatin1String("screen")] = serializeScreen(config->screen());
    }

    obj[QLatin1String("tabletModeAvailable")] = config->tabletModeAvailable();
    obj[QLatin1String("tabletModeEngaged")] = config->tabletModeEngaged();

    return obj;
}

QJsonObject ConfigSerializer::serializeOutput(const OutputPtr &output)
{
    QJsonObject obj;

    obj[QLatin1String("id")] = output->id();
    obj[QLatin1String("name")] = output->name();
    obj[QLatin1String("type")] = static_cast<int>(output->type());
    obj[QLatin1String("icon")] = output->icon();
    obj[QLatin1String("pos")] = serializePoint(output->pos());
    obj[QLatin1String("scale")] = output->scale();
    obj[QLatin1String("size")] = serializeSize(output->size());
    obj[QLatin1String("rotation")] = static_cast<int>(output->rotation());
    obj[QLatin1String("currentModeId")] = output->currentModeId();
    obj[QLatin1String("preferredModes")] = serializeList(output->preferredModes());
    obj[QLatin1String("connected")] = output->isConnected();
    obj[QLatin1String("followPreferredMode")] = output->followPreferredMode();
    obj[QLatin1String("enabled")] = output->isEnabled();
    obj[QLatin1String("priority")] = static_cast<int>(output->priority());
    obj[QLatin1String("clones")] = serializeList(output->clones());
    // obj[QLatin1String("edid")] = output->edid()->raw();
    obj[QLatin1String("sizeMM")] = serializeSize(output->sizeMm());
    obj[QLatin1String("replicationSource")] = output->replicationSource();

    QJsonArray modes;
    for (const ModePtr &mode : output->modes()) {
        modes.append(serializeMode(mode));
    }
    obj[QLatin1String("modes")] = modes;

    if (output->capabilities() & Output::Capability::Overscan) {
        obj[QLatin1String("overscan")] = static_cast<int>(output->overscan());
    }
    if (output->capabilities() & Output::Capability::Vrr) {
        obj[QLatin1String("vrrPolicy")] = static_cast<int>(output->vrrPolicy());
    }
    if (output->capabilities() & Output::Capability::RgbRange) {
        obj[QLatin1String("rgbRange")] = static_cast<int>(output->rgbRange());
    }
    if (output->capabilities() & Output::Capability::HighDynamicRange) {
        obj[QLatin1String("hdr")] = output->isHdrEnabled();
    }
    if (output->capabilities() & Output::Capability::HighDynamicRange) {
        obj[QLatin1String("sdr-brightness")] = static_cast<int>(output->sdrBrightness());
    }
    if (output->capabilities() & Output::Capability::WideColorGamut) {
        obj[QLatin1String("wcg")] = output->isWcgEnabled();
    }
    if (output->capabilities() & Output::Capability::AutoRotation) {
        obj[QLatin1String("autoRotatePolicy")] = static_cast<int>(output->autoRotatePolicy());
    }
    if (output->capabilities() & Output::Capability::IccProfile) {
        obj[QLatin1String("iccProfilePath")] = output->iccProfilePath();
    }
    if (output->capabilities() & Output::Capability::BrightnessControl) {
        obj[QLatin1String("brightness")] = output->brightness();
    }
    if (output->capabilities() & Output::Capability::DdcCi) {
        obj[QLatin1String("ddcCiAllowed")] = output->ddcCiAllowed();
    }
    if (output->capabilities() & Output::Capability::MaxBitsPerColor) {
        obj[QLatin1String("maxBpc")] = int(output->maxBitsPerColor());
    }
    if (output->capabilities() & Output::Capability::ExtendedDynamicRange) {
        obj[QLatin1String("edrPolicy")] = static_cast<int>(output->edrPolicy());
    }

    return obj;
}

QJsonObject ConfigSerializer::serializeMode(const ModePtr &mode)
{
    QJsonObject obj;

    obj[QLatin1String("id")] = mode->id();
    obj[QLatin1String("name")] = mode->name();
    obj[QLatin1String("size")] = serializeSize(mode->size());
    obj[QLatin1String("refreshRate")] = mode->refreshRate();

    return obj;
}

QJsonObject ConfigSerializer::serializeScreen(const ScreenPtr &screen)
{
    QJsonObject obj;

    obj[QLatin1String("id")] = screen->id();
    obj[QLatin1String("currentSize")] = serializeSize(screen->currentSize());
    obj[QLatin1String("maxSize")] = serializeSize(screen->maxSize());
    obj[QLatin1String("minSize")] = serializeSize(screen->minSize());
    obj[QLatin1String("maxActiveOutputsCount")] = screen->maxActiveOutputsCount();

    return obj;
}
