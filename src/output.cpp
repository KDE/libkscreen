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

    Private(const Private &other):
        id(other.id),
        name(other.name),
        type(other.type),
        icon(other.icon),
        clones(other.clones),
        currentMode(other.currentMode),
        preferredMode(other.preferredMode),
        preferredModes(other.preferredModes),
        size(other.size),
        sizeMm(other.sizeMm),
        pos(other.pos),
        rotation(other.rotation),
        connected(other.connected),
        enabled(other.enabled),
        primary(other.primary)
    {
        Q_FOREACH (Mode *otherMode, other.modeList) {
            modeList.insert(otherMode->id(), otherMode->clone());
        }
        if (other.edid) {
            edid = other.edid->clone();
        }
    }

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
    QSize sizeMm;
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

Output::Output(Output::Private *dd)
 : QObject()
 , d(dd)
{
}

Output::~Output()
{
    delete d;
}

Output *Output::clone() const
{
    Output *output = new Output(new Private(*d));
    // Make sure the new output takes ownership of the cloned modes
    Q_FOREACH (Mode *mode, output->d->modeList) {
        mode->setParent(output);
    }
    if (output->d->edid) {
        output->d->edid->setParent(output);
    }

    return output;
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

QSize Output::sizeMm() const
{
    return d->sizeMm;
}

void Output::setSizeMm(const QSize &size)
{
    d->sizeMm = size;
}

} //KScreen namespace

QDebug operator<<(QDebug dbg, const KScreen::Output *output)
{
    dbg << "KScreen::Output(Id:" << output->id() <<", Name:" << output->name() << ")";
    return dbg;
}

#include "output.moc"
