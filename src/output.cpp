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

class Output::Private
{
  public:
    Private():
        id(0),
        rotation(None),
        connected(false),
        enabled(false),
        primary(false),
        edid(0)
    {}

    QString biggestMode(const ModeList& modes) const;

    int id;
    QString name;
    Type type;
    QString icon;
    ModeList modeList;
    QList<int> clones;
    QString currentMode;
    QString preferredMode;
    QStringList preferredModes;
    QSize size;
    QPoint pos;
    Rotation rotation;
    bool connected;
    bool enabled;
    bool primary;

    mutable QPointer<Edid> edid;
};

QString Output::Private::biggestMode(const ModeList& modes) const
{
    int area, total = 0;
    KScreen::Mode* biggest = 0;
    Q_FOREACH(KScreen::Mode* mode, modes) {
        area = mode->size().width() * mode->size().height();
        if (area < total) {
            continue;
        }
        if (area == total && mode->refreshRate() < biggest->refreshRate()) {
            continue;
        }
        if (area == total && mode->refreshRate() > biggest->refreshRate()) {
            biggest = mode;
            continue;
        }

        total = area;
        biggest = mode;
    }

    if (!biggest) {
        return 0;
    }

    return biggest->id();
}

Output::Output(QObject *parent)
 : QObject(parent)
 , d(new Private())
{

}

Output::~Output()
{
    delete d;
}

int Output::id() const
{
    return d->id;
}

void Output::setId(int id)
{
    if (d->id == id) {
        return;
    }

    d->id = id;

    Q_EMIT outputChanged();
}

QString Output::name() const
{
    return d->name;
}

void Output::setName(const QString& name)
{
    if (d->name == name) {
        return;
    }

    d->name = name;

    Q_EMIT outputChanged();
}

Output::Type Output::type() const
{
    return d->type;
}

void Output::setType(Type type)
{
    if (d->type == type) {
        return;
    }

    d->type = type;

    Q_EMIT outputChanged();
}

QString Output::icon() const
{
    return d->icon;
}

void Output::setIcon(const QString& icon)
{
    if (d->icon == icon) {
        return;
    }

    d->icon = icon;

    Q_EMIT outputChanged();
}

Mode* Output::mode(const QString& id) const
{
    if (!d->modeList.contains(id)) {
        return 0;
    }

    return d->modeList[id];
}

QHash< QString, Mode* > Output::modes() const
{
    return d->modeList;
}

void Output::setModes(ModeList modes)
{
    if (!d->modeList.isEmpty()) {
        qDeleteAll(d->modeList);
    }
    d->modeList = modes;
}

QString Output::currentModeId() const
{
    return d->currentMode;
}

void Output::setCurrentModeId(const QString& mode)
{
    if (d->currentMode == mode) {
        return;
    }

    d->currentMode = mode;

    Q_EMIT currentModeIdChanged();
}

Mode *Output::currentMode() const
{
    return d->modeList.value(d->currentMode);
}

void Output::setPreferredModes(const QStringList &modes)
{
    d->preferredMode = QString();
    d->preferredModes = modes;
}

QStringList Output::preferredModes() const
{
    return d->preferredModes;
}

QString Output::preferredModeId() const
{
    if (!d->preferredMode.isEmpty()) {
        return d->preferredMode;
    }
    if (d->preferredModes.isEmpty()) {
        return d->biggestMode(modes());
    }

    int area, total = 0;
    KScreen::Mode* biggest = 0;
    KScreen::Mode* candidateMode = 0;
    Q_FOREACH(const QString &modeId, d->preferredModes) {
        candidateMode = mode(modeId);
        area = candidateMode->size().width() * candidateMode->size().height();
        if (area < total) {
            continue;
        }
        if (area == total && candidateMode->refreshRate() < biggest->refreshRate()) {
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
        return d->preferredMode;
    }

    d->preferredMode = biggest->id();
    return d->preferredMode;
}

Mode* Output::preferredMode() const
{
    return d->modeList.value(preferredModeId());
}

QPoint Output::pos() const
{
    return d->pos;
}

void Output::setPos(const QPoint& pos)
{
    if (d->pos == pos) {
        return;
    }

    d->pos = pos;

    Q_EMIT posChanged();
}

Output::Rotation Output::rotation() const
{
    return d->rotation;
}

void Output::setRotation(Output::Rotation rotation)
{
    if (d->rotation == rotation) {
        return;
    }

    d->rotation = rotation;

    Q_EMIT rotationChanged();
}

bool Output::isConnected() const
{
    return d->connected;
}

void Output::setConnected(bool connected)
{
    if (d->connected == connected) {
        return;
    }

    d->connected = connected;

    Q_EMIT isConnectedChanged();
}

bool Output::isEnabled() const
{
    return d->enabled;
}

void Output::setEnabled(bool enabled)
{
    if (d->enabled == enabled) {
        return;
    }

    d->enabled = enabled;

    Q_EMIT isEnabledChanged();
}

bool Output::isPrimary() const
{
    return d->primary;
}

void Output::setPrimary(bool primary)
{
    if (d->primary == primary) {
        return;
    }

    d->primary = primary;

    Q_EMIT isPrimaryChanged();
}

QList<int> Output::clones() const
{
    return d->clones;
}

void Output::setClones(QList<int> outputlist)
{
    if (d->clones == outputlist) {
        return;
    }

    d->clones = outputlist;

    Q_EMIT clonesChanged();
}

Edid *Output::edid() const
{
    if (d->edid == 0) {
        AbstractBackend *backend = BackendLoader::backend();
        d->edid = backend->edid(d->id);
    }

    return d->edid;
}


} //KScreen namespace

#include "output.moc"
