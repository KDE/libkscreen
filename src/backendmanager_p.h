/*
 * SPDX-FileCopyrightText: 2014 Daniel Vratil <dvratil@redhat.com>
 * SPDX-FileCopyrightText: 2015 Sebastian Kügler <sebas@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 */

/**
 * WARNING: This header is *not* part of public API and is subject to change.
 * There are not guarantees or API or ABI stability or compatibility between
 * releases
 */

#pragma once

#include <QEventLoop>
#include <QFileInfoList>
#include <QObject>
#include <QPluginLoader>
#include <QProcess>
#include <QTimer>

#include "kscreen_export.h"
#include "types.h"

namespace KScreen
{
class AbstractBackend;

class KSCREEN_EXPORT BackendManager : public QObject
{
    Q_OBJECT

public:
    static BackendManager *instance();
    ~BackendManager() override;

    KScreen::ConfigPtr config() const;
    void setConfig(KScreen::ConfigPtr c);

    /** Set arguments map which a backend may use on initialization.
     *
     * Calling this method after a backend has been initialized will have no effect.
     * Arguments map will NOT be automatically cleared on backend shutdown; which
     * makes possible setting arguments before restarting backend in tests.
     *
     * @param map of arbitrary arguments for backends; each backend is free to interpret
     * them as it sees fit.
     * @since 5.27
     */
    void setBackendArgs(const QVariantMap &arguments);

    /** Get arguments map which a backend may use on initialization.
     *
     * @return map of arbitrary arguments for backends.
     * @since 5.27
     */
    QVariantMap getBackendArgs();

    KScreen::AbstractBackend *loadBackendInProcess();

    void shutdownBackend();

private:
    friend class SetInProcessOperation;
    friend class InProcessConfigOperationPrivate;
    friend class SetConfigOperation;
    friend class SetConfigOperationPrivate;

    explicit BackendManager();
    static BackendManager *sInstance;

    KScreen::ConfigPtr mConfig;
    QVariantMap mBackendArguments;

    KScreen::AbstractBackend *mInProcessBackend;
};

}
