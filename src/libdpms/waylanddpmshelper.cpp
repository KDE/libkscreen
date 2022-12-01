// SPDX-FileCopyrightText: 2015 by Martin Gräßlin <mgraesslin@kde.org>
// SPDX-FileCopyrightText: 2022 Aleix Pol Gonzalez <aleixpol@kde.org>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "kscreendpms_debug.h"
#include "waylanddpmshelper_p.h"

#include "qwayland-dpms.h"
#include <QDebug>
#include <QGuiApplication>
#include <QPointer>
#include <QScreen>
#include <QVector>
#include <QWaylandClientExtensionTemplate>
#include <qpa/qplatformnativeinterface.h>

class Dpms : public QObject, public QtWayland::org_kde_kwin_dpms
{
public:
    Dpms(struct ::org_kde_kwin_dpms *object, WaylandDpmsHelper *dpms, QScreen *parent)
        : QObject(parent)
        , org_kde_kwin_dpms(object)
        , m_screen(parent)
        , m_dpms(dpms)
    {
    }

    bool isSupported() const
    {
        return m_supported;
    }

    void org_kde_kwin_dpms_supported(uint32_t supported) override
    {
        m_pendingSupported = supported;
    }

    void org_kde_kwin_dpms_mode(uint32_t newMode) override
    {
        m_mode = mode(newMode);
    }

    void org_kde_kwin_dpms_done() override
    {
        m_supported = m_pendingSupported;
        KScreen::Dpms::Mode mode;
        switch (m_mode) {
        case Dpms::mode_On:
            mode = KScreen::Dpms::On;
            break;
        case Dpms::mode_Standby:
            mode = KScreen::Dpms::Standby;
            break;
        case Dpms::mode_Suspend:
            mode = KScreen::Dpms::Suspend;
            break;
        case Dpms::mode_Off:
            mode = KScreen::Dpms::Off;
            break;
        }
        if (m_dpms) {
            Q_EMIT m_dpms->modeChanged(mode, m_screen);
        }
    }

    QScreen *const m_screen;
    QPointer<WaylandDpmsHelper> m_dpms;
    bool m_supported = false;
    bool m_pendingSupported = false;
    mode m_mode;
};

class DpmsManager : public QWaylandClientExtensionTemplate<::DpmsManager>, public QtWayland::org_kde_kwin_dpms_manager
{
public:
    DpmsManager(WaylandDpmsHelper *dpms)
        : QWaylandClientExtensionTemplate<DpmsManager>(1)
        , m_dpms(dpms)
    {
        connect(this, &DpmsManager::activeChanged, this, [this] {
            const bool hasDpms = isActive();
            if (hasDpms) {
                qCDebug(KSCREEN_DPMS) << "Compositor provides a DpmsManager";
            } else {
                qCDebug(KSCREEN_DPMS) << "Compositor does not provide a DpmsManager";
                m_dpms->setSupported(hasDpms);
                return;
            }

            const auto screens = qGuiApp->screens();
            for (QScreen *screen : screens) {
                QPlatformNativeInterface *native = qGuiApp->platformNativeInterface();
                wl_output *output = reinterpret_cast<wl_output *>(native->nativeResourceForScreen(QByteArrayLiteral("output"), screen));
                m_dpmsPerScreen[screen] = new Dpms(get(output), m_dpms, screen);
            }
            m_dpms->setSupported(hasDpms);
        });
    }

    Dpms *fetch(QScreen *screen)
    {
        return m_dpmsPerScreen.value(screen);
    }

private:
    WaylandDpmsHelper *const m_dpms;
    QHash<QScreen *, Dpms *> m_dpmsPerScreen;
};

WaylandDpmsHelper::WaylandDpmsHelper()
    : AbstractDpmsHelper()
    , m_dpmsManager(new DpmsManager(this))
{
}

WaylandDpmsHelper::~WaylandDpmsHelper()
{
    delete m_dpmsManager;
}

void WaylandDpmsHelper::trigger(KScreen::Dpms::Mode mode, const QList<QScreen *> &screens)
{
    Q_ASSERT(isSupported());

    if (screens.isEmpty()) {
        return;
    }

    setHasPendingChanges(true);

    auto level = Dpms::mode_On;
    switch (mode) {
    case KScreen::Dpms::Toggle: {
        for (QScreen *screen : screens) {
            auto dpms = m_dpmsManager->fetch(screen);
            if (!dpms->isSupported()) {
                qCDebug(KSCREEN_DPMS) << "screen does not provide dpms" << screen;
                continue;
            }
            if (dpms->m_mode == Dpms::mode_On) {
                dpms->set(Dpms::mode_Off);
            } else {
                dpms->set(Dpms::mode_On);
            }
        }
    }
        return;
    case KScreen::Dpms::Off:
        level = Dpms::mode_Off;
        break;
    case KScreen::Dpms::Standby:
        level = Dpms::mode_Standby;
        break;
    case KScreen::Dpms::Suspend:
        level = Dpms::mode_Suspend;
        break;
    case KScreen::Dpms::On:
        level = Dpms::mode_On;
        break;
    }
    for (auto screen : screens) {
        auto dpms = m_dpmsManager->fetch(screen);
        dpms->set(level);
    }
    setHasPendingChanges(false);
}

void WaylandDpmsHelper::blockUntilSupported() const
{
    QMetaObject::invokeMethod(m_dpmsManager, "addRegistryListener");
}
