/*************************************************************************************
 *  Copyright (C) 2012 by Alejandro Fiestas Olivares <afiestas@kde.org>              *
 *  Copyright (C) 2012, 2013 by Daniel VrÃ¡til <dvratil@redhat.com>                   *
 *  Copyright 2014 Sebastian Kügler <sebas@kde.org>                                  *
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

#ifndef QSCREEN_BACKEND_H
#define QSCREEN_BACKEND_H

#include "../abstractbackend.h"

#include <QScreen>
#include <QtCore/QSize>
#include <QLoggingCategory>

namespace KScreen
{
class Output;
class QScreenConfig;
}

class QScreenBackend : public QObject, public AbstractBackend
{
    Q_OBJECT
    Q_INTERFACES(AbstractBackend)
    Q_PLUGIN_METADATA(IID "org.kf5.kscreen.backends.qscreen")

public:
    explicit QScreenBackend(QObject *parent = 0);
    virtual ~QScreenBackend();

    virtual QString name() const;
    virtual KScreen::Config *config() const;
    virtual void setConfig(KScreen::Config *config) const;
    virtual bool isValid() const;
    virtual KScreen::Edid *edid(int outputId) const;
    virtual void updateConfig(KScreen::Config *config) const;

private:
    bool m_isValid;
    static KScreen::QScreenConfig *s_internalConfig;
};

Q_DECLARE_LOGGING_CATEGORY(KSCREEN_QSCREEN)

#endif //QSCREEN_BACKEND_H
