/*************************************************************************************
 *  Copyright 2017 Sebastian Kügler <sebas@kde.org>                                  *
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

#include "tstool.h"

#include <QCoreApplication>
#include <QCommandLineParser>
#include <QGuiApplication>
#include <QLoggingCategory>
#include <QTimer>

#include <X11/Xlib.h>
#include <X11/extensions/XInput.h>


Q_LOGGING_CATEGORY(KSCREEN_TSTOOL, "kscreen.touchscreen")


static QTextStream cout(stdout);
static QTextStream cerr(stderr);

const static QString green = QStringLiteral("\033[01;32m");
const static QString red = QStringLiteral("\033[01;31m");
const static QString yellow = QStringLiteral("\033[01;33m");
const static QString blue = QStringLiteral("\033[01;34m");
const static QString bold = QStringLiteral("\033[01;39m");
const static QString cr = QStringLiteral("\033[0;0m");


using namespace KScreen;

TsTool::TsTool(QObject *parent)
    : QObject(parent)
{
}

TsTool::~TsTool()
{
}

void TsTool::start(QCommandLineParser *parser)
{
    m_parser = parser;
    if (m_parser->isSet("list")) {
        listTouchscreens();
    }

    // We need to kick the event loop, otherwise .quit() hangs
    QTimer::singleShot(0, qApp->quit);
}


void TsTool::listTouchscreens() const
{
    Display *display = XOpenDisplay(0);
    int nDevices = 0;
    XDeviceInfo *devices = XListInputDevices(display, &nDevices);
    for (int i = 0; i < nDevices; i++) {
        const char *name = devices[i].name;
        char *type = 0;
        if (devices[i].type) {
            type = XGetAtomName(display, devices[i].type);
        }
        if (QString::fromLocal8Bit(type) == QStringLiteral("TOUCH")) {
            cout << "Name: " << name << " Type: " << (type ? type : "unknown");
        }
        XFree(type);
    }
    XFreeDeviceList(devices);
    XCloseDisplay(display);

 }

void TsTool::setOptionList(const QStringList &positionalArgs)
{
    m_positionalArgs = positionalArgs;
}


