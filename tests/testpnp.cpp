/*************************************************************************************
 *  Copyright 2013 Alejandro Fiestas Olivares <afiestas@kde.org>                  *
 *  Copyright 2014 Sebastian Kügler <sebas@kde.org>                                  *
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

#include "testpnp.h"

#include "../src/config.h"
#include "../src/configmonitor.h"
#include "../src/edid.h"
#include "../src/mode.h"
#include "../src/output.h"
#include "../src/screen.h"
#include "../src/getconfigoperation.h"

#include <QGuiApplication>
#include <QRect>
#include <QScreen>
//#include <QX11Info>

using namespace KScreen;

QString typetoString(const Output::Type& type)
{
    switch (type) {
        case Output::Unknown:
            return QStringLiteral("Unknown");
        case Output::Panel:
            return QStringLiteral("Panel (Laptop)");
        case Output::VGA:
            return QStringLiteral("VGA");
        case Output::DVII:
            return QStringLiteral("DVI-I");
        case Output::DVIA:
            return QStringLiteral("DVI-A");
        case Output::DVID:
            return QStringLiteral("DVI-D");
        case Output::HDMI:
            return QStringLiteral("HDMI");
        case Output::TV:
            return QStringLiteral("TV");
        case Output::TVComposite:
            return QStringLiteral("TV-Composite");
        case Output::TVSVideo:
            return QStringLiteral("TV-SVideo");
        case Output::TVComponent:
            return QStringLiteral("TV-Component");
        case Output::TVSCART:
            return QStringLiteral("TV-SCART");
        case Output::TVC4:
            return QStringLiteral("TV-C4");
        case Output::DisplayPort:
            return QStringLiteral("DisplayPort");
        default:
            return QStringLiteral("Invalid Type");

    };
}

TestPnp::TestPnp(bool monitor, QObject *parent)
    : QObject(parent)
    , m_monitor(monitor)
{
    init();
}

TestPnp::~TestPnp()
{
}

void TestPnp::init()
{
    connect(new KScreen::GetConfigOperation(), &KScreen::GetConfigOperation::finished,
            this, &TestPnp::configReady);
}

void TestPnp::configReady(KScreen::ConfigOperation *op)
{
    m_config = qobject_cast<KScreen::GetConfigOperation*>(op)->config();
    if (!m_config) {
        qDebug() << "Config is invalid, probably backend couldn't load";
        qApp->quit();
        return;
    }
    if (!m_config->screen()) {
        qDebug() << "No screen in the configuration, broken backend";
        qApp->quit();
        return;
    }

    print();
    if (m_monitor) {
        ConfigMonitor::instance()->addConfig(m_config);
    } else {
        qApp->quit();
    }
}

void TestPnp::print()
{
    qDebug() << "Screen:";
    qDebug() << "\tmaxSize:" << m_config->screen()->maxSize();
    qDebug() << "\tminSize:" << m_config->screen()->minSize();
    qDebug() << "\tcurrentSize:" << m_config->screen()->currentSize();

    const OutputList outputs = m_config->outputs();
    Q_FOREACH(const OutputPtr &output, outputs) {
        qDebug() << "\n-----------------------------------------------------\n";
        qDebug() << "Id: " << output->id();
        qDebug() << "Name: " << output->name();
        qDebug() << "Type: " << typetoString(output->type());
        qDebug() << "Connected: " << output->isConnected();
        if (!output->isConnected()) {
            continue;
        }
        qDebug() << "Enabled: " << output->isEnabled();
        qDebug() << "Primary: " << output->isPrimary();
        qDebug() << "Rotation: " << output->rotation();
        qDebug() << "Pos: " << output->pos();
        qDebug() << "MMSize: " << output->sizeMm();
        if (output->currentMode()) {
            qDebug() << "Size: " << output->currentMode()->size();
        }
        if (output->clones().isEmpty()) {
            qDebug() << "Clones: " << "None";
        } else {
            qDebug() << "Clones: " << output->clones().count();
        }
        qDebug() << "Mode: " << output->currentModeId();
        qDebug() << "Preferred Mode: " << output->preferredModeId();
        qDebug() << "Preferred modes: " << output->preferredModes();
        qDebug() << "Modes: ";

        const ModeList modes = output->modes();
        Q_FOREACH(const ModePtr &mode, modes) {
            qDebug() << "\t" << mode->id() << "  " << mode->name() << " " << mode->size() << " " << mode->refreshRate();
        }

        const Edid * const edid = output->edid();
        qDebug() << "EDID Info: ";
        if (edid && edid->isValid()) {
            qDebug() << "\tDevice ID: " << edid->deviceId();
            qDebug() << "\tName: " << edid->name();
            qDebug() << "\tVendor: " << edid->vendor();
            qDebug() << "\tSerial: " << edid->serial();
            qDebug() << "\tEISA ID: " << edid->eisaId();
            qDebug() << "\tHash: " << edid->hash();
            qDebug() << "\tWidth: " << edid->width();
            qDebug() << "\tHeight: " << edid->height();
            qDebug() << "\tGamma: " << edid->gamma();
            qDebug() << "\tRed: " << edid->red();
            qDebug() << "\tGreen: " << edid->green();
            qDebug() << "\tBlue: " << edid->blue();
            qDebug() << "\tWhite: " << edid->white();
        } else {
            qDebug() << "\tUnavailable";
        }
    }
}
