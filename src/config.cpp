/*
 *  SPDX-FileCopyrightText: 2012 Alejandro Fiestas Olivares <afiestas@kde.org>
 *  SPDX-FileCopyrightText: 2014 Daniel Vrátil <dvratil@redhat.com>
 *
 *  SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "config.h"
#include "abstractbackend.h"
#include "backendmanager_p.h"
#include "kscreen_debug.h"
#include "output.h"

#include <QCryptographicHash>
#include <QDebug>
#include <QRect>
#include <QStringList>

using namespace KScreen;

class Q_DECL_HIDDEN Config::Private : public QObject
{
    Q_OBJECT
public:
    Private(Config *parent)
        : QObject(parent)
        , valid(true)
        , supportedFeatures(Config::Feature::None)
        , tabletModeAvailable(false)
        , tabletModeEngaged(false)
        , q(parent)
    {
    }

    KScreen::OutputPtr findPrimaryOutput() const
    {
        auto iter = std::find_if(outputs.constBegin(), outputs.constEnd(), [](const KScreen::OutputPtr &output) -> bool {
            return output->isPrimary();
        });
        return iter == outputs.constEnd() ? KScreen::OutputPtr() : iter.value();
    }

    void onPrimaryOutputChanged()
    {
        const KScreen::OutputPtr output(qobject_cast<KScreen::Output *>(sender()), [](void *) {});
        Q_ASSERT(output);
        if (output->isPrimary()) {
            q->setPrimaryOutput(output);
        } else {
            q->setPrimaryOutput(findPrimaryOutput());
        }
    }

    OutputList::Iterator removeOutput(OutputList::Iterator iter)
    {
        if (iter == outputs.end()) {
            return iter;
        }

        OutputPtr output = iter.value();
        if (!output) {
            return outputs.erase(iter);
        }

        const int outputId = iter.key();
        iter = outputs.erase(iter);

        if (primaryOutput == output) {
            q->setPrimaryOutput(OutputPtr());
        }
        output->disconnect(q);

        Q_EMIT q->outputRemoved(outputId);

        return iter;
    }

    bool valid;
    ScreenPtr screen;
    OutputPtr primaryOutput;
    OutputList outputs;
    Features supportedFeatures;
    bool tabletModeAvailable;
    bool tabletModeEngaged;

private:
    Config *q;
};

bool Config::canBeApplied(const ConfigPtr &config)
{
    return canBeApplied(config, ValidityFlag::None);
}

bool Config::canBeApplied(const ConfigPtr &config, ValidityFlags flags)
{
    if (!config) {
        qCDebug(KSCREEN) << "canBeApplied: Config not available, returning false";
        return false;
    }
    ConfigPtr currentConfig = BackendManager::instance()->config();
    if (!currentConfig) {
        qCDebug(KSCREEN) << "canBeApplied: Current config not available, returning false";
        return false;
    }

    QRect rect;
    OutputPtr currentOutput;
    const OutputList outputs = config->outputs();
    int enabledOutputsCount = 0;
    for (const OutputPtr &output : outputs) {
        if (!output->isEnabled()) {
            continue;
        }

        ++enabledOutputsCount;

        currentOutput = currentConfig->output(output->id());
        // If there is no such output
        if (!currentOutput) {
            qCDebug(KSCREEN) << "canBeApplied: The output:" << output->id() << "does not exists";
            return false;
        }
        // If the output is not connected
        if (!currentOutput->isConnected()) {
            qCDebug(KSCREEN) << "canBeApplied: The output:" << output->id() << "is not connected";
            return false;
        }
        // if there is no currentMode
        if (output->currentModeId().isEmpty()) {
            qCDebug(KSCREEN) << "canBeApplied: The output:" << output->id() << "has no currentModeId";
            return false;
        }
        // If the mode is not found in the current output
        if (!currentOutput->mode(output->currentModeId())) {
            qCDebug(KSCREEN) << "canBeApplied: The output:" << output->id() << "has no mode:" << output->currentModeId();
            return false;
        }

        const ModePtr currentMode = output->currentMode();

        const QSize outputSize = currentMode->size();

        if (output->pos().x() < rect.x()) {
            rect.setX(output->pos().x());
        }

        if (output->pos().y() < rect.y()) {
            rect.setY(output->pos().y());
        }

        QPoint bottomRight;
        if (output->isHorizontal()) {
            bottomRight = QPoint(output->pos().x() + outputSize.width(), output->pos().y() + outputSize.height());
        } else {
            bottomRight = QPoint(output->pos().x() + outputSize.height(), output->pos().y() + outputSize.width());
        }

        if (bottomRight.x() > rect.width()) {
            rect.setWidth(bottomRight.x());
        }

        if (bottomRight.y() > rect.height()) {
            rect.setHeight(bottomRight.y());
        }
    }

    if (flags & ValidityFlag::RequireAtLeastOneEnabledScreen && enabledOutputsCount == 0) {
        qCDebug(KSCREEN) << "canBeAppled: There are no enabled screens, at least one required";
        return false;
    }

    const int maxEnabledOutputsCount = config->screen()->maxActiveOutputsCount();
    if (enabledOutputsCount > maxEnabledOutputsCount) {
        qCDebug(KSCREEN) << "canBeApplied: Too many active screens. Requested: " << enabledOutputsCount << ", Max: " << maxEnabledOutputsCount;
        return false;
    }

    if (rect.width() > config->screen()->maxSize().width()) {
        qCDebug(KSCREEN) << "canBeApplied: The configuration is too wide:" << rect.width();
        return false;
    }
    if (rect.height() > config->screen()->maxSize().height()) {
        qCDebug(KSCREEN) << "canBeApplied: The configuration is too high:" << rect.height();
        return false;
    }

    return true;
}

Config::Config()
    : QObject(nullptr)
    , d(new Private(this))
{
}

Config::~Config()
{
    delete d;
}

ConfigPtr Config::clone() const
{
    ConfigPtr newConfig(new Config());
    newConfig->d->screen = d->screen->clone();
    for (const OutputPtr &ourOutput : d->outputs) {
        newConfig->addOutput(ourOutput->clone());
    }
    newConfig->d->primaryOutput = newConfig->d->findPrimaryOutput();
    newConfig->setSupportedFeatures(supportedFeatures());
    newConfig->setTabletModeAvailable(tabletModeAvailable());
    newConfig->setTabletModeEngaged(tabletModeEngaged());
    return newConfig;
}

QString Config::connectedOutputsHash() const
{
    QStringList hashedOutputs;

    const auto outputs = connectedOutputs();
    hashedOutputs.reserve(outputs.count());
    for (const OutputPtr &output : outputs) {
        hashedOutputs << output->hash();
    }
    std::sort(hashedOutputs.begin(), hashedOutputs.end());
    const auto hash = QCryptographicHash::hash(hashedOutputs.join(QString()).toLatin1(), QCryptographicHash::Md5);
    return QString::fromLatin1(hash.toHex());
}

ScreenPtr Config::screen() const
{
    return d->screen;
}

void Config::setScreen(const ScreenPtr &screen)
{
    d->screen = screen;
}

OutputPtr Config::output(int outputId) const
{
    return d->outputs.value(outputId);
}

Config::Features Config::supportedFeatures() const
{
    return d->supportedFeatures;
}

void Config::setSupportedFeatures(const Config::Features &features)
{
    d->supportedFeatures = features;
}

bool Config::tabletModeAvailable() const
{
    return d->tabletModeAvailable;
}

void Config::setTabletModeAvailable(bool available)
{
    d->tabletModeAvailable = available;
}

bool Config::tabletModeEngaged() const
{
    return d->tabletModeEngaged;
}

void Config::setTabletModeEngaged(bool engaged)
{
    d->tabletModeEngaged = engaged;
}

OutputList Config::outputs() const
{
    return d->outputs;
}

OutputList Config::connectedOutputs() const
{
    OutputList outputs;
    for (const OutputPtr &output : qAsConst(d->outputs)) {
        if (!output->isConnected()) {
            continue;
        }
        outputs.insert(output->id(), output);
    }

    return outputs;
}

OutputPtr Config::primaryOutput() const
{
    if (d->primaryOutput) {
        return d->primaryOutput;
    }

    d->primaryOutput = d->findPrimaryOutput();
    return d->primaryOutput;
}

void Config::setPrimaryOutput(const OutputPtr &newPrimary)
{
    // Don't call primaryOutput(): at this point d->primaryOutput is either
    // initialized, or we need to look for the primary anyway
    if (d->primaryOutput == newPrimary) {
        return;
    }

    // qCDebug(KSCREEN) << "Primary output changed from" << primaryOutput()
    //                  << "(" << (primaryOutput().isNull() ? "none" : primaryOutput()->name()) << ") to"
    //                  << newPrimary << "(" << (newPrimary.isNull() ? "none" : newPrimary->name()) << ")";

    for (OutputPtr &output : d->outputs) {
        disconnect(output.data(), &KScreen::Output::isPrimaryChanged, d, &KScreen::Config::Private::onPrimaryOutputChanged);
        output->setPrimary(output == newPrimary);
        connect(output.data(), &KScreen::Output::isPrimaryChanged, d, &KScreen::Config::Private::onPrimaryOutputChanged);
    }

    d->primaryOutput = newPrimary;
    Q_EMIT primaryOutputChanged(newPrimary);
}

void Config::addOutput(const OutputPtr &output)
{
    d->outputs.insert(output->id(), output);
    connect(output.data(), &KScreen::Output::isPrimaryChanged, d, &KScreen::Config::Private::onPrimaryOutputChanged);

    Q_EMIT outputAdded(output);

    if (output->isPrimary()) {
        setPrimaryOutput(output);
    }
}

void Config::removeOutput(int outputId)
{
    d->removeOutput(d->outputs.find(outputId));
}

void Config::setOutputs(const OutputList &outputs)
{
    for (auto iter = d->outputs.begin(), end = d->outputs.end(); iter != end;) {
        iter = d->removeOutput(iter);
        end = d->outputs.end();
    }

    for (const OutputPtr &output : outputs) {
        addOutput(output);
    }
}

bool Config::isValid() const
{
    return d->valid;
}

void Config::setValid(bool valid)
{
    d->valid = valid;
}

void Config::apply(const ConfigPtr &other)
{
    d->screen->apply(other->screen());

    setTabletModeAvailable(other->tabletModeAvailable());
    setTabletModeEngaged(other->tabletModeEngaged());

    // Remove removed outputs
    for (auto it = d->outputs.begin(); it != d->outputs.end();) {
        if (!other->d->outputs.contains((*it)->id())) {
            it = d->removeOutput(it);
        } else {
            ++it;
        }
    }

    for (const OutputPtr &otherOutput : qAsConst(other->d->outputs)) {
        // Add new outputs
        if (!d->outputs.contains(otherOutput->id())) {
            addOutput(otherOutput->clone());
        } else {
            // Update existing outputs
            d->outputs[otherOutput->id()]->apply(otherOutput);
        }
    }

    // Update validity
    setValid(other->isValid());
}

QDebug operator<<(QDebug dbg, const KScreen::ConfigPtr &config)
{
    if (config) {
        dbg << "KScreen::Config(";
        const auto outputs = config->outputs();
        for (const auto &output : outputs) {
            if (output->isConnected()) {
                dbg << Qt::endl << output;
            }
        }
        dbg << ")";
    } else {
        dbg << "KScreen::Config(NULL)";
    }
    return dbg;
}

#include "config.moc"
