/*************************************************************************************
 *  Copyright (C) 2012 by Alejandro Fiestas Olivares <afiestas@kde.org>              *
 *  Copyright (C) 2014 by Daniel Vrátil <dvratil@redhat.com>                         *
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

using namespace KScreen;

class Config::Private
{
  public:
    Private():
      valid(true)
    { }

    Private(const Private &other):
      valid(other.valid),
      primaryOutput(other.primaryOutput)
    {
      screen = other.screen->clone();
      Q_FOREACH (const OutputPtr &otherOutput, other.outputs) {
          outputs.insert(otherOutput->id(), otherOutput->clone());
      }
    }

    bool valid;
    ScreenPtr screen;
    OutputPtr primaryOutput;
    OutputList outputs;
};

bool Config::loadBackend()
{
    return BackendLoader::init();
}

ConfigPtr Config::current()
{
    if (!BackendLoader::init()) {
        return ConfigPtr();
    }

    return BackendLoader::backend()->config();
}

bool Config::setConfig(const ConfigPtr &config)
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

bool Config::canBeApplied(const ConfigPtr &config)
{
    ConfigPtr currentConfig = BackendLoader::backend()->config();
    QRect rect;
    QSize outputSize;
    OutputPtr currentOutput;
    const OutputList outputs = config->outputs();
    int enabledOutputsCount = 0;
    Q_FOREACH(const OutputPtr &output, outputs) {
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


        const ModePtr currentMode = output->currentMode();

        const QSize outputSize = currentMode->size();

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

Config::Config()
 : QObject(0)
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

ConfigPtr Config::clone() const
{
    return ConfigPtr(new Config(new Private(*d)));
}


ScreenPtr Config::screen() const
{
    return d->screen;
}

void Config::setScreen(const ScreenPtr &screen)
{
    d->screen = screen;
}

OutputPtr Config::output(int outputId) const
{
    if (!d->outputs.contains(outputId)) {
        return OutputPtr();
    }

    return d->outputs[outputId];
}

OutputList Config::outputs() const
{
    return d->outputs;
}

OutputList Config::connectedOutputs() const
{
    OutputList outputs;
    Q_FOREACH(const OutputPtr &output, d->outputs) {
        if (!output->isConnected()) {
            continue;
        }
        outputs.insert(output->id(), output);
    }

    return outputs;
}

OutputPtr Config::primaryOutput() const
{
    if (d->primaryOutput) {
        return d->primaryOutput;
    }

    Q_FOREACH(const OutputPtr &output, d->outputs) {
        if (output->isPrimary()) {
            d->primaryOutput = output;
            return d->primaryOutput;
        }
    }

    return OutputPtr();
}

void Config::setPrimaryOutput(const OutputPtr &output)
{
    d->primaryOutput = output;

    Q_EMIT primaryOutputChanged(output);
}

void Config::addOutput(const OutputPtr &output)
{
    d->outputs.insert(output->id(), output);

    Q_EMIT outputAdded(output);
}

void Config::removeOutput(int outputId)
{
    OutputPtr output = d->outputs.take(outputId);
    if (output) {
        if (d->primaryOutput == output) {
            setPrimaryOutput(OutputPtr());
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
