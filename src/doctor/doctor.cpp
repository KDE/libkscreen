/*
 *  SPDX-FileCopyrightText: 2014-2016 Sebastian Kügler <sebas@kde.org>
 *
 *  SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "doctor.h"
#include "mode.h"
#include <dpms.h>

#include <QCollator>
#include <QCoreApplication>
#include <QDateTime>
#include <QFile>
#include <QGuiApplication>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLoggingCategory>
#include <QRect>
#include <QScreen>
#include <QStandardPaths>

#include <utility>

#include "../backendmanager_p.h"
#include "../config.h"
#include "../configoperation.h"
#include "../getconfigoperation.h"
#include "../log.h"
#include "../output.h"
#include "../setconfigoperation.h"

Q_LOGGING_CATEGORY(KSCREEN_DOCTOR, "kscreen.doctor")

static QTextStream cout(stdout);
static QTextStream cerr(stderr);

const static QString green = QStringLiteral("\033[01;32m");
const static QString red = QStringLiteral("\033[01;31m");
const static QString yellow = QStringLiteral("\033[01;33m");
const static QString blue = QStringLiteral("\033[01;34m");
const static QString bold = QStringLiteral("\033[01;39m");
const static QString cr = QStringLiteral("\033[0;0m");

namespace KScreen
{
namespace ConfigSerializer
{
// Exported private symbol in configserializer_p.h in KScreen
extern QJsonObject serializeConfig(const KScreen::ConfigPtr &config);
}
}

using namespace KScreen;

Doctor::Doctor(QObject *parent)
    : QObject(parent)
    , m_config(nullptr)
    , m_changed(false)
    , m_dpmsClient(nullptr)
{
}

Doctor::~Doctor()
{
}

void Doctor::start(QCommandLineParser *parser)
{
    m_parser = parser;
    if (m_parser->isSet(QStringLiteral("info"))) {
        showBackends();
    }
    if (parser->isSet(QStringLiteral("json")) || parser->isSet(QStringLiteral("outputs")) || !m_outputArgs.isEmpty()) {
        KScreen::GetConfigOperation *op = new KScreen::GetConfigOperation();
        connect(op, &KScreen::GetConfigOperation::finished, this, [this](KScreen::ConfigOperation *op) {
            configReceived(op);
        });
        return;
    }
    if (m_parser->isSet(QStringLiteral("dpms"))) {
        if (!QGuiApplication::platformName().startsWith(QLatin1String("wayland"))) {
            cerr << "DPMS is only supported on Wayland." << Qt::endl;
            // We need to kick the event loop, otherwise .quit() hangs
            QTimer::singleShot(0, qApp->quit);
            return;
        }

        m_dpmsClient = new Dpms(this);
        auto screens = qGuiApp->screens();
        if (m_parser->isSet(QStringLiteral("dpms-excluded"))) {
            const auto excludedConnectors = m_parser->values(QStringLiteral("dpms-excluded"));
            auto it = std::remove_if(screens.begin(), screens.end(), [&excludedConnectors](QScreen *screen) {
                return excludedConnectors.contains(screen->name());
            });
            screens.erase(it, screens.end());
        }

        connect(m_dpmsClient, &Dpms::hasPendingChangesChanged, qGuiApp, [](bool hasChanges) {
            if (!hasChanges) {
                qGuiApp->quit();
            }
        });

        const QString dpmsArg = m_parser->value(QStringLiteral("dpms"));
        if (dpmsArg == QLatin1String("show")) {
        } else {
            auto performSwitch = [this, dpmsArg, screens](bool supported) {
                if (!supported) {
                    cerr << "DPMS not supported in this system";
                    qGuiApp->quit();
                    return;
                }

                if (dpmsArg == QLatin1String("off")) {
                    m_dpmsClient->switchMode(KScreen::Dpms::Off, screens);
                } else if (dpmsArg == QLatin1String("on")) {
                    m_dpmsClient->switchMode(KScreen::Dpms::On, screens);
                } else {
                    cerr << "--dpms argument not understood (" << dpmsArg << ")";
                }
            };
            if (m_dpmsClient->isSupported()) {
                performSwitch(m_dpmsClient->isSupported());
            } else {
                connect(m_dpmsClient, &Dpms::supportedChanged, this, performSwitch);
            }
        }
        return;
    }

    if (m_parser->isSet(QStringLiteral("log"))) {
        const QString logmsg = m_parser->value(QStringLiteral("log"));
        if (!Log::instance()->enabled()) {
            qCWarning(KSCREEN_DOCTOR) << "Logging is disabled, unset KSCREEN_LOGGING in your environment.";
        } else {
            Log::log(logmsg);
        }
    }
    // We need to kick the event loop, otherwise .quit() hangs
    QTimer::singleShot(0, qApp->quit);
}

void Doctor::showBackends() const
{
    cout << "Environment: " << Qt::endl;
    auto env_kscreen_backend = qEnvironmentVariable("KSCREEN_BACKEND", QStringLiteral("[not set]"));
    cout << "  * KSCREEN_BACKEND           : " << env_kscreen_backend << Qt::endl;
    auto env_kscreen_backend_inprocess = qEnvironmentVariable("KSCREEN_BACKEND_INPROCESS", QStringLiteral("[not set]"));
    cout << "  * KSCREEN_BACKEND_INPROCESS : " << env_kscreen_backend_inprocess << Qt::endl;
    auto env_kscreen_logging = qEnvironmentVariable("KSCREEN_LOGGING", QStringLiteral("[not set]"));
    cout << "  * KSCREEN_LOGGING           : " << env_kscreen_logging << Qt::endl;

    cout << "Logging to                : " << (Log::instance()->enabled() ? Log::instance()->logFile() : QStringLiteral("[logging disabled]")) << Qt::endl;
    const auto backends = BackendManager::instance()->listBackends();
    auto preferred = BackendManager::instance()->preferredBackend();
    cout << "Preferred KScreen backend : " << green << preferred.fileName() << cr << Qt::endl;
    cout << "Available KScreen backends:" << Qt::endl;
    for (const QFileInfo &f : backends) {
        auto c = blue;
        if (preferred == f) {
            c = green;
        }
        cout << "  * " << c << f.fileName() << cr << ": " << f.absoluteFilePath() << Qt::endl;
    }
    cout << Qt::endl;
}

void Doctor::setOptionList(const QStringList &outputArgs)
{
    m_outputArgs = outputArgs;
}

OutputPtr Doctor::findOutput(const QString &query)
{
    // try as an output name or ID
    for (const auto &output : m_config->outputs()) {
        if (output->name() == query) {
            return output;
        }
    }
    bool ok;
    int id = query.toInt(&ok);
    if (!ok) {
        cerr << "Output with name " << query << " not found." << Qt::endl;
        return OutputPtr();
    }

    if (m_config->outputs().contains(id)) {
        return m_config->outputs()[id];
    } else {
        cerr << "Output with id " << id << " not found." << Qt::endl;
        return OutputPtr();
    }
}

void Doctor::parseOutputArgs()
{
    // qCDebug(KSCREEN_DOCTOR) << "POSARGS" << m_positionalArgs;
    for (const QString &op : std::as_const(m_outputArgs)) {
        auto ops = op.split(QLatin1Char('.'));
        if (ops.count() > 2) {
            bool ok;
            if (ops[0] == QLatin1String("output")) {
                OutputPtr output = findOutput(ops[1]);
                if (!output) {
                    qApp->exit(3);
                    return;
                }
                int output_id = output->id();

                const QString subcmd = ops.length() > 2 ? ops[2] : QString();

                if (ops.count() == 3 && subcmd == QLatin1String("primary")) {
                    setPrimary(output);
                } else if (ops.count() == 4 && subcmd == QLatin1String("priority")) {
                    uint32_t priority = ops[3].toUInt(&ok);
                    if (!ok || priority > 100) {
                        qCWarning(KSCREEN_DOCTOR) << "Wrong input: allowed values for priority are from 1 to 100";
                        qApp->exit(5);
                        return;
                    }
                    setPriority(output, priority);
                } else if (ops.count() == 3 && subcmd == QLatin1String("enable")) {
                    setEnabled(output, true);
                } else if (ops.count() == 3 && subcmd == QLatin1String("disable")) {
                    setEnabled(output, false);
                } else if (ops.count() == 4 && subcmd == QLatin1String("mode")) {
                    QString mode_id = ops[3];
                    // set mode
                    if (!setMode(output, mode_id)) {
                        qApp->exit(9);
                        return;
                    }
                    qCDebug(KSCREEN_DOCTOR) << "Output" << output_id << "set mode" << mode_id;

                } else if (ops.count() == 4 && subcmd == QLatin1String("position")) {
                    QStringList _pos = ops[3].split(QLatin1Char(','));
                    if (_pos.count() != 2) {
                        qCWarning(KSCREEN_DOCTOR) << "Invalid position:" << ops[3];
                        qApp->exit(5);
                        return;
                    }
                    int x = _pos[0].toInt(&ok);
                    int y = _pos[1].toInt(&ok);
                    if (!ok) {
                        cerr << "Unable to parse position: " << ops[3] << Qt::endl;
                        qApp->exit(5);
                        return;
                    }

                    QPoint p(x, y);
                    qCDebug(KSCREEN_DOCTOR) << "Output position" << p;
                    setPosition(output, p);

                } else if ((ops.count() == 4 || ops.count() == 5) && subcmd == QLatin1String("scale")) {
                    // be lenient about . vs. comma as separator
                    qreal scale = ops[3].replace(QLatin1Char(','), QLatin1Char('.')).toDouble(&ok);
                    if (ops.count() == 5) {
                        const QString dbl = ops[3] + QStringLiteral(".") + ops[4];
                        scale = dbl.toDouble(&ok);
                    };
                    // set scale
                    if (!ok || qFuzzyCompare(scale, 0.0)) {
                        qCDebug(KSCREEN_DOCTOR) << "Could not set scale " << scale << " to output " << output_id;
                        qApp->exit(9);
                        return;
                    }
                    setScale(output, scale);
                } else if ((ops.count() == 4) && (subcmd == QLatin1String("orientation") || subcmd == QStringLiteral("rotation"))) {
                    const QString _rotation = ops[3].toLower();
                    bool ok = false;
                    const QHash<QString, KScreen::Output::Rotation> rotationMap({{QStringLiteral("none"), KScreen::Output::None},
                                                                                 {QStringLiteral("normal"), KScreen::Output::None},
                                                                                 {QStringLiteral("left"), KScreen::Output::Left},
                                                                                 {QStringLiteral("right"), KScreen::Output::Right},
                                                                                 {QStringLiteral("inverted"), KScreen::Output::Inverted}});
                    KScreen::Output::Rotation rot = KScreen::Output::None;
                    // set orientation
                    if (rotationMap.contains(_rotation)) {
                        ok = true;
                        rot = rotationMap[_rotation];
                    }
                    if (!ok) {
                        qCDebug(KSCREEN_DOCTOR) << "Could not set orientation " << _rotation << " to output " << output_id;
                        qApp->exit(9);
                        return;
                    }
                    setRotation(output, rot);
                } else if (ops.count() == 4 && subcmd == QLatin1String("overscan")) {
                    const uint32_t overscan = ops[3].toInt();
                    if (overscan > 100) {
                        qCWarning(KSCREEN_DOCTOR) << "Wrong input: allowed values for overscan are from 0 to 100";
                        qApp->exit(9);
                        return;
                    }
                    setOverscan(output, overscan);
                } else if (ops.count() == 4 && subcmd == QLatin1String("vrrpolicy")) {
                    const QString _policy = ops[3].toLower();
                    KScreen::Output::VrrPolicy policy;
                    if (_policy == QStringLiteral("never")) {
                        policy = KScreen::Output::VrrPolicy::Never;
                    } else if (_policy == QStringLiteral("always")) {
                        policy = KScreen::Output::VrrPolicy::Always;
                    } else if (_policy == QStringLiteral("automatic")) {
                        policy = KScreen::Output::VrrPolicy::Automatic;
                    } else {
                        qCDebug(KSCREEN_DOCTOR) << "Wrong input: Only allowed values are \"never\", \"always\" and \"automatic\"";
                        qApp->exit(9);
                        return;
                    }
                    setVrrPolicy(output, policy);
                } else if (ops.count() == 4 && subcmd == QLatin1String("rgbrange")) {
                    const QString _range = ops[3].toLower();
                    KScreen::Output::RgbRange range;
                    if (_range == QStringLiteral("automatic")) {
                        range = KScreen::Output::RgbRange::Automatic;
                    } else if (_range == QStringLiteral("full")) {
                        range = KScreen::Output::RgbRange::Full;
                    } else if (_range == QStringLiteral("limited")) {
                        range = KScreen::Output::RgbRange::Limited;
                    } else {
                        qCDebug(KSCREEN_DOCTOR) << "Wrong input: Only allowed values for rgbrange are \"automatic\", \"full\" and \"limited\"";
                        qApp->exit(9);
                        return;
                    }
                    setRgbRange(output, range);
                } else {
                    cerr << "Unable to parse arguments: " << op << Qt::endl;
                    qApp->exit(2);
                    return;
                }
            }
        }
    }
}

void Doctor::configReceived(KScreen::ConfigOperation *op)
{
    m_config = op->config();

    if (!m_config) {
        qCWarning(KSCREEN_DOCTOR) << "Invalid config.";
        return;
    }

    if (m_parser->isSet(QStringLiteral("json"))) {
        showJson();
        qApp->quit();
    }
    if (m_parser->isSet(QStringLiteral("outputs"))) {
        showOutputs();
        qApp->quit();
    }

    parseOutputArgs();

    if (m_changed) {
        applyConfig();
        m_changed = false;
    }
}

void Doctor::showOutputs() const
{
    QHash<KScreen::Output::Type, QString> typeString;
    typeString[KScreen::Output::Unknown] = QStringLiteral("Unknown");
    typeString[KScreen::Output::VGA] = QStringLiteral("VGA");
    typeString[KScreen::Output::DVI] = QStringLiteral("DVI");
    typeString[KScreen::Output::DVII] = QStringLiteral("DVII");
    typeString[KScreen::Output::DVIA] = QStringLiteral("DVIA");
    typeString[KScreen::Output::DVID] = QStringLiteral("DVID");
    typeString[KScreen::Output::HDMI] = QStringLiteral("HDMI");
    typeString[KScreen::Output::Panel] = QStringLiteral("Panel");
    typeString[KScreen::Output::TV] = QStringLiteral("TV");
    typeString[KScreen::Output::TVComposite] = QStringLiteral("TVComposite");
    typeString[KScreen::Output::TVSVideo] = QStringLiteral("TVSVideo");
    typeString[KScreen::Output::TVComponent] = QStringLiteral("TVComponent");
    typeString[KScreen::Output::TVSCART] = QStringLiteral("TVSCART");
    typeString[KScreen::Output::TVC4] = QStringLiteral("TVC4");
    typeString[KScreen::Output::DisplayPort] = QStringLiteral("DisplayPort");

    QCollator collator;
    collator.setNumericMode(true);

    for (const auto &output : m_config->outputs()) {
        cout << green << "Output: " << cr << output->id() << " " << output->name();
        cout << " " << (output->isEnabled() ? green + QStringLiteral("enabled") : red + QStringLiteral("disabled")) << cr;
        cout << " " << (output->isConnected() ? green + QStringLiteral("connected") : red + QStringLiteral("disconnected")) << cr;
        cout << " " << (output->isEnabled() ? green : red) + QStringLiteral("priority ") << output->priority() << cr;
        auto _type = typeString[output->type()];
        cout << " " << yellow << (_type.isEmpty() ? QStringLiteral("UnmappedOutputType") : _type);
        cout << blue << " Modes: " << cr;

        const auto modes = output->modes();
        auto modeKeys = modes.keys();
        std::sort(modeKeys.begin(), modeKeys.end(), collator);

        for (const auto &key : modeKeys) {
            auto mode = *modes.find(key);

            auto name = QStringLiteral("%1x%2@%3")
                            .arg(QString::number(mode->size().width()), QString::number(mode->size().height()), QString::number(qRound(mode->refreshRate())));
            if (mode == output->currentMode()) {
                name = green + name + QLatin1Char('*') + cr;
            }
            if (mode == output->preferredMode()) {
                name = name + QLatin1Char('!');
            }
            cout << mode->id() << ":" << name << " ";
        }
        const auto g = output->geometry();
        cout << yellow << "Geometry: " << cr << g.x() << "," << g.y() << " " << g.width() << "x" << g.height() << " ";
        cout << yellow << "Scale: " << cr << output->scale() << " ";
        cout << yellow << "Rotation: " << cr << output->rotation() << " ";
        cout << yellow << "Overscan: " << cr << output->overscan() << " ";
        cout << yellow << "Vrr: ";
        if (output->capabilities() & Output::Capability::Vrr) {
            switch (output->vrrPolicy()) {
            case Output::VrrPolicy::Never:
                cout << cr << "Never ";
                break;
            case Output::VrrPolicy::Automatic:
                cout << cr << "Automatic ";
                break;
            case Output::VrrPolicy::Always:
                cout << cr << "Always ";
            }
        } else {
            cout << cr << "incapable ";
        }
        cout << yellow << "RgbRange: ";
        if (output->capabilities() & Output::Capability::RgbRange) {
            switch (output->rgbRange()) {
            case Output::RgbRange::Automatic:
                cout << cr << "Automatic";
                break;
            case Output::RgbRange::Full:
                cout << cr << "Full";
                break;
            case Output::RgbRange::Limited:
                cout << cr << "Limited";
            }
        } else {
            cout << cr << "unknown";
        }
        cout << cr << Qt::endl;
    }
}

void Doctor::showJson() const
{
    QJsonDocument doc(KScreen::ConfigSerializer::serializeConfig(m_config));
    cout << doc.toJson(QJsonDocument::Indented);
}

void Doctor::setEnabled(OutputPtr output, bool enable)
{
    cout << (enable ? "Enabling " : "Disabling ") << "output " << output->id() << Qt::endl;
    output->setEnabled(enable);
    m_changed = true;
}

void Doctor::setPosition(OutputPtr output, const QPoint &pos)
{
    qCDebug(KSCREEN_DOCTOR) << "Set output position" << pos;
    output->setPos(pos);
    m_changed = true;
}

KScreen::ModePtr Doctor::findMode(OutputPtr output, const QString &query)
{
    for (const KScreen::ModePtr &mode : output->modes()) {
        auto name = QStringLiteral("%1x%2@%3")
                        .arg(QString::number(mode->size().width()), QString::number(mode->size().height()), QString::number(qRound(mode->refreshRate())));
        if (mode->id() == query || name == query) {
            qCDebug(KSCREEN_DOCTOR) << "Taddaaa! Found mode" << mode->id() << name;
            return mode;
        }
    }
    cout << "Output mode " << query << " not found." << Qt::endl;
    return ModePtr();
}

bool Doctor::setMode(OutputPtr output, const QString &query)
{
    // find mode
    const KScreen::ModePtr mode = findMode(output, query);
    if (!mode) {
        return false;
    }
    output->setCurrentModeId(mode->id());
    m_changed = true;
    return true;
}

void Doctor::setScale(OutputPtr output, qreal scale)
{
    output->setScale(scale);
    m_changed = true;
}

void Doctor::setRotation(OutputPtr output, KScreen::Output::Rotation rot)
{
    output->setRotation(rot);
    m_changed = true;
}

void Doctor::setOverscan(OutputPtr output, uint32_t overscan)
{
    output->setOverscan(overscan);
    m_changed = true;
}

void Doctor::setVrrPolicy(OutputPtr output, KScreen::Output::VrrPolicy policy)
{
    output->setVrrPolicy(policy);
    m_changed = true;
}

void Doctor::setRgbRange(OutputPtr output, KScreen::Output::RgbRange rgbRange)
{
    output->setRgbRange(rgbRange);
    m_changed = true;
}

void KScreen::Doctor::setPrimary(OutputPtr output)
{
    setPriority(output, 1);
}

void KScreen::Doctor::setPriority(OutputPtr output, uint32_t priority)
{
    m_config->setOutputPriority(output, priority);
    m_changed = true;
}

void Doctor::applyConfig()
{
    if (!m_changed) {
        return;
    }
    auto setop = new SetConfigOperation(m_config, this);
    setop->exec();
    qCDebug(KSCREEN_DOCTOR) << "setop exec returned" << m_config;
    qApp->exit(0);
}
