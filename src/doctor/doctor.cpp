/*
 *  SPDX-FileCopyrightText: 2014-2016 Sebastian Kügler <sebas@kde.org>
 *
 *  SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "doctor.h"
#include "dpmsclient.h"

#include <QCommandLineParser>
#include <QCoreApplication>
#include <QDateTime>
#include <QFile>
#include <QGuiApplication>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLoggingCategory>
#include <QRect>
#include <QStandardPaths>

#include "../backendmanager_p.h"
#include "../config.h"
#include "../configoperation.h"
#include "../edid.h"
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
    if (parser->isSet(QStringLiteral("json")) || parser->isSet(QStringLiteral("outputs")) || !m_positionalArgs.isEmpty()) {
        KScreen::GetConfigOperation *op = new KScreen::GetConfigOperation();
        connect(op, &KScreen::GetConfigOperation::finished, this, [this](KScreen::ConfigOperation *op) {
            configReceived(op);
        });
        return;
    }
    if (m_parser->isSet(QStringLiteral("dpms"))) {
        if (!QGuiApplication::platformName().startsWith(QLatin1String("wayland"))) {
            cerr << "DPMS is only supported on Wayland." << endl;
            // We need to kick the event loop, otherwise .quit() hangs
            QTimer::singleShot(0, qApp->quit);
            return;
        }

        m_dpmsClient = new DpmsClient(this);
        if (m_parser->isSet(QStringLiteral("dpms-excluded"))) {
            const auto excludedConnectors = m_parser->values(QStringLiteral("dpms-excluded"));
            m_dpmsClient->setExcludedOutputNames(excludedConnectors);
        }

        connect(m_dpmsClient, &DpmsClient::finished, qApp, &QCoreApplication::quit);

        const QString dpmsArg = m_parser->value(QStringLiteral("dpms"));
        if (dpmsArg == QLatin1String("show")) {
            showDpms();
        } else {
            setDpms(dpmsArg);
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

void KScreen::Doctor::setDpms(const QString &dpmsArg)
{
    qDebug() << "SetDpms: " << dpmsArg;
    connect(m_dpmsClient, &DpmsClient::ready, this, [this, dpmsArg]() {
        cout << "DPMS.ready()";
        if (dpmsArg == QLatin1String("off")) {
            m_dpmsClient->off();
        } else if (dpmsArg == QLatin1String("on")) {
            m_dpmsClient->on();
        } else {
            cout << "--dpms argument not understood (" << dpmsArg << ")";
        }
    });

    m_dpmsClient->connect();
}

void Doctor::showDpms()
{
    m_dpmsClient = new DpmsClient(this);

    connect(m_dpmsClient, &DpmsClient::ready, this, []() {
        cout << "DPMS.ready()";
    });

    m_dpmsClient->connect();
}

void Doctor::showBackends() const
{
    cout << "Environment: " << endl;
    auto env_kscreen_backend = (qgetenv("KSCREEN_BACKEND").isEmpty()) ? QStringLiteral("[not set]") : QString::fromUtf8(qgetenv("KSCREEN_BACKEND"));
    cout << "  * KSCREEN_BACKEND           : " << env_kscreen_backend << endl;
    auto env_kscreen_backend_inprocess =
        (qgetenv("KSCREEN_BACKEND_INPROCESS").isEmpty()) ? QStringLiteral("[not set]") : QString::fromUtf8(qgetenv("KSCREEN_BACKEND_INPROCESS"));
    cout << "  * KSCREEN_BACKEND_INPROCESS : " << env_kscreen_backend_inprocess << endl;
    auto env_kscreen_logging = (qgetenv("KSCREEN_LOGGING").isEmpty()) ? QStringLiteral("[not set]") : QString::fromUtf8(qgetenv("KSCREEN_LOGGING"));
    cout << "  * KSCREEN_LOGGING           : " << env_kscreen_logging << endl;

    cout << "Logging to                : " << (Log::instance()->enabled() ? Log::instance()->logFile() : QStringLiteral("[logging disabled]")) << endl;
    auto backends = BackendManager::instance()->listBackends();
    auto preferred = BackendManager::instance()->preferredBackend();
    cout << "Preferred KScreen backend : " << green << preferred.fileName() << cr << endl;
    cout << "Available KScreen backends:" << endl;
    Q_FOREACH (const QFileInfo f, backends) {
        auto c = blue;
        if (preferred == f) {
            c = green;
        }
        cout << "  * " << c << f.fileName() << cr << ": " << f.absoluteFilePath() << endl;
    }
    cout << endl;
}

void Doctor::setOptionList(const QStringList &positionalArgs)
{
    m_positionalArgs = positionalArgs;
}

void Doctor::parsePositionalArgs()
{
    // qCDebug(KSCREEN_DOCTOR) << "POSARGS" << m_positionalArgs;
    Q_FOREACH (const QString &op, m_positionalArgs) {
        auto ops = op.split(QLatin1Char('.'));
        if (ops.count() > 2) {
            bool ok;
            int output_id = -1;
            if (ops[0] == QLatin1String("output")) {
                Q_FOREACH (const auto &output, m_config->outputs()) {
                    if (output->name() == ops[1]) {
                        output_id = output->id();
                    }
                }
                if (output_id == -1) {
                    output_id = ops[1].toInt(&ok);
                    if (!ok) {
                        cerr << "Unable to parse output id: " << ops[1] << endl;
                        qApp->exit(3);
                        return;
                    }
                }
                if (ops.count() == 3 && ops[2] == QLatin1String("enable")) {
                    if (!setEnabled(output_id, true)) {
                        qApp->exit(1);
                        return;
                    };
                } else if (ops.count() == 3 && ops[2] == QLatin1String("disable")) {
                    if (!setEnabled(output_id, false)) {
                        qApp->exit(1);
                        return;
                    };
                } else if (ops.count() == 4 && ops[2] == QLatin1String("mode")) {
                    QString mode_id = ops[3];
                    // set mode
                    if (!setMode(output_id, mode_id)) {
                        qApp->exit(9);
                        return;
                    }
                    qCDebug(KSCREEN_DOCTOR) << "Output" << output_id << "set mode" << mode_id;

                } else if (ops.count() == 4 && ops[2] == QLatin1String("position")) {
                    QStringList _pos = ops[3].split(QLatin1Char(','));
                    if (_pos.count() != 2) {
                        qCWarning(KSCREEN_DOCTOR) << "Invalid position:" << ops[3];
                        qApp->exit(5);
                        return;
                    }
                    int x = _pos[0].toInt(&ok);
                    int y = _pos[1].toInt(&ok);
                    if (!ok) {
                        cerr << "Unable to parse position: " << ops[3] << endl;
                        qApp->exit(5);
                        return;
                    }

                    QPoint p(x, y);
                    qCDebug(KSCREEN_DOCTOR) << "Output position" << p;
                    if (!setPosition(output_id, p)) {
                        qApp->exit(1);
                        return;
                    }
                } else if ((ops.count() == 4 || ops.count() == 5) && ops[2] == QLatin1String("scale")) {
                    // be lenient about . vs. comma as separator
                    qreal scale = ops[3].replace(QLatin1Char(','), QLatin1Char('.')).toDouble(&ok);
                    if (ops.count() == 5) {
                        const QString dbl = ops[3] + QLatin1String(".") + ops[4];
                        scale = dbl.toDouble(&ok);
                    };
                    // set scale
                    if (!ok || qFuzzyCompare(scale, 0.0) || !setScale(output_id, scale)) {
                        qCDebug(KSCREEN_DOCTOR) << "Could not set scale " << scale << " to output " << output_id;
                        qApp->exit(9);
                        return;
                    }
                } else if ((ops.count() == 4) && (ops[2] == QLatin1String("orientation") || ops[2] == QStringLiteral("rotation"))) {
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
                    if (!ok || !setRotation(output_id, rot)) {
                        qCDebug(KSCREEN_DOCTOR) << "Could not set orientation " << _rotation << " to output " << output_id;
                        qApp->exit(9);
                        return;
                    }
                } else {
                    cerr << "Unable to parse arguments: " << op << endl;
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

    if (m_parser->isSet(QStringLiteral("json"))) {
        showJson();
        qApp->quit();
    }
    if (m_parser->isSet(QStringLiteral("outputs"))) {
        showOutputs();
        qApp->quit();
    }

    parsePositionalArgs();

    if (m_changed) {
        applyConfig();
        m_changed = false;
    }
}

int Doctor::outputCount() const
{
    if (!m_config) {
        qCWarning(KSCREEN_DOCTOR) << "Invalid config.";
        return 0;
    }
    return m_config->outputs().count();
}

void Doctor::showOutputs() const
{
    if (!m_config) {
        qCWarning(KSCREEN_DOCTOR) << "Invalid config.";
        return;
    }

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

    Q_FOREACH (const auto &output, m_config->outputs()) {
        cout << green << "Output: " << cr << output->id() << " " << output->name();
        cout << " " << (output->isEnabled() ? green + QLatin1String("enabled") : red + QLatin1String("disabled"));
        cout << " " << (output->isConnected() ? green + QLatin1String("connected") : red + QLatin1String("disconnected"));
        cout << " " << (output->isPrimary() ? green + QLatin1String("primary") : QString());
        auto _type = typeString[output->type()];
        cout << " " << yellow << (_type.isEmpty() ? QStringLiteral("UnmappedOutputType") : _type);
        cout << blue << " Modes: " << cr;
        Q_FOREACH (auto mode, output->modes()) {
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
        if (output->isPrimary()) {
            cout << blue << "primary";
        }
        cout << cr << endl;
    }
}

void Doctor::showJson() const
{
    QJsonDocument doc(KScreen::ConfigSerializer::serializeConfig(m_config));
    cout << doc.toJson(QJsonDocument::Indented);
}

bool Doctor::setEnabled(int id, bool enabled = true)
{
    if (!m_config) {
        qCWarning(KSCREEN_DOCTOR) << "Invalid config.";
        return false;
    }

    Q_FOREACH (const auto &output, m_config->outputs()) {
        if (output->id() == id) {
            cout << (enabled ? "Enabling " : "Disabling ") << "output " << id << endl;
            output->setEnabled(enabled);
            m_changed = true;
            return true;
        }
    }
    cerr << "Output with id " << id << " not found." << endl;
    qApp->exit(8);
    return false;
}

bool Doctor::setPosition(int id, const QPoint &pos)
{
    if (!m_config) {
        qCWarning(KSCREEN_DOCTOR) << "Invalid config.";
        return false;
    }

    Q_FOREACH (const auto &output, m_config->outputs()) {
        if (output->id() == id) {
            qCDebug(KSCREEN_DOCTOR) << "Set output position" << pos;
            output->setPos(pos);
            m_changed = true;
            return true;
        }
    }
    cout << "Output with id " << id << " not found." << endl;
    return false;
}

bool Doctor::setMode(int id, const QString &mode_id)
{
    if (!m_config) {
        qCWarning(KSCREEN_DOCTOR) << "Invalid config.";
        return false;
    }

    Q_FOREACH (const auto &output, m_config->outputs()) {
        if (output->id() == id) {
            // find mode
            Q_FOREACH (const KScreen::ModePtr mode, output->modes()) {
                auto name =
                    QStringLiteral("%1x%2@%3")
                        .arg(QString::number(mode->size().width()), QString::number(mode->size().height()), QString::number(qRound(mode->refreshRate())));
                if (mode->id() == mode_id || name == mode_id) {
                    qCDebug(KSCREEN_DOCTOR) << "Taddaaa! Found mode" << mode->id() << name;
                    output->setCurrentModeId(mode->id());
                    m_changed = true;
                    return true;
                }
            }
        }
    }
    cout << "Output mode " << mode_id << " not found." << endl;
    return false;
}

bool Doctor::setScale(int id, qreal scale)
{
    if (!m_config) {
        qCWarning(KSCREEN_DOCTOR) << "Invalid config.";
        return false;
    }

    Q_FOREACH (const auto &output, m_config->outputs()) {
        if (output->id() == id) {
            output->setScale(scale);
            m_changed = true;
            return true;
        }
    }
    cout << "Output scale " << id << " invalid." << endl;
    return false;
}

bool Doctor::setRotation(int id, KScreen::Output::Rotation rot)
{
    if (!m_config) {
        qCWarning(KSCREEN_DOCTOR) << "Invalid config.";
        return false;
    }

    Q_FOREACH (const auto &output, m_config->outputs()) {
        if (output->id() == id) {
            output->setRotation(rot);
            m_changed = true;
            return true;
        }
    }
    cout << "Output rotation " << id << " invalid." << endl;
    return false;
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
