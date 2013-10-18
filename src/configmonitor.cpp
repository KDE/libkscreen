/*************************************************************************************
 *  Copyright 2012, 2013  Daniel Vrátil <dvratil@redhat.com>                         *
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

#include "configmonitor.h"
#include "backendloader.h"
#include "backends/abstractbackend.h"

using namespace KScreen;


class ConfigMonitor::Private
{
  public:
    Private():
      backend(BackendLoader::backend())
    { }

    void updateConfigs();

    QList< QPointer<KScreen::Config> >  watchedConfigs;
    AbstractBackend* backend;
};

void ConfigMonitor::Private::updateConfigs()
{
    Q_FOREACH(QPointer<Config> config, watchedConfigs) {
        if (config) {
            backend->updateConfig(config);
        }
    }
}

ConfigMonitor *ConfigMonitor::instance()
{
    static ConfigMonitor *s_instance;

    if (s_instance == 0) {
        s_instance = new ConfigMonitor();
    }

    return s_instance;
}

ConfigMonitor::ConfigMonitor():
    QObject(),
    d(new Private())
{
}

ConfigMonitor::~ConfigMonitor()
{
    delete d;
}

void ConfigMonitor::addConfig(Config *config)
{
    if (!d->watchedConfigs.contains(QPointer<Config>(config))) {
        d->watchedConfigs << QPointer<Config>(config);
    }
}

void ConfigMonitor::removeConfig(Config *config)
{
    if (d->watchedConfigs.contains(QPointer<Config>(config))) {
        d->watchedConfigs.removeAll(QPointer<Config>(config));
    }
}

void ConfigMonitor::notifyUpdate()
{
    d->updateConfigs();

    Q_EMIT configurationChanged();
}


#include "configmonitor.moc"
