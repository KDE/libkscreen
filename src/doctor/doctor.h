/*
 *  SPDX-FileCopyrightText: 2015 Sebastian Kügler <sebas@kde.org>
 *
 *  SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef KSCREEN_DOCTOR_H
#define KSCREEN_DOCTOR_H

#include "../config.h"
#include <QCommandLineParser>
#include <QObject>

#include "output.h"

namespace KScreen
{
class ConfigOperation;
class DpmsClient;

class Doctor : public QObject
{
    Q_OBJECT

public:
    explicit Doctor(QObject *parent = nullptr);
    ~Doctor() override;

    void setOptionList(const QStringList &positionalArgs);
    void start(QCommandLineParser *m_parser);
    void configReceived(KScreen::ConfigOperation *op);

    void showDpms();

    void showBackends() const;
    void showOutputs() const;
    void showJson() const;
    int outputCount() const;
    void setDpms(const QString &dpmsArg);

    bool setEnabled(int id, bool enabled);
    bool setPosition(int id, const QPoint &pos);
    bool setMode(int id, const QString &mode_id);
    bool setScale(int id, qreal scale);
    bool setRotation(int id, KScreen::Output::Rotation rot);
    bool setOverscan(int id, uint32_t overscan);
    bool setVrrPolicy(int id, KScreen::Output::VrrPolicy policy);
    bool setRgbRange(int id, KScreen::Output::RgbRange rgbRange);

Q_SIGNALS:
    void outputsChanged();
    void started();
    void configChanged();

private:
    // static QString modeString(KWayland::Server::OutputDeviceInterface* outputdevice, int mid);
    void applyConfig();
    void parseOutputArgs();
    KScreen::ConfigPtr m_config;
    QCommandLineParser *m_parser;
    bool m_changed;
    QStringList m_outputArgs;
    DpmsClient *m_dpmsClient;
};

} // namespace

#endif // KSCREEN_WAYLAND_SCREEN_H
