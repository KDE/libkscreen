/*************************************************************************************
 *  Copyright (C) 2012 by Alejandro Fiestas Olivares <afiestas@kde.org>              *
 *  Copyright (C) 2014 by Daniel Vrátil <dvratil@redhat.com>                         *
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
#include "types.h"
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
    Q_PROPERTY(ScreenPtr screen READ screen)
    Q_PROPERTY(OutputList outputs READ outputs)

  public:
    enum class ValidityFlag {
        None = 0x0,
        RequireAtLeastOneEnabledScreen = 0x1
    };
    Q_DECLARE_FLAGS(ValidityFlags, ValidityFlag)

    /**
     * Validates that a config can be applied in the current system
     *
     * Each system has different constrains, this method will test
     * the given config with those constrains to see if it
     * can be applied.
     *
     * @arg config to be checked
     * @flags enable additional optional checks
     * @return true if the configuration can be applied, false if not.
     */
    static bool canBeApplied(const ConfigPtr &config, ValidityFlags flags = ValidityFlag::None);

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
    explicit Config();
    virtual ~Config();

    /**
     * Duplicates the config
     *
     * @return a new Config instance with same property values
     */
    ConfigPtr clone() const;

    ScreenPtr screen() const;
    void setScreen(const ScreenPtr &screen);

    OutputPtr output(int outputId) const;
    OutputList outputs() const;
    OutputList connectedOutputs() const;
    OutputPtr primaryOutput() const;
    void setPrimaryOutput(const OutputPtr &output);
    void addOutput(const OutputPtr &output);
    void removeOutput(int outputId);
    void setOutputs(OutputList outputs);

    bool isValid() const;
    void setValid(bool valid);

    void apply(const ConfigPtr &other);

  Q_SIGNALS:
      void outputAdded(const KScreen::OutputPtr &output);
      void outputRemoved(int outputId);
      void primaryOutputChanged(const KScreen::OutputPtr &output);

  private:
    Q_DISABLE_COPY(Config)

    class Private;
    Private * const d;

    Config(Private *dd);
};

} //KScreen namespace


#endif //KSCREEN_CONFIG_H
