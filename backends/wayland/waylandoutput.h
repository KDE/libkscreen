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

namespace KWayland {
    namespace Client {
        class Output;
    }
}

namespace KScreen
{

/** @class WaylandMode
 *  A typedef'ed list of quint32 holding:
 *  - width
 *  - height
 *  - refresh rate
 *  - whether it is the current mode (0 - it's not, 1 - it's the current mode)
 *
 *  This is used as internal representation of the modes of an output.
 */
typedef QList<quint32> WaylandMode;

class WaylandOutput : public QObject
{
    Q_OBJECT

public:
    explicit WaylandOutput(KWayland::Client::Output *wloutput, QObject *parent = 0);
    virtual ~WaylandOutput();

    KScreen::Output* toKScreenOutput(KScreen::Config *parent) const;
    void updateKScreenOutput(KScreen::Output *output) const;

    /** QScreen doesn't support querying for the EDID, this function centralizes
     *  creating the EDID per output, anyway, so a drop-in solution will "just work".
     */
    KScreen::Edid *edid();

    quint32 id() const;
    void setId(const quint32 newId);

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

    void addMode(quint32 w, quint32 h, quint32 refresh, bool current);
    /*
     * notify users after changes have been applied.
     */
    void flush();

    KWayland::Client::Output* output() const;

Q_SIGNALS:
    void complete();

private:
    void updateFromQScreen(const QScreen *qscreen);
    mutable QPointer<KScreen::Edid> m_edid;
    quint32 m_id;

    KWayland::Client::Output *m_output;
    QSize m_physicalSize;
    QPoint m_globalPosition;
    QString m_manufacturer;
    QString m_model;
    QSize m_pixelSize;
    int m_refreshRate;

    QHash<QString, KScreen::WaylandMode> m_modes;
};

} // namespace

#endif
