/*************************************************************************************
 *  Copyright 2014 Sebastian Kügler <sebas@kde.org>                                  *
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

#ifndef WAYLAND_OUTPUT_H
#define WAYLAND_OUTPUT_H

#include "../abstractbackend.h"
//#include "qscreenconfig.h"

#include "config.h"
#include "output.h"

#include <QScreen>
#include <QtCore/QSize>
#include <QLoggingCategory>

class wl_output;

namespace KScreen
{

class WaylandOutput : public QObject
{
    Q_OBJECT

public:
    explicit WaylandOutput(wl_output *wloutput, QObject *parent = 0);
    virtual ~WaylandOutput();

    KScreen::Output* toKScreenOutput(KScreen::Config *parent) const;
    void updateKScreenOutput(KScreen::Output *output) const;

    /** QScreen doesn't support querying for the EDID, this function centralizes
     *  creating the EDID per output, anyway, so a drop-in solution will "just work".
     */
    KScreen::Edid *edid();

    int id() const;
    void setId(const int newId);

    const QSize &physicalSize() const;
    const QPoint &globalPosition() const;
    const QString &manufacturer() const;
    const QString &model() const;
    const QSize &pixelSize() const;
    int refreshRate() const;

    void setPhysicalSize(const QSize &size);
    void setGlobalPosition(const QPoint &pos);
    void setManufacturer(const QString &manufacturer);
    void setModel(const QString &model);
    void setPixelSize(const QSize &size);
    void setRefreshRate(int refreshRate);
    /*
     * notify users after changes have been applied.
     */
    void flush();

    wl_output* output() const;

private:
    void updateFromQScreen(const QScreen *qscreen);
    const QScreen *m_qscreen;
    mutable QPointer<KScreen::Edid> m_edid;
    int m_id;

    wl_output *m_output;
    QSize m_physicalSize;
    QPoint m_globalPosition;
    QString m_manufacturer;
    QString m_model;
    QSize m_pixelSize;
    int m_refreshRate;
};

} // namespace

#endif
