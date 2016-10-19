/*************************************************************************************
 *  Copyright (C) 2012 by Alejandro Fiestas Olivares <afiestas@kde.org>              *
 *  Copyright (C) 2014 by Daniel Vrátil <dvratil@redhat.com>                         *
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
#include "abstractbackend.h"
#include "backendmanager_p.h"
#include "debug_p.h"

#include <QStringList>
#include <QPointer>
#include <QRect>

using namespace KScreen;

class Output::Private
{
  public:
    Private():
        id(0),
        type(Unknown),
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
        sizeMm(other.sizeMm),
        pos(other.pos),
        size(other.size),
        rotation(other.rotation),
        connected(other.connected),
        enabled(other.enabled),
        primary(other.primary)
    {
        Q_FOREACH (const ModePtr &otherMode, other.modeList) {
            modeList.insert(otherMode->id(), otherMode->clone());
        }
        if (other.edid) {
            edid = other.edid->clone();
        }
    }

    QString biggestMode(const ModeList& modes) const;
    bool compareModeList(const ModeList& before, const ModeList& after);

    int id;
    QString name;
    Type type;
    QString icon;
    ModeList modeList;
    QList<int> clones;
    QString currentMode;
    QString preferredMode;
    QStringList preferredModes;
    QSize sizeMm;
    QPoint pos;
    QSize size;
    Rotation rotation;
    bool connected;
    bool enabled;
    bool primary;

    mutable QPointer<Edid> edid;
};

bool Output::Private::compareModeList(const ModeList& before, const ModeList &after)
{
    if (before.keys() != after.keys()) {
        return false;
    }
    for (const QString &key : before.keys()) {
        const auto mb = before.value(key);
        const auto ma = after.value(key);
        if (mb->id() != ma->id()) {
            return false;
        }
        if (mb->size() != ma->size()) {
            return false;
        }
        if (mb->refreshRate() != ma->refreshRate()) {
            return false;
        }
        if (mb->name() != ma->name()) {
            return false;
        }
    }
    // They're the same
    return true;
}


QString Output::Private::biggestMode(const ModeList& modes) const
{
    int area, total = 0;
    KScreen::ModePtr biggest;
    Q_FOREACH(const KScreen::ModePtr &mode, modes) {
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

Output::Output()
 : QObject(0)
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

OutputPtr Output::clone() const
{
    return OutputPtr(new Output(new Private(*d)));
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

ModePtr Output::mode(const QString& id) const
{
    if (!d->modeList.contains(id)) {
        return ModePtr();
    }

    return d->modeList[id];
}

ModeList Output::modes() const
{
    return d->modeList;
}

void Output::setModes(const ModeList &modes)
{
    bool changed = !d->compareModeList(d->modeList, modes);
    d->modeList = modes;
    if (changed) {
        qDebug() << "modelist changed!";
        emit modesChanged();
        emit outputChanged();
    }
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

ModePtr Output::currentMode() const
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
    KScreen::ModePtr biggest;
    KScreen::ModePtr candidateMode;
    Q_FOREACH(const QString &modeId, d->preferredModes) {
        candidateMode = mode(modeId);
        area = candidateMode->size().width() * candidateMode->size().height();
        if (area < total) {
            continue;
        }
        if (area == total && biggest && candidateMode->refreshRate() < biggest->refreshRate()) {
            continue;
        }
        if (area == total && biggest && candidateMode->refreshRate() > biggest->refreshRate()) {
            biggest = candidateMode;
            continue;
        }

        total = area;
        biggest = candidateMode;
    }

    Q_ASSERT_X(biggest, "preferredModeId", "biggest mode must exist");

    d->preferredMode = biggest->id();
    return d->preferredMode;
}

ModePtr Output::preferredMode() const
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

QSize Output::size() const
{
    return d->size;
}

void Output::setSize(const QSize& size)
{
    if (d->size == size) {
        return;
    }

    d->size = size;

    Q_EMIT sizeChanged();
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

void Output::setEdid(const QByteArray& rawData)
{
    Q_ASSERT(d->edid == 0);
    d->edid = new Edid(rawData);
}

Edid *Output::edid() const
{
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

QRect Output::geometry() const
{
    if (!currentMode()) {
        return QRect();
    }

    // We can't use QRect(d->pos, d->size), because d->size does not reflect the
    // actual rotation() set by caller, it's only updated when we get update from
    // KScreen, but not when user changes mode or rotation manually
    return isHorizontal()
            ? QRect(d->pos, currentMode()->size())
            : QRect(d->pos, currentMode()->size().transposed());
}

void Output::apply(const OutputPtr& other)
{
    typedef void (KScreen::Output::*ChangeSignal)();
    QList<ChangeSignal> changes;

    // We block all signals, and emit them only after we have set up everything
    // This is necessary in order to prevent clients from accessing inconsistent
    // outputs from intermediate change signals
    const bool keepBlocked = signalsBlocked();
    blockSignals(true);
    if (d->name != other->d->name) {
        changes << &Output::outputChanged;
        setName(other->d->name);
    }
    if (d->type != other->d->type) {
        changes << &Output::outputChanged;
        setType(other->d->type);
    }
    if (d->icon != other->d->icon) {
        changes << &Output::outputChanged;
        setIcon(other->d->icon);
    }
    if (d->pos != other->d->pos) {
        changes << &Output::posChanged;
        setPos(other->pos());
    }
    if (d->rotation != other->d->rotation) {
        changes << &Output::rotationChanged;
        setRotation(other->d->rotation);
    }
    if (d->currentMode != other->d->currentMode) {
        changes << &Output::currentModeIdChanged;
        setCurrentModeId(other->d->currentMode);
    }
    if (d->connected != other->d->connected) {
        changes << &Output::isConnectedChanged;
        setConnected(other->d->connected);
    }
    if (d->enabled != other->d->enabled) {
        changes << &Output::isEnabledChanged;
        setEnabled(other->d->enabled);
    }
    if (d->primary != other->d->primary) {
        changes << &Output::isPrimaryChanged;
        setPrimary(other->d->primary);
    }
    if (d->clones != other->d->clones) {
        changes << &Output::clonesChanged;
        setClones(other->d->clones);;
    }
    if (!d->compareModeList(d->modeList, other->d->modeList)) {
        qDebug() << "Queueing change signal for modelist!";
        changes << &Output::outputChanged;
    }

    setPreferredModes(other->d->preferredModes);
    ModeList modes;
    Q_FOREACH (const ModePtr &otherMode, other->modes()) {
        modes.insert(otherMode->id(), otherMode->clone());
    }
    setModes(modes);

    // Non-notifyable changes
    if (other->d->edid) {
        delete d->edid;
        d->edid = other->d->edid->clone();
    }

    blockSignals(keepBlocked);

    while (!changes.isEmpty()) {
        const ChangeSignal &sig = changes.first();
        Q_EMIT (this->*sig)();
        changes.removeAll(sig);
    }
}

QDebug operator<<(QDebug dbg, const KScreen::OutputPtr &output)
{
    if(output) {
        dbg << "KScreen::Output(" << output->id() << " "
                                  << output->name()
                                  << (output->isConnected() ? "connected" : "disconnected")
                                  << (output->isEnabled() ? "enabled" : "disabled")
                                  << output->pos() << output->size()
                                  << output->currentModeId()
                                  << ")";
    } else {
        dbg << "KScreen::Output(NULL)";
    }
    return dbg;
}
