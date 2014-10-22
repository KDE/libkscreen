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

#ifndef BACKENDDBUSWRAPPER_H
#define BACKENDDBUSWRAPPER_H

#include <QObject>
#include <QTimer>

#include "src/types.h"

namespace KScreen
{
class AbstractBackend;
}

class BackendDBusWrapper : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.kde.KScreen.Backend")

public:
    explicit BackendDBusWrapper(KScreen::AbstractBackend *backend);
    virtual ~BackendDBusWrapper();

    bool init();

    QVariantMap getConfig() const;
    QVariantMap setConfig(const QVariantMap &config);
    QByteArray getEdid(int output) const;

Q_SIGNALS:
    void configChanged(const QVariantMap &config);

private Q_SLOTS:
    void backendConfigChanged(const KScreen::ConfigPtr &config);
    void doEmitConfigChanged();


private:
    KScreen::AbstractBackend *mBackend;
    QTimer mChangeCollector;
    KScreen::ConfigPtr mCurrentConfig;

};

#endif // BACKENDDBUSWRAPPER_H
