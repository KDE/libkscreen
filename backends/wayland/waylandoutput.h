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

#ifndef KSCREEN_WAYLAND_OUTPUT_H
#define KSCREEN_WAYLAND_OUTPUT_H

#include "abstractbackend.h"

#include "config.h"
#include "output.h"

#include <QScreen>
#include <QSize>
#include <QLoggingCategory>
#include <KWayland/Client/output.h>


namespace KScreen
{

class WaylandOutput : public KWayland::Client::Output
{
    Q_OBJECT

public:
    explicit WaylandOutput(QObject *parent = 0);
    virtual ~WaylandOutput();

    KScreen::OutputPtr toKScreenOutput(KScreen::ConfigPtr &parent) const;
    void updateKScreenOutput(KScreen::OutputPtr &output) const;

    /**
     * Access to the Output's Edid object.
     */
    //KScreen::Edid edid();

    quint32 id() const;
    void setId(const quint32 newId);


Q_SIGNALS:
    void complete();

private:
    /**
     * Notify users after changes have been applied.
     */
    void flush();
    void update();
    void updateModes();
    QString modeName(const KWayland::Client::Output::Mode &m) const;

    mutable QSharedPointer<KScreen::Edid> m_edid;
    quint32 m_id;

    /** Check if we consider this object to be complete (i.e. done initializing).*/
    bool isComplete();
    /** Track if we've emitted the complete() signal, as to not do it twice. */
    bool m_completed;
};

} // namespace

#endif
