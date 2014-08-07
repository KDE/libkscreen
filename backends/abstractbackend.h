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

#include <QtCore/QString>
#include <QtCore/QObject>

namespace KScreen {
    class Config;
    class Edid;
}
/** Abstract class for backends.
 *
 * The returned objects are expected to be memory-managed by the users. After creation,
 * the backend assumes no ownership of the backends.
 *
 * This means that we can not keep track of objects after we returned them, as the user
 * might have deleted the object.
 */
class AbstractBackend
{
    public:
        virtual ~AbstractBackend() {}
        virtual QString name() const = 0;

        /** Returns a new Config object, holding Screen, Output objects, etc..
         *
         * The receiver of the Config* object is expected to manage its lifetime, and
         * the lifetime of its outputs.
         *
         * @return Config object for the system.
         */
        virtual KScreen::Config* config() const = 0;

        /** Apply a config object to the system.
         */
        virtual void setConfig(KScreen::Config* config) const = 0;

        virtual bool isValid() const = 0;

        /** Returns an Edid object for a given output.
         *
         * The receiver of the Edid* object is expected to manage its lifetime.
         *
         * @return Edid object for an output, or zero if no output exists.
         */
        virtual KScreen::Edid* edid(int outputId) const = 0;

        /** This method is called from the ConfigMonitor instance.
         *
         * This is how it works:
         * The backend notes a change, for example a screen has been added. It updates
         * its internal data, then calls ConfigMonitor::instance()->notifyUpdate. The
         * ConfigMonitor holds a pointer to the Config that is used and passes this into
         * the backend's updateConfig(Config*) function (i.e. this method).
         *
         * Your reimplementation of this method should update the configuration's outputs,
         * screen, etc..
         */
        virtual void updateConfig(KScreen::Config* config) const = 0;
};

Q_DECLARE_INTERFACE(AbstractBackend, "org.kde.libkscreen")
#endif //ABSTRACT_BACKEND_H
