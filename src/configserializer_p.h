/*
 * SPDX-FileCopyrightText: 2014 Daniel Vratil <dvratil@redhat.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 */

#pragma once

#include <QDBusArgument>
#include <QJsonArray>
#include <QJsonObject>
#include <QVariant>

#include "kscreen_export.h"
#include "types.h"

namespace KScreen
{
namespace ConfigSerializer
{
KSCREEN_EXPORT QJsonObject serializePoint(const QPoint &point);
KSCREEN_EXPORT QJsonObject serializeSize(const QSize &size);
template<typename T>
KSCREEN_EXPORT QJsonArray serializeList(const QList<T> &list)
{
    QJsonArray arr;
    for (const T &t : list) {
        arr.append(t);
    }
    return arr;
}

KSCREEN_EXPORT QJsonObject serializeConfig(const KScreen::ConfigPtr &config);
KSCREEN_EXPORT QJsonObject serializeOutput(const KScreen::OutputPtr &output);
KSCREEN_EXPORT QJsonObject serializeMode(const KScreen::ModePtr &mode);
KSCREEN_EXPORT QJsonObject serializeScreen(const KScreen::ScreenPtr &screen);
}

}
