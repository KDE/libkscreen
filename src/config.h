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

#ifndef KSCREEN_CONFIG_H
#define KSCREEN_CONFIG_H

#include "screen.h"
#include "output.h"
#include "kscreen_export.h"

#include <QtCore/QHash>
#include <QtCore/QObject>
#include <QtCore/QMetaType>

namespace KScreen {

/**
 * Represents a (or the) screen configuration.
 *
 * This is the main class of KScreen, with it you can use
 * the static methods current() to get the systems config and
 * setConfig() to apply a config to the system.
 *
 * Also, you can instance an empty Config, this is usualy done
 * to create a config (with the objective of setting it) from scratch
 * and for example unserialize a saved config to it.
 *
 */
class KSCREEN_EXPORT Config : public QObject
{
    Q_OBJECT
    Q_PROPERTY(Screen* screen READ screen)
    Q_PROPERTY(OutputList outputs READ outputs)

  public:
    /**
    * Tries to load a backend (it might be already loaded)
    *
    * @return true if there is a working backend, false if none are found or work
    */
    static bool loadBackend();

    /**
     * Gets the current system configuration
     *
     * The returned config is a representation of the current system setup, for
     * example if your screens a currently cloned, it will show that.
     *
     * @return the current system config, or null on error
     */
    static Config* current();

    /**
     * Sets the given config to the system
     *
     * The config will first be validated via canBeApplied(), then
     * it will be applied to the system.
     *
     * @arg config to be applied
     * @return true if everything went well, false if something failed
     */
    static bool setConfig(Config* config);

    /**
     * Validates that a config can be applied in the current system
     *
     * Each system has different constrains, this method will test
     * the given config with those constrains to see if it
     * can be applied.
     *
     * @arg config to be checked
     * @return true if the configuration can be applied, false if not.
     */
    static bool canBeApplied(Config* config);

    /**
     * Instance an empty config
     *
     * Usually you never want to use this constructor since there are some
     * values that make no sense to set (for example you want the Screen of
     * the current systme).
     *
     * So usually what you do is call current() and then modify
     * whatever you need.
     */
    explicit Config(QObject *parent = 0);
    virtual ~Config();

    /**
     * Duplicates the config
     *
     * @return a new Config instance with same property values
     */
    Config* clone() const;

    Screen* screen() const;
    void setScreen(Screen* screen);

    Output* output(int outputId) const;
    QHash<int, Output*> outputs() const;
    QHash<int, Output*> connectedOutputs() const;
    Output* primaryOutput() const;
    void setPrimaryOutput(Output *output);
    void addOutput(Output *output);
    void removeOutput(int outputId);
    void setOutputs(OutputList outputs);

    bool isValid() const;
    void setValid(bool valid);

  Q_SIGNALS:
      void outputAdded(KScreen::Output *output);
      void outputRemoved(int outputId);
      void primaryOutputChanged(KScreen::Output *output);
  private:
    Q_DISABLE_COPY(Config)

    class Private;
    Private * const d;

    Config(Private *dd);
};

} //KScreen namespace

#endif //KSCREEN_CONFIG_H
