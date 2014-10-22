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

#ifndef ABSTRACT_BACKEND_H
#define ABSTRACT_BACKEND_H

#include "kscreen_export.h"
#include "types.h"

#include <QString>
#include <QObject>

namespace KScreen {
    class Config;
    class Edid;

/** Abstract class for backends.
 *
 * The returned objects are expected to be memory-managed by the users. After creation,
 * the backend assumes no ownership of the backends.
 *
 * This means that we can not keep track of objects after we returned them, as the user
 * might have deleted the object.
 */
class KSCREEN_EXPORT AbstractBackend : public QObject
{
    Q_OBJECT

public:
    virtual ~AbstractBackend() {}

    virtual QString name() const = 0;

    virtual QString serviceName() const = 0;

    /** Returns a new Config object, holding Screen, Output objects, etc..
     *
     * The receiver of the Config* object is expected to manage its lifetime, and
     * the lifetime of its outputs.
     *
     * @return Config object for the system.
     */
    virtual KScreen::ConfigPtr config() const = 0;

    /** Apply a config object to the system.
     */
    virtual void setConfig(const KScreen::ConfigPtr &config) = 0;

    virtual bool isValid() const = 0;

    /** Returns encoded EDID data */
    virtual QByteArray edid(int outputId) const;

Q_SIGNALS:
    void configChanged(const KScreen::ConfigPtr &config);

};

} // namespace KScreen

#endif //ABSTRACT_BACKEND_H
