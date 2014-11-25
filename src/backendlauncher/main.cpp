/*
 * Copyright (C) 2014  Daniel Vratil <dvratil@redhat.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#include <QGuiApplication>
#include <QCommandLineParser>
#include <QDebug>

#include "backendloader.h"
#include "backenddbuswrapper.h"

#include <src/abstractbackend.h>

int main(int argc, char **argv)
{
    QGuiApplication app(argc, argv);

    BackendLoader launcher;

    QCommandLineOption backendOption(QLatin1String("backend"),
                                     QLatin1String("Backend to load. When not specified, BackendLauncher will "
                                                   "try to load the best backend for current platform."),
                                     QLatin1String("backend"));
    QCommandLineParser parser;
    parser.addOption(backendOption);
    parser.addHelpOption();

    parser.process(app);

    bool success = false;
    if (parser.isSet(backendOption)) {
        success = launcher.loadBackend(parser.value(backendOption));
    } else {
        success = launcher.loadBackend();
    }

    // We failed to load any backend: abort immediatelly
    if (!success) {
        return BackendLoader::BackendFailedToLoad;
    }

    // Check if another Backend Launcher with this particual backend is already running
    const bool alreadyRunning = launcher.checkIsAlreadyRunning();
    if (alreadyRunning) {
        // If it is, let caller now it's DBus service name and terminate
        printf("%s", qPrintable(launcher.backend()->serviceName()));
        fflush(stdout);
        return BackendLoader::BackendAlreadyExists;
    }


    // Create BackendDBusWrapper that takes implements the DBus interface and translates
    // DBus calls to backend implementations. It will also take care of terminating this
    // launcher when no other KScreen-enabled processes are running
    BackendDBusWrapper backendWrapper(launcher.backend());
    if (!backendWrapper.init()) {
        return BackendLoader::BackendFailedToLoad;
    }

    // Now let caller now what's our DBus service name, so it can connect to us
    printf("%s", qPrintable(launcher.backend()->serviceName()));
    fflush(stdout);

    // And go!
    return app.exec();
}
