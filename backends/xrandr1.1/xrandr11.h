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

#ifndef XRANDR11_BACKEND_H
#define XRANDR11_BACKEND_H

#include "xlibandxcb.h"
#include "../abstractbackend.h"
#include <QtCore/QObject>
#include <QLoggingCategory>

class XRandRXCBHelper;

class XRandR11 : public KScreen::AbstractBackend
{
    Q_OBJECT
    Q_INTERFACES(KScreen::AbstractBackend)
    Q_PLUGIN_METADATA(IID "org.kf5.kscreen.backends.xrandr11")

public:
    explicit XRandR11();
    virtual ~XRandR11();

    virtual QString name() const;
    virtual QString serviceName() const;
    virtual KScreen::ConfigPtr config() const;
    virtual void setConfig(const KScreen::ConfigPtr &config);
    virtual bool isValid() const;

private Q_SLOTS:
    void updateConfig();

private:
    bool m_valid;
    XRandRXCBHelper* m_x11Helper;
    KScreen::ConfigPtr m_currentConfig;
    xcb_timestamp_t m_currentTimestamp;
};

Q_DECLARE_LOGGING_CATEGORY(KSCREEN_XRANDR11)
#endif //FAKE_BACKEND_H
