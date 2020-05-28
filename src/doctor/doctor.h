/*************************************************************************************
 *  Copyright 2015 Sebastian Kügler <sebas@kde.org>                                  *
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

#ifndef KSCREEN_DOCTOR_H
#define KSCREEN_DOCTOR_H

#include <QCommandLineParser>
#include <QObject>
#include "../config.h"

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

Q_SIGNALS:
    void outputsChanged();
    void started();
    void configChanged();

private:
    //static QString modeString(KWayland::Server::OutputDeviceInterface* outputdevice, int mid);
    void applyConfig();
    void parsePositionalArgs();
    int parseInt(const QString &str, bool &ok) const;
    KScreen::ConfigPtr m_config;
    QCommandLineParser* m_parser;
    bool m_changed;
    QStringList m_positionalArgs;
    DpmsClient *m_dpmsClient;
};

} // namespace

#endif // KSCREEN_WAYLAND_SCREEN_H
