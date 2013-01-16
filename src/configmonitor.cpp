/*
 * Copyright 2012  Dan Vratil <dvratil@redhat.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

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
