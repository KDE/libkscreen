/*************************************************************************************
 *  Copyright 2012 - 2014  Daniel Vrátil <dvratil@redhat.com>                        *
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

#ifndef KSCREEN_CONFIGMONITOR_H
#define KSCREEN_CONFIGMONITOR_H

#include <QtCore/QObject>
#include <QtCore/QPointer>

#include "config.h"
#include "kscreen_export.h"

namespace KScreen
{

class AbstractBackend;
class BackendManager;

class KSCREEN_EXPORT ConfigMonitor : public QObject
{
    Q_OBJECT

public:
    static ConfigMonitor* instance();

    void addConfig(const KScreen::ConfigPtr &config);
    void removeConfig(const KScreen::ConfigPtr &config);

Q_SIGNALS:
    void configurationChanged();

private:
    explicit ConfigMonitor();
    virtual ~ConfigMonitor();

    Q_DISABLE_COPY(ConfigMonitor)

    friend BackendManager;
    void connectInProcessBackend(KScreen::AbstractBackend *backend);

    class Private;
    Private * const d;

};

} /* namespace KScreen */

#endif // KSCREEN_CONFIGMONITOR_H
