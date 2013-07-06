/*
 * Copyright 2012  Dan Vratil <dvratil@redhat.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef KSCREEN_CONFIGMONITOR_H
#define KSCREEN_CONFIGMONITOR_H

#include <QtCore/QObject>
#include <QtCore/QPointer>

#include "config.h"
#include "kscreen_export.h"


namespace KScreen
{

class KSCREEN_EXPORT ConfigMonitor : public QObject
{
    Q_OBJECT

  public:
    static ConfigMonitor* instance();

    void addConfig(KScreen::Config *config);
    void removeConfig(KScreen::Config *config);

    void notifyUpdate();

  Q_SIGNALS:
    void configurationChanged();

  private:
    explicit ConfigMonitor();
    virtual ~ConfigMonitor();

    Q_DISABLE_COPY(ConfigMonitor)

    class Private;
    Private * const d;
};

} /* namespace KScreen */

#endif // KSCREEN_CONFIGMONITOR_H
