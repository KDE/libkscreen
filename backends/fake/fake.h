/*************************************************************************************
 *  Copyright (C) 2012 by Alejandro Fiestas Olivares <afiestas@kde.org>              *
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

#ifndef FAKE_BACKEND_H
#define FAKE_BACKEND_H

#include "../abstractbackend.h"
#include <QtCore/QObject>

class Fake : public QObject, public AbstractBackend
{
    Q_OBJECT
    Q_INTERFACES(AbstractBackend)

    public:
        explicit Fake(QObject* parent = 0);
        virtual ~Fake();

        virtual QString name() const;
        virtual KScreen::Config* config() const;
        virtual void setConfig(KScreen::Config* config) const;
        virtual bool isValid() const;
        virtual KScreen::Edid *edid(int outputId) const;
        virtual void updateConfig(KScreen::Config *config) const;
};

#endif //FAKE_BACKEND_H
