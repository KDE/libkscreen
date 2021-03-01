/*
 *  SPDX-FileCopyrightText: 2012 Alejandro Fiestas Olivares <afiestas@kde.org>
 *  SPDX-FileCopyrightText: 2014 Daniel Vrátil <dvratil@redhat.com>
 *
 *  SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef KSCREEN_CONFIG_H
#define KSCREEN_CONFIG_H

#include "kscreen_export.h"
#include "screen.h"
#include "types.h"

#include <QHash>
#include <QMetaType>
#include <QObject>

namespace KScreen
{
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
        RequireAtLeastOneEnabledScreen = 0x1,
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
        PerOutputScaling = 1 << 2, ///< The backend supports scaling each output individually.
        OutputReplication = 1 << 3, ///< The backend supports replication of outputs.
        AutoRotation = 1 << 4, ///< The backend supports automatic rotation of outputs.
        TabletMode = 1 << 5, ///< The backend supports querying if a device is in tablet mode.
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

    /**
     * Indicates that the device supports switching between a default and a tablet mode. This is
     * common for convertibles.
     *
     * @return true when tablet mode is available, otherwise false
     * @see setTabletModeAvailable
     * @since 5.18
     */
    bool tabletModeAvailable() const;

    /** Sets if the device supports a tablet mode. This should not be called by the
     * user, but by the backend.
     *
     * @see tabletModeAvailable
     * @since 5.18
     */
    void setTabletModeAvailable(bool available);

    /**
     * Indicates that the device is currently in tablet mode.
     *
     * @return true when in tablet mode, otherwise false
     * @see setTabletModeEngaged
     * @since 5.18
     */
    bool tabletModeEngaged() const;

    /**
     * Sets if the device is currently in tablet mode. This should not be called by the
     * user, but by the backend.
     *
     * @see tabletModeEngaged
     * @since 5.18
     */
    void setTabletModeEngaged(bool engaged);

Q_SIGNALS:
    void outputAdded(const KScreen::OutputPtr &output);
    void outputRemoved(int outputId);
    void primaryOutputChanged(const KScreen::OutputPtr &output);

private:
    Q_DISABLE_COPY(Config)

    class Private;
    Private *const d;
};

} // KScreen namespace

Q_DECLARE_OPERATORS_FOR_FLAGS(KScreen::Config::Features)

KSCREEN_EXPORT QDebug operator<<(QDebug dbg, const KScreen::ConfigPtr &config);

#endif // KSCREEN_CONFIG_H
