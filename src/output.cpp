/*
 *  SPDX-FileCopyrightText: 2012 Alejandro Fiestas Olivares <afiestas@kde.org>
 *  SPDX-FileCopyrightText: 2014 Daniel Vrátil <dvratil@redhat.com>
 *
 *  SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "output.h"
#include "abstractbackend.h"
#include "backendmanager_p.h"
#include "edid.h"
#include "kscreen_debug.h"
#include "mode.h"

#include <QCryptographicHash>
#include <QRect>
#include <QScopedPointer>
#include <QStringList>

using namespace KScreen;

class Q_DECL_HIDDEN Output::Private
{
public:
    Private()
        : id(0)
        , type(Unknown)
        , replicationSource(0)
        , rotation(None)
        , scale(1.0)
        , logicalSize(QSizeF())
        , connected(false)
        , enabled(false)
        , primary(false)
        , edid(nullptr)
    {
    }

    Private(const Private &other)
        : id(other.id)
        , name(other.name)
        , type(other.type)
        , icon(other.icon)
        , clones(other.clones)
        , replicationSource(other.replicationSource)
        , currentMode(other.currentMode)
        , preferredMode(other.preferredMode)
        , preferredModes(other.preferredModes)
        , sizeMm(other.sizeMm)
        , pos(other.pos)
        , size(other.size)
        , rotation(other.rotation)
        , scale(other.scale)
        , connected(other.connected)
        , enabled(other.enabled)
        , primary(other.primary)
        , followPreferredMode(other.followPreferredMode)
        , capabilities(other.capabilities)
        , overscan(other.overscan)
        , vrrPolicy(other.vrrPolicy)
        , rgbRange(other.rgbRange)
    {
        const auto otherModeList = other.modeList;
        for (const ModePtr &otherMode : otherModeList) {
            modeList.insert(otherMode->id(), otherMode->clone());
        }
        if (other.edid) {
            edid.reset(other.edid->clone());
        }
    }

    QString biggestMode(const ModeList &modes) const;
    bool compareModeList(const ModeList &before, const ModeList &after);

    int id;
    QString name;
    Type type;
    QString icon;
    ModeList modeList;
    QList<int> clones;
    int replicationSource;
    QString currentMode;
    QString preferredMode;
    QStringList preferredModes;
    QSize sizeMm;
    QPoint pos;
    QSize size;
    Rotation rotation;
    qreal scale;
    QSizeF logicalSize;
    bool connected;
    bool enabled;
    bool primary;
    bool followPreferredMode = false;
    Capabilities capabilities;
    uint32_t overscan = 0;
    VrrPolicy vrrPolicy = VrrPolicy::Automatic;
    RgbRange rgbRange = RgbRange::Automatic;

    QScopedPointer<Edid> edid;
};

bool Output::Private::compareModeList(const ModeList &before, const ModeList &after)
{
    if (before.count() != after.count()) {
        return false;
    }

    for (auto itb = before.constBegin(); itb != before.constEnd(); ++itb) {
        auto ita = after.constFind(itb.key());
        if (ita == after.constEnd()) {
            return false;
        }
        const auto &mb = itb.value();
        const auto &ma = ita.value();
        if (mb->id() != ma->id()) {
            return false;
        }
        if (mb->size() != ma->size()) {
            return false;
        }
        if (!qFuzzyCompare(mb->refreshRate(), ma->refreshRate())) {
            return false;
        }
        if (mb->name() != ma->name()) {
            return false;
        }
    }
    // They're the same
    return true;
}

QString Output::Private::biggestMode(const ModeList &modes) const
{
    int area, total = 0;
    KScreen::ModePtr biggest;
    for (const KScreen::ModePtr &mode : modes) {
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
        return QString();
    }

    return biggest->id();
}

Output::Output()
    : QObject(nullptr)
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

void Output::setName(const QString &name)
{
    if (d->name == name) {
        return;
    }

    d->name = name;

    Q_EMIT outputChanged();
}

// TODO KF6: remove this deprecated method
QString Output::hash() const
{
    if (edid() && edid()->isValid()) {
        return edid()->hash();
    }
    return name();
}

QString Output::hashMd5() const
{
    if (edid() && edid()->isValid()) {
        return edid()->hash();
    }
    const auto hash = QCryptographicHash::hash(name().toLatin1(), QCryptographicHash::Md5);
    return QString::fromLatin1(hash.toHex());
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

void Output::setIcon(const QString &icon)
{
    if (d->icon == icon) {
        return;
    }

    d->icon = icon;

    Q_EMIT outputChanged();
}

ModePtr Output::mode(const QString &id) const
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
        Q_EMIT modesChanged();
        Q_EMIT outputChanged();
    }
}

QString Output::currentModeId() const
{
    return d->currentMode;
}

void Output::setCurrentModeId(const QString &mode)
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

    int total = 0;
    KScreen::ModePtr biggest;
    KScreen::ModePtr candidateMode;
    for (const QString &modeId : qAsConst(d->preferredModes)) {
        candidateMode = mode(modeId);
        const int area = candidateMode->size().width() * candidateMode->size().height();
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

void Output::setPos(const QPoint &pos)
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

void Output::setSize(const QSize &size)
{
    if (d->size == size) {
        return;
    }

    d->size = size;

    Q_EMIT sizeChanged();
}

// TODO KF6: make the Rotation enum an enum class and align values with Wayland transformation property
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

qreal Output::scale() const
{
    return d->scale;
}

void Output::setScale(qreal factor)
{
    if (qFuzzyCompare(d->scale, factor)) {
        return;
    }
    d->scale = factor;
    Q_EMIT scaleChanged();
}

QSizeF Output::logicalSize() const
{
    if (d->logicalSize.isValid()) {
        return d->logicalSize;
    }

    QSizeF size = enforcedModeSize();
    if (!size.isValid()) {
        return QSizeF();
    }
    size = size / d->scale;

    // We can't use d->size, because d->size does not reflect the actual rotation() set by caller.
    // It is only updated when we get update from KScreen, but not when user changes mode or
    // rotation manually.

    if (!isHorizontal()) {
        size = size.transposed();
    }
    return size;
}

QSizeF Output::explicitLogicalSize() const
{
    return d->logicalSize;
}

void Output::setLogicalSize(const QSizeF &size)
{
    if (qFuzzyCompare(d->logicalSize.width(), size.width()) && qFuzzyCompare(d->logicalSize.height(), size.height())) {
        return;
    }
    d->logicalSize = size;
    Q_EMIT logicalSizeChanged();
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

void Output::setClones(const QList<int> &outputlist)
{
    if (d->clones == outputlist) {
        return;
    }

    d->clones = outputlist;

    Q_EMIT clonesChanged();
}

int Output::replicationSource() const
{
    return d->replicationSource;
}

void Output::setReplicationSource(int source)
{
    if (d->replicationSource == source) {
        return;
    }

    d->replicationSource = source;

    Q_EMIT replicationSourceChanged();
}

void Output::setEdid(const QByteArray &rawData)
{
    Q_ASSERT(d->edid.isNull());
    d->edid.reset(new Edid(rawData));
}

Edid *Output::edid() const
{
    return d->edid.data();
}

QSize Output::sizeMm() const
{
    return d->sizeMm;
}

void Output::setSizeMm(const QSize &size)
{
    d->sizeMm = size;
}

bool KScreen::Output::followPreferredMode() const
{
    return d->followPreferredMode;
}

void KScreen::Output::setFollowPreferredMode(bool follow)
{
    if (follow != d->followPreferredMode) {
        d->followPreferredMode = follow;
        Q_EMIT followPreferredModeChanged(follow);
    }
}

bool Output::isPositionable() const
{
    return isConnected() && isEnabled() && !replicationSource();
}

QSize Output::enforcedModeSize() const
{
    if (const auto mode = currentMode()) {
        return mode->size();
    } else if (const auto mode = preferredMode()) {
        return mode->size();
    } else if (d->modeList.count() > 0) {
        return d->modeList.first()->size();
    }
    return QSize();
}

QRect Output::geometry() const
{
    QSize size = logicalSize().toSize();
    if (!size.isValid()) {
        return QRect();
    }

    return QRect(d->pos, size);
}

Output::Capabilities Output::capabilities() const
{
    return d->capabilities;
}

void Output::setCapabilities(Capabilities capabilities)
{
    if (d->capabilities != capabilities) {
        d->capabilities = capabilities;
        Q_EMIT capabilitiesChanged();
    }
}

uint32_t Output::overscan() const
{
    return d->overscan;
}

void Output::setOverscan(uint32_t overscan)
{
    if (d->overscan != overscan) {
        d->overscan = overscan;
        Q_EMIT overscanChanged();
    }
}

Output::VrrPolicy Output::vrrPolicy() const
{
    return d->vrrPolicy;
}

void Output::setVrrPolicy(VrrPolicy policy)
{
    if (d->vrrPolicy != policy) {
        d->vrrPolicy = policy;
        Q_EMIT vrrPolicyChanged();
    }
}

Output::RgbRange Output::rgbRange() const
{
    return d->rgbRange;
}

void Output::setRgbRange(Output::RgbRange rgbRange)
{
    if (d->rgbRange != rgbRange) {
        d->rgbRange = rgbRange;
        Q_EMIT rgbRangeChanged();
    }
}

void Output::apply(const OutputPtr &other)
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
    if (!qFuzzyCompare(d->scale, other->d->scale)) {
        changes << &Output::scaleChanged;
        setScale(other->d->scale);
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
        setClones(other->d->clones);
    }
    if (d->replicationSource != other->d->replicationSource) {
        changes << &Output::replicationSourceChanged;
        setReplicationSource(other->d->replicationSource);
    }
    if (!d->compareModeList(d->modeList, other->d->modeList)) {
        changes << &Output::outputChanged;
        changes << &Output::modesChanged;
    }

    setPreferredModes(other->d->preferredModes);
    ModeList modes;
    for (const ModePtr &otherMode : other->modes()) {
        modes.insert(otherMode->id(), otherMode->clone());
    }
    setModes(modes);

    if (d->capabilities != other->d->capabilities) {
        changes << &Output::capabilitiesChanged;
        setCapabilities(other->d->capabilities);
    }
    if (d->vrrPolicy != other->d->vrrPolicy) {
        changes << &Output::vrrPolicyChanged;
        setVrrPolicy(other->d->vrrPolicy);
    }
    if (d->overscan != other->d->overscan) {
        changes << &Output::overscanChanged;
        setOverscan(other->d->overscan);
    }
    if (d->rgbRange != other->d->rgbRange) {
        changes << &Output::rgbRangeChanged;
        setRgbRange(other->d->rgbRange);
    }

    // Non-notifyable changes
    if (other->d->edid) {
        d->edid.reset(other->d->edid->clone());
    }

    blockSignals(keepBlocked);

    while (!changes.isEmpty()) {
        const ChangeSignal &sig = changes.first();
        Q_EMIT(this->*sig)();
        changes.removeAll(sig);
    }
}

QDebug operator<<(QDebug dbg, const KScreen::OutputPtr &output)
{
    QDebugStateSaver saver(dbg);
    if (!output) {
        dbg << "KScreen::Output(NULL)";
        return dbg;
    }

    dbg.nospace() << "KScreen::Output(" << output->id() << ", " << output->name() << ", " << (output->isConnected() ? "connected " : "disconnected ")
                  << (output->isEnabled() ? "enabled" : "disabled") << (output->isPrimary() ? " primary" : "") << ", pos: " << output->pos()
                  << ", res: " << output->size() << ", modeId: " << output->currentModeId() << ", scale: " << output->scale()
                  << ", clone: " << (output->clones().isEmpty() ? "no" : "yes") << ", rotation: " << output->rotation()
                  << ", followPreferredMode: " << output->followPreferredMode() << ")";
    return dbg;
}
