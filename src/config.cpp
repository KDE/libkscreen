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

#include "config.h"
#include "output.h"
#include "backendloader.h"
#include "backends/abstractbackend.h"

#include <QtCore/QDebug>
#include <QtCore/QRect>

namespace KScreen {

class Config::Private
{
  public:
    Private():
      valid(true),
      screen(0),
      primaryOutput(0)
    { }

    Private(const Private &other):
      valid(other.valid),
      primaryOutput(other.primaryOutput)
    {
      screen = other.screen->clone();
      Q_FOREACH (Output *otherOutput, other.outputs) {
          outputs.insert(otherOutput->id(), otherOutput->clone());
      }
    }

    bool valid;
    Screen* screen;
    Output* primaryOutput;
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

    if (!Config::canBeApplied(config)) {
        return false;
    }

    BackendLoader::backend()->setConfig(config);
    return true;
}

bool Config::canBeApplied(Config* config)
{
    Config* currentConfig = BackendLoader::backend()->config();
    QRect rect;
    QSize outputSize;
    Output* currentOutput = 0;
    OutputList outputs = config->outputs();
    int enabledOutputsCount = 0;
    Q_FOREACH(Output *output, outputs) {
        if (!output->isEnabled()) {
            continue;
        }

        ++enabledOutputsCount;

        currentOutput = currentConfig->output(output->id());
        //If there is no such output
        if (!currentOutput) {
            qDebug() << "The output:" << output->id() << "does not exists";
            return false;
        }
        //If the output is not connected
        if (!currentOutput->isConnected()) {
            qDebug() << "The output:" << output->id() << "is not connected";
            return false;
        }
        //if there is no currentMode
        if (output->currentModeId().isEmpty()) {
            qDebug() << "The output:" << output->id() << "has no currentModeId";
            return false;
        }
        //If the mode is not found in the current output
        if (!currentOutput->mode(output->currentModeId())) {
            qDebug() << "The output:" << output->id() << "has no mode:" << output->currentModeId();
            return false;
        }


        Mode *currentMode = output->currentMode();

        QSize outputSize = currentMode->size();

        if (output->pos().x() < rect.x()) {
            rect.setX(output->pos().x());
        }

        if (output->pos().y() < rect.y()) {
            rect.setY(output->pos().y());
        }

        QPoint bottomRight;
        if (output->isHorizontal()) {
            bottomRight = QPoint(output->pos().x() + outputSize.width(),
                                    output->pos().y() + outputSize.height());
        } else {
            bottomRight = QPoint(output->pos().x() + outputSize.height(),
                                    output->pos().y() + outputSize.width());
        }

        if (bottomRight.x() > rect.width()) {
            rect.setWidth(bottomRight.x());
        }

        if (bottomRight.y() > rect.height()) {
            rect.setHeight(bottomRight.y());
        }
    }

    const int maxEnabledOutputsCount = config->screen()->maxActiveOutputsCount();
    if (enabledOutputsCount > maxEnabledOutputsCount) {
        qDebug() << "Too many active screens. Requested: " << enabledOutputsCount << ", Max: " << maxEnabledOutputsCount;
        return false;
    }

    if (rect.width() > config->screen()->maxSize().width()) {
        qDebug() << "The configuration has too much width:" << rect.width();
        return false;
    }
    if (rect.height() > config->screen()->maxSize().height()) {
        qDebug() << "The configuration has too much height:" << rect.height();
        return false;
    }

    return true;
}

Config::Config(QObject* parent)
 : QObject(parent)
 , d(new Private())
{
}

Config::Config(Config::Private *dd)
  : QObject()
  , d(dd)
{
}


Config::~Config()
{
    delete d;
}

Config *Config::clone() const
{
    Config *config = new Config(new Private(*d));
    // Set parent of the newly copied items
    config->d->screen->setParent(config);
    Q_FOREACH (Output *output, config->d->outputs) {
        output->setParent(config);
    }

    return config;
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
    if (d->primaryOutput) {
        return d->primaryOutput;
    }

    Q_FOREACH(Output* output, d->outputs) {
        if (output->isPrimary()) {
            d->primaryOutput = output;
            return d->primaryOutput;
        }
    }

    return 0;
}

void Config::setPrimaryOutput(Output* output)
{
    d->primaryOutput = output;

    Q_EMIT primaryOutputChanged(output);
}

void Config::addOutput(Output* output)
{
    d->outputs.insert(output->id(), output);

    Q_EMIT outputAdded(output);
}

void Config::removeOutput(int outputId)
{
    Output *output = d->outputs.take(outputId);
    if (output) {
        output->deleteLater();
        if (d->primaryOutput == output) {
            setPrimaryOutput(0);
        }
    }

    Q_EMIT outputRemoved(outputId);
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
