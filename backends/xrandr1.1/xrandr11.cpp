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

#include "xrandr11.h"
#include "wrapper.h"
#include "../xrandr/xrandrxcbhelper.h"

#include "config.h"
#include "edid.h"
#include "output.h"

#include <xcb/xcb.h>
#include <xcb/randr.h>

#include <QtCore/QString>
#include <QtCore/QDebug>
#include <QtCore/qplugin.h>

Q_LOGGING_CATEGORY(KSCREEN_XRANDR11, "kscreen.xrandr11")

XRandR11::XRandR11()
 : KScreen::AbstractBackend()
 , m_valid(false)
 , m_x11Helper(0)
 , m_currentConfig(new KScreen::Config)
 , m_currentTimestamp(0)
{
    QLoggingCategory::setFilterRules(QLatin1Literal("kscreen.xrandr11.debug = true"));

    xcb_generic_error_t *error = 0;
    xcb_randr_query_version_reply_t* version;
    version = xcb_randr_query_version_reply(connection(), xcb_randr_query_version(connection(), XCB_RANDR_MAJOR_VERSION, XCB_RANDR_MINOR_VERSION), &error);

    if (!version || error) {
        free(error);
        qCDebug(KSCREEN_XRANDR11) << "Can't get XRandR version";
        return;
    }
    if (version->major_version != 1 || version->minor_version != 1) {
        qCDebug(KSCREEN_XRANDR11) << "This backend is only for XRandR 1.1, your version is: " << version->major_version << "." << version->minor_version;
        return;
    }

    m_x11Helper = new XRandRXCBHelper();

    connect(m_x11Helper, SIGNAL(outputsChanged()), SLOT(updateConfig()));

    m_valid = true;
}

XRandR11::~XRandR11()
{
    closeConnection();
    delete m_x11Helper;
}

QString XRandR11::name() const
{
    return QLatin1Literal("XRandR 1.1");
}

QString XRandR11::serviceName() const
{
    return QLatin1Literal("org.kde.KScreen.Backend.XRandR11");
}


bool XRandR11::isValid() const
{
    return m_valid;
}

KScreen::ConfigPtr XRandR11::config() const
{
    KScreen::ConfigPtr config(new KScreen::Config);

    const int screenId = QX11Info::appScreen();
    xcb_screen_t* xcbScreen = screen_of_display(connection(), screenId);
    const ScreenInfo info(xcbScreen->root);
    const ScreenSize size(xcbScreen->root);

    if (info.isNull() || size.isNull()) {
        return 0;
    }

    if (info->config_timestamp == m_currentTimestamp) {
        return m_currentConfig;
    }

    KScreen::ScreenPtr screen(new KScreen::Screen);
    screen->setId(screenId);
    screen->setCurrentSize(QSize(xcbScreen->width_in_pixels, xcbScreen->height_in_pixels));
    screen->setMaxSize(QSize(size->max_width, size->max_height));
    screen->setMinSize(QSize(size->min_width, size->min_height));
    screen->setMaxActiveOutputsCount(1);

    config->setScreen(screen);

    KScreen::OutputList outputs;
    KScreen::OutputPtr output(new KScreen::Output);
    output->setId(1);

    output->setConnected(true);
    output->setEnabled(true);
    output->setName(QLatin1String("Default"));
    output->setPos(QPoint(0,0));
    output->setPrimary(true);
    output->setRotation((KScreen::Output::Rotation) info->rotation);
    output->setSizeMm(QSize(xcbScreen->width_in_millimeters, xcbScreen->height_in_millimeters));

    outputs.insert(1, output);
    config->setOutputs(outputs);

    KScreen::ModePtr mode;
    KScreen::ModeList modes;

    xcb_randr_refresh_rates_iterator_t ite =  xcb_randr_get_screen_info_rates_iterator(info.data());
    xcb_randr_screen_size_t* sizes = xcb_randr_get_screen_info_sizes(info.data());
    for (int x = 0; x < info->nSizes; x++) {
        const uint16_t* rates = xcb_randr_refresh_rates_rates(ite.data);
        const int nrates = xcb_randr_refresh_rates_rates_length(ite.data);

        for (int j = 0; j < nrates; j++) {
            mode = KScreen::ModePtr(new KScreen::Mode);
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

void XRandR11::setConfig(const KScreen::ConfigPtr &config)
{
    const KScreen::OutputPtr output = config->outputs().take(1);
    const KScreen::ModePtr mode = output->currentMode();

    const int screenId = QX11Info::appScreen();
    xcb_screen_t* xcbScreen = screen_of_display(connection(), screenId);

    const ScreenInfo info(xcbScreen->root);
    xcb_generic_error_t *err;
    xcb_randr_set_screen_config_cookie_t cookie;
    xcb_randr_set_screen_config_reply_t *result;
    const int sizeId = mode->id().split("-").first().toInt();
    cookie = xcb_randr_set_screen_config(connection(), xcbScreen->root, CurrentTime, info->config_timestamp, sizeId,
                                         (short) output->rotation(), mode->refreshRate());
    result = xcb_randr_set_screen_config_reply(connection(), cookie, &err);

    delete result;
}

void XRandR11::updateConfig()
{
    m_currentConfig = config();
    Q_EMIT configChanged(m_currentConfig);
}
