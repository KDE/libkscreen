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

#ifndef SCREEN_CONFIG_H
#define SCREEN_CONFIG_H

#include "kscreen_export.h"
#include "output.h"

#include <QtCore/QHash>
#include <QtCore/QObject>
#include <QtCore/QMetaType>

class Output;
class KSCREEN_EXPORT Config : public QObject
{
    Q_OBJECT
    Q_PROPERTY(OutputList outputs READ outputs)

    public:
        explicit Config(QObject *parent = 0);
        virtual ~Config();

        QHash<int, Output*> outputs();
        void setOutputs(OutputList outputs);

    private:
        OutputList m_outputs;
};

#endif //SCREEN_CONFIG_H