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

#include "config.h"
#include "output.h"

Config::Config(QObject* parent)
 : QObject(parent)
 , m_valid(false)
{

}

Config::~Config()
{

}

Output* Config::output(int outputId)
{
    if (!m_outputs.contains(outputId)) {
        return 0;
    }

    return m_outputs[outputId];
}

QHash< int, Output* > Config::outputs()
{
    return m_outputs;
}

void Config::setOutputs(OutputList outputs)
{
    m_outputs = outputs;
}

bool Config::isValid()
{
    return isValid();
}

void Config::setValid(bool valid)
{
    m_valid = valid;
}


#include "config.moc"