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

#include "fake.h"
#include "config.h"

Fake::Fake()
{
    Output *output = new Output(1);
    output->setModes(ModeList());
}

Fake::~Fake()
{

}

QString Fake::name() const
{
    return QString("Fake");
}

Config* Fake::config() const
{
    ModeList modes;
    modes.insert(1, new Mode());

    Output *output = new Output(1);
    output->setName("FakeOutput_1");
    output->setModes(modes);
    output->setPos(QPoint(0,0));
    output->setSize(QSize(1280,800));
    output->setRotation(Output::None);
    output->setConnected(true);
    output->setEnabled(true);
    output->setPrimary(true);

    OutputList outputs;
    outputs.insert(1, output);

    Config *config = new Config();
    config->setOutputs(outputs);

    return config;
}

bool Fake::isValid() const
{
    return true;
}