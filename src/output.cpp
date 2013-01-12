/*************************************************************************************
 *  Copyright (C) 2012 by Alejandro Fiestas Olivares <afiestas@kde.org>              *
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

#include "output.h"
#include "mode.h"
#include "edid.h"
#include "backendloader.h"
#include <backends/abstractbackend.h>

#include <QtCore/QStringList>

namespace KScreen {

Output::Output(QObject *parent)
 : QObject(parent)
 , m_id(0)
 , m_currentMode(0)
 , m_rotation(None)
 , m_connected(false)
 , m_enabled(false)
 , m_primary(false)
 , m_edid(0)
{

}

Output::~Output()
{

}

int Output::id() const
{
    return m_id;
}

void Output::setId(int id)
{
    if (m_id == id) {
        return;
    }

    m_id = id;

    Q_EMIT outputChanged();
}

QString Output::name() const
{
    return m_name;
}

void Output::setName(const QString& name)
{
    if (m_name == name) {
        return;
    }

    m_name = name;

    Q_EMIT outputChanged();
}

QString Output::type() const
{
    return m_type;
}

void Output::setType(const QString& type)
{
    if (m_type == type) {
        return;
    }

    m_type = type;

    Q_EMIT outputChanged();
}

QString Output::icon() const
{
    return m_icon;
}

void Output::setIcon(const QString& icon)
{
    if (m_icon == icon) {
        return;
    }

    m_icon = icon;

    Q_EMIT outputChanged();
}

Mode* Output::mode(int id) const
{
    if (!m_modeList.contains(id)) {
        return 0;
    }

    return m_modeList[id];
}

QHash< int, Mode* > Output::modes() const
{
    return m_modeList;
}

void Output::setModes(ModeList modes)
{
    if (!m_modeList.isEmpty()) {
        qDeleteAll(m_modeList);
    }
    m_modeList = modes;
}

int Output::currentMode() const
{
    return m_currentMode;
}

void Output::setCurrentMode(int mode)
{
    if (m_currentMode == mode) {
        return;
    }

    m_currentMode = mode;

    Q_EMIT currentModeChanged();
}

void Output::setPreferredModes(QList<int> modes)
{
    m_preferredMode = 0;
    m_preferredModes = modes;
}

int Output::preferredMode() const
{
    if (m_preferredMode || m_preferredModes.isEmpty()) {
        return m_preferredMode;
    }

    int area, total = 0;
    KScreen::Mode* biggest = 0;
    KScreen::Mode* candidateMode = 0;
    Q_FOREACH(int modeId, m_preferredModes) {
        candidateMode = mode(modeId);
        area = candidateMode->size().width() * candidateMode->size().height();
        if (area < total) {
            continue;
        }
        if (area == total && biggest && candidateMode->refreshRate() > biggest->refreshRate()) {
            biggest = candidateMode;
            continue;
        }

        total = area;
        biggest = candidateMode;
    }

    if (!biggest) {
        return m_preferredMode;
    }

    m_preferredMode = biggest->id();
    return m_preferredMode;
}

QPoint Output::pos() const
{
    return m_pos;
}

void Output::setPos(const QPoint& pos)
{
    if (m_pos == pos) {
        return;
    }

    m_pos = pos;

    Q_EMIT posChanged();
}

Output::Rotation Output::rotation() const
{
    return m_rotation;
}

void Output::setRotation(Output::Rotation rotation)
{
    if (m_rotation == rotation) {
        return;
    }

    m_rotation = rotation;

    Q_EMIT rotationChanged();
}

bool Output::isConnected() const
{
    return m_connected;
}

void Output::setConnected(bool connected)
{
    if (m_connected == connected) {
        return;
    }

    m_connected = connected;

    Q_EMIT isConnectedChanged();
}

bool Output::isEnabled() const
{
    return m_enabled;
}

void Output::setEnabled(bool enabled)
{
    if (m_enabled == enabled) {
        return;
    }

    m_enabled = enabled;

    Q_EMIT isEnabledChanged();
}

bool Output::isPrimary() const
{
    return m_primary;
}

void Output::setPrimary(bool primary)
{
    if (m_primary == primary) {
        return;
    }

    m_primary = primary;

    Q_EMIT isPrimaryChanged();
}

bool Output::isEmbedded() const
{
    QStringList embedded;
    embedded << "LVDS";
    embedded << "IDP";
    embedded << "EDP";

    Q_FOREACH(const QString &pre, embedded) {
        if (m_name.toUpper().startsWith(pre)) {
            return true;
        }
    }

    return false;
}

QList<int> Output::clones()
{
    return m_clones;
}

void Output::setClones(QList<int> outputlist)
{
    if (m_clones == outputlist) {
        return;
    }

    m_clones = outputlist;

    Q_EMIT clonesChanged();
}

Edid *Output::edid() const
{
    if (m_edid == 0) {
        AbstractBackend *backend = BackendLoader::backend();
        m_edid = backend->edid(m_id);
    }

    return m_edid;
}


} //KScreen namespace
#include "output.moc"