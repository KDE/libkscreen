/*************************************************************************************
 *  Copyright (C) 2013 by Alejandro Fiestas Olivares <afiestas@kde.org>              *
 *                                                                                   *
 *  This program is free software; you can redistribute it and/or                    *
 *  modify it under the terms of the GNU General Public License                      *
 *  as published by the Free Software Foundation; either version 2                   *
 *  of the License, or (at your option) any later version.                           *
 *                                                                                   *
 *  This program is distributed in the hope that it will be useful,                  *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of                   *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                    *
 *  GNU General Public License for more details.                                     *
 *                                                                                   *
 *  You should have received a copy of the GNU General Public License                *
 *  along with this program; if not, write to the Free Software                      *
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA   *
 *************************************************************************************/

#ifndef XRANDR11_BACKEND_H
#define XRANDR11_BACKEND_H

#include "xlibandxcb.h"
#include "../abstractbackend.h"
#include <QtCore/QObject>

class XRandRX11Helper;

class XRandR11 : public QObject, public AbstractBackend
{
    Q_OBJECT
    Q_INTERFACES(AbstractBackend)

public:
    explicit XRandR11(QObject* parent = 0);
    virtual ~XRandR11();

    virtual QString name() const;
    virtual KScreen::Config* config() const;
    virtual void setConfig(KScreen::Config* config) const;
    virtual bool isValid() const;
    virtual KScreen::Edid *edid(int outputId) const;
    virtual void updateConfig(KScreen::Config *config) const;

private Q_SLOTS:
    void updateConfig();

private:
    bool m_valid;
    XRandRX11Helper* m_x11Helper;
    KScreen::Config* m_currentConfig;
    xcb_timestamp_t m_currentTimestamp;
};

extern int dXndr();
#endif //FAKE_BACKEND_H
