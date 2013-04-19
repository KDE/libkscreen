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
#include "../xrandr/xrandrx11helper.h"

#include "config.h"
#include "edid.h"
#include "configmonitor.h"

#include <xcb/xcb.h>
#include <xcb/randr.h>

#include <QtCore/QString>
#include <QtCore/QDebug>
#include <QtCore/qplugin.h>

#include <KDebug>

Q_EXPORT_PLUGIN2(XRandR11, XRandR11)

XRandR11::XRandR11(QObject* parent)
 : QObject(parent)
 , m_valid(false)
 , m_x11Helper(new XRandRX11Helper())
 , m_currentConfig(0)
 , m_currentTimestamp(0)
{
    xcb_randr_query_version_reply_t* version;
    version = xcb_randr_query_version_reply(connection(), xcb_randr_query_version(connection(), XCB_RANDR_MAJOR_VERSION, XCB_RANDR_MINOR_VERSION), 0);
    if (!version) {
        qDebug() << "Can't get XRandR version";
        return;
    }
    if (version->minor_version > 1) {
        qWarning() << "This backend is only for XRandR 1.1, your version is: " << version->major_version << "." << version->minor_version;
        return;
    }

    connect(m_x11Helper, SIGNAL(outputsChanged()), SLOT(updateConfig()));

    m_valid = true;
}

XRandR11::~XRandR11()
{
    delete m_currentConfig;
    delete m_x11Helper;
}

QString XRandR11::name() const
{
    return "XRandR 1.1";
}

bool XRandR11::isValid() const
{
    return m_valid;
}

KScreen::Config* XRandR11::config() const
{
    KScreen::Config* config = new KScreen::Config();

    int screenId = QX11Info().screen();
    xcb_screen_t* xcbScreen = screen_of_display(connection(), screenId);
    ScreenInfo info(xcbScreen->root);
    ScreenSize size(xcbScreen->root);

    if (info->config_timestamp == m_currentTimestamp) {
        return m_currentConfig;
    }

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
    output->setRotation((KScreen::Output::Rotation) info->rotation);

    outputs.insert(1, output);
    config->setOutputs(outputs);

    KScreen::Mode *mode = 0;
    KScreen::ModeList modes;

    int nrates;
    uint16_t* rates;
    xcb_randr_refresh_rates_iterator_t ite =  xcb_randr_get_screen_info_rates_iterator(info.data());
    xcb_randr_screen_size_t* sizes = xcb_randr_get_screen_info_sizes(info.data());
    for (int x = 0; x < info->nSizes; x++) {
        rates = xcb_randr_refresh_rates_rates(ite.data);
        nrates = xcb_randr_refresh_rates_rates_length(ite.data);

        for (int j = 0; j < nrates; j++) {
            mode = new KScreen::Mode();
            mode->setId(QString::number(x) + "-" + QString::number(j));
            mode->setSize(QSize(sizes[x].width, sizes[x].height));
            mode->setRefreshRate((float) rates[j]);
            mode->setName(QString(QString::number(sizes[x].width) + "x" + QString::number(sizes[x].height)));

            if (x == info->sizeID && rates[j] == info->rate) {
                output->setCurrentModeId(mode->id());
            }
            modes.insert(mode->id(), mode);
        }

        xcb_randr_refresh_rates_next(&ite);
    }

    output->setModes(modes);
    return config;
}

void XRandR11::setConfig(KScreen::Config* config) const
{
    KScreen::Output* output = config->outputs().take(1);
    KScreen::Mode *mode = output->currentMode();

    int screenId = QX11Info().screen();
    xcb_screen_t* xcbScreen = screen_of_display(connection(), screenId);

    ScreenInfo info(xcbScreen->root);
    xcb_generic_error_t *err;
    xcb_randr_set_screen_config_cookie_t cookie;
    xcb_randr_set_screen_config_reply_t *result;
    int sizeId = mode->id().split("-").first().toInt();
    cookie = xcb_randr_set_screen_config(connection(), xcbScreen->root, CurrentTime, info->config_timestamp, sizeId,
                                       (short) output->rotation(), mode->refreshRate());
    result = xcb_randr_set_screen_config_reply(connection(), cookie, &err);

    delete result;
}

KScreen::Edid* XRandR11::edid(int outputId) const
{
    Q_UNUSED(outputId)
    return new KScreen::Edid();
}

void XRandR11::updateConfig(KScreen::Config* config) const
{
    KScreen::Output* output = config->output(1);
    KScreen::Output *current = m_currentConfig->output(1);
    output->setCurrentModeId(current->currentModeId());
    output->setRotation(current->rotation());
}

void XRandR11::updateConfig()
{
    delete m_currentConfig;
    m_currentConfig = config();
    KScreen::ConfigMonitor::instance()->notifyUpdate();
}

extern int dXndr() { static int s_area = KDebug::registerArea("KSRandr11", false); return s_area; }
#include "xrandr11.moc"
