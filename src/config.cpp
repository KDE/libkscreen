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
#include "backendloader.h"
#include "backends/abstractbackend.h"

namespace KScreen {

class Config::Private
{
  public:
    Private():
      valid(true),
      screen(0)
    { }

    bool valid;
    Screen* screen;
    OutputList outputs;
};

bool Config::loadBackend()
{
    return BackendLoader::init();
}

Config* Config::current()
{
    if (!BackendLoader::init()) {
        return 0;
    }

    return BackendLoader::backend()->config();
}

bool Config::setConfig(Config* config)
{
    if (!BackendLoader::init()) {
        return false;
    }

    BackendLoader::backend()->setConfig(config);
    return true;
}

Config::Config(QObject* parent)
 : QObject(parent)
 , d(new Private())
{

}

Config::~Config()
{
    delete d;
}

Screen* Config::screen() const
{
    return d->screen;
}

void Config::setScreen(Screen* screen)
{
    d->screen = screen;
}

Output* Config::output(int outputId) const
{
    if (!d->outputs.contains(outputId)) {
        return 0;
    }

    return d->outputs[outputId];
}

QHash< int, Output* > Config::outputs() const
{
    return d->outputs;
}

QHash< int, Output* > Config::connectedOutputs() const
{
    QHash< int, Output* > outputs;
    Q_FOREACH(Output* output, d->outputs) {
        if (!output->isConnected()) {
            continue;
        }
        outputs.insert(output->id(), output);
    }

    return outputs;
}

Output* Config::primaryOutput() const
{
    Q_FOREACH(Output* output, d->outputs) {
        if (output->isPrimary()) {
            return output;
        }
    }
    return 0;
}

void Config::setOutputs(OutputList outputs)
{
    d->outputs = outputs;
}

bool Config::isValid() const
{
    return d->valid;
}

void Config::setValid(bool valid)
{
    d->valid = valid;
}

} //KScreen namespace

#include "config.moc"
