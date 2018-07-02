/********************************************************************
 K Win - the KDE window manager
 This file is part of the KDE project.

Copyright (C) 2012, 2013 Martin Gräßlin <mgraesslin@kde.org>
Copyright (C) 2015       Daniel Vrátil <dvratil@redhat.com>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*********************************************************************/

#include "xcbwrapper.h"

static xcb_connection_t *sXRandR11XCBConnection = nullptr;

xcb_connection_t* XCB::connection()
{
    // Use our own connection to make sure that we won't mess up Qt's connection
    // if something goes wrong on our side.
    if (sXRandR11XCBConnection == nullptr) {
        sXRandR11XCBConnection = xcb_connect(nullptr, nullptr);
    }
    return sXRandR11XCBConnection;
}

void XCB::closeConnection()
{
    if (sXRandR11XCBConnection) {
        xcb_disconnect(sXRandR11XCBConnection);
        sXRandR11XCBConnection = nullptr;
    }
}

xcb_screen_t* XCB::screenOfDisplay(xcb_connection_t* c, int screen)
{
    for (auto iter = xcb_setup_roots_iterator(xcb_get_setup(c));
         iter.rem; --screen, xcb_screen_next(&iter))
    {
        if (screen == 0) {
            return iter.data;
        }
    }

    return nullptr;
}


XCB::GrabServer::GrabServer()
{
    xcb_grab_server(connection());
}

XCB::GrabServer::~GrabServer()
{
    xcb_ungrab_server(connection());
    xcb_flush(connection());
}
