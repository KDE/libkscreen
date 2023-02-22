// SPDX-FileCopyrightText: 2015 by Martin Gräßlin <mgraesslin@kde.org>
// SPDX-FileCopyrightText: 2022 Aleix Pol Gonzalez <aleixpol@kde.org>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef ABSTRACTDPMSHELPER_H
#define ABSTRACTDPMSHELPER_H

#include "dpms.h"
#include <QObject>
#include <optional>

class QScreen;

namespace KScreen
{

class AbstractDpmsHelper : public QObject
{
    Q_OBJECT
public:
    virtual ~AbstractDpmsHelper();

    virtual void trigger(Dpms::Mode, const QList<QScreen *> &screens) = 0;

    bool isSupported()
    {
        if (!m_supported.has_value()) {
            blockUntilSupported();
        }
        Q_ASSERT(m_supported.has_value());
        return *m_supported;
    }
    void setSupported(bool supported)
    {
        if (m_supported != supported) {
            m_supported = supported;
            Q_EMIT supportedChanged(supported);
        }
    }
    void setHasPendingChanges(bool hasThem)
    {
        if (m_hasPendingChanges != hasThem) {
            return;
        }
        m_hasPendingChanges = hasThem;
        Q_EMIT hasPendingChangesChanged(hasThem);
    }

    bool hasPendingChanges() const
    {
        return m_hasPendingChanges;
    }

Q_SIGNALS:
    void supportedChanged(bool supported);
    void modeChanged(Dpms::Mode mode, QScreen *screen);
    void hasPendingChangesChanged(bool pendingChanges);

private:
    virtual void blockUntilSupported() {}

    std::optional<bool> m_supported;
    bool m_hasPendingChanges = false;
};

}

#endif
