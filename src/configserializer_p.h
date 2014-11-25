/*
 * Copyright (C) 2014  Daniel Vratil <dvratil@redhat.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#ifndef CONFIGSERIALIZER_H
#define CONFIGSERIALIZER_H

#include <QJsonObject>
#include <QJsonArray>
#include <QVariant>
#include <QDBusArgument>

#include "types.h"
#include "kscreen_export.h"

namespace KScreen
{

namespace ConfigSerializer
{

KSCREEN_EXPORT QJsonObject serializePoint(const QPoint &point);
KSCREEN_EXPORT QJsonObject serializeSize(const QSize &size);
template<typename T>
KSCREEN_EXPORT  QJsonArray serializeList(const QList<T> &list)
{
    QJsonArray arr;
    Q_FOREACH (const T &t, list) {
        arr.append(t);
    }
    return arr;
}

KSCREEN_EXPORT QJsonObject serializeConfig(const KScreen::ConfigPtr &config);
KSCREEN_EXPORT QJsonObject serializeOutput(const KScreen::OutputPtr &output);
KSCREEN_EXPORT QJsonObject serializeMode(const KScreen::ModePtr &mode);
KSCREEN_EXPORT QJsonObject serializeScreen(const KScreen::ScreenPtr &screen);

KSCREEN_EXPORT QPoint deserializePoint(const QDBusArgument &map);
KSCREEN_EXPORT QSize deserializeSize(const QDBusArgument &map);
template<typename T>
KSCREEN_EXPORT QList<T> deserializeList(const QDBusArgument &arg)
{
    QList<T> list;
    arg.beginArray();
    while (!arg.atEnd()) {
        QVariant v;
        arg >> v;
        list.append(v.value<T>());
    }
    arg.endArray();
    return list;
}
KSCREEN_EXPORT KScreen::ConfigPtr deserializeConfig(const QVariantMap &map);
KSCREEN_EXPORT KScreen::OutputPtr deserializeOutput(const QDBusArgument &output);
KSCREEN_EXPORT KScreen::ModePtr deserializeMode(const QDBusArgument &mode);
KSCREEN_EXPORT KScreen::ScreenPtr deserializeScreen(const QDBusArgument &screen);

}

}

#endif // CONFIGSERIALIZER_H
