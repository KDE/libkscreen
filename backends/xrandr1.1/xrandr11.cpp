/*************************************************************************************
 *  Copyright (C) 2013 by Alejandro Fiestas Olivares <afiestas@kde.org>              *
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

#include "xrandr11.h"
#include "wrapper.h"

#include <xcb/xcb.h>
#include <xcb/randr.h>

#include <QtCore/QString>
#include <QtCore/QDebug>
#include <QtCore/qplugin.h>

#include <kscreen/config.h>
#include <kscreen/edid.h>

Q_EXPORT_PLUGIN2(XRandR11, XRandR11)

XRandR11::XRandR11(QObject* parent)
 : QObject(parent)
 , m_valild(false)
{
    xcb_randr_query_version_reply_t* version;
    version = xcb_randr_query_version_reply(connection(), xcb_randr_query_version(connection(), XCB_RANDR_MAJOR_VERSION, XCB_RANDR_MINOR_VERSION), 0);
    if (!version) {
        qDebug() << "Can't get XRandR version";
        return;
    }
    if (version->minor_version > 1) {
        qWarning() << "This backend is only for XRandR 1.1, your version is: " << version->major_version << "." << version->minor_version;
//         return;
    }

    m_valild = true;
}

XRandR11::~XRandR11()
{

}

QString XRandR11::name() const
{
    return "XRandR 1.1";
}

bool XRandR11::isValid() const
{
    return m_valild;
}

KScreen::Config* XRandR11::config() const
{
    KScreen::Config* config = new KScreen::Config();

    int screenId = QX11Info().screen();
    xcb_screen_t* xcbScreen = screen_of_display(connection(), screenId);

    ScreenSize size(xcbScreen->root);

    KScreen::Screen* screen = new KScreen::Screen();
    screen->setId(screenId);
    screen->setCurrentSize(QSize(xcbScreen->width_in_pixels, xcbScreen->height_in_pixels));
    screen->setMaxSize(QSize(size->max_width, size->max_height));
    screen->setMinSize(QSize(size->min_width, size->min_height));

    config->setScreen(screen);

    KScreen::OutputList outputs;
    KScreen::Output* output = new KScreen::Output();
    output->setId(1);

    output->setConnected(true);
    output->setEnabled(true);
    output->setName(QLatin1String("Default"));
    output->setPos(QPoint(0,0));
    output->setPrimary(true);

    outputs.insert(1, output);
    config->setOutputs(outputs);


    return config;
}

void XRandR11::setConfig(KScreen::Config* config) const
{

}

KScreen::Edid* XRandR11::edid(int outputId) const
{
    return new KScreen::Edid();
}

void XRandR11::updateConfig(KScreen::Config* config) const
{

}

#include "xrandr11.moc"