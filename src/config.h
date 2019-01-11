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

#include <QHash>
#include <QObject>
#include <QMetaType>


namespace KScreen {

/**
 * Represents a (or the) screen configuration.
 *
 * This is the main class of KScreen, with it you can use
 * the static methods current() to get the systems config and
 * setConfig() to apply a config to the system.
 *
 * Also, you can instantiate an empty Config, this is usually done
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

    /** This indicates which features the used backend supports.
     *
     * @see supportedFeatures
     * @since 5.7
     */
    enum class Feature {
        None = 0, ///< None of the mentioned features are supported.
        PrimaryDisplay = 1, ///< The backend knows about the concept of a primary display, this is mostly limited to X11.
        Writable = 1 << 1, ///< The backend supports setting the config, it's not read-only.
        PerOutputScaling = 1 << 2 ///< The backend supports scaling each output individually.
    };
    Q_DECLARE_FLAGS(Features, Feature)

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
     * @since 5.3.0
     */
    static bool canBeApplied(const ConfigPtr &config, ValidityFlags flags);

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
    static bool canBeApplied(const ConfigPtr &config);

    /**
     * Instantiate an empty config
     *
     * Usually you do not want to use this constructor since there are some
     * values that make no sense to set (for example you want the Screen of
     * the current systme).
     *
     * So usually what you do is call current() and then modify
     * whatever you need.
     */
    explicit Config();
    ~Config() override;

    /**
     * Duplicates the config
     *
     * @return a new Config instance with same property values
     */
    ConfigPtr clone() const;

    /**
     * Returns an identifying hash for this config in regards to its
     * connected outputs.
     *
     * The hash is calculated with a sorted combination of all
     * connected output hashes.
     *
     * @return sorted hash combination of all connected outputs
     * @since 5.15
     */
    QString connectedOutputsHash() const;

    ScreenPtr screen() const;
    void setScreen(const ScreenPtr &screen);

    OutputPtr output(int outputId) const;
    OutputList outputs() const;
    OutputList connectedOutputs() const;
    OutputPtr primaryOutput() const;
    void setPrimaryOutput(const OutputPtr &output);
    void addOutput(const OutputPtr &output);
    void removeOutput(int outputId);
    void setOutputs(const OutputList &outputs);

    bool isValid() const;
    void setValid(bool valid);

    void apply(const ConfigPtr &other);

    /** Indicates features supported by the backend. This exists to allow the user
     * to find out which of the features offered by libkscreen are actually supported
     * by the backend. Not all backends are writable (QScreen, for example is
     * read-only, only XRandR, but not KWayland support the primary display, etc.).
     *
     * @return Flags for features that are supported for this config, determined by
     * the backend.
     * @see setSupportedFeatures
     * @since 5.7
     */
    Features supportedFeatures() const;

    /** Sets the features supported by this backend. This should not be called by the
     * user, but by the backend.
     *
     * @see supportedFeatures
     * @since 5.7
     */
    void setSupportedFeatures(const Features &features);

  Q_SIGNALS:
      void outputAdded(const KScreen::OutputPtr &output);
      void outputRemoved(int outputId);
      void primaryOutputChanged(const KScreen::OutputPtr &output);

  private:
    Q_DISABLE_COPY(Config)

    class Private;
    Private * const d;
};

} //KScreen namespace

Q_DECLARE_OPERATORS_FOR_FLAGS(KScreen::Config::Features)

KSCREEN_EXPORT QDebug operator<<(QDebug dbg, const KScreen::ConfigPtr &config);



#endif //KSCREEN_CONFIG_H
