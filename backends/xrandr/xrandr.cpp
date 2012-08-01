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

#include "xrandr.h"
#include "config.h"

#include "qrandr/qrandr.h"
#include "qrandr/screen.h"
#include "qrandr/output.h"
#include "qrandr/mode.h"
#include "qrandr/crtc.h"

#include <QtCore/QDebug>
#include <QtCore/QFile>
#include <QtCore/qplugin.h>

Q_EXPORT_PLUGIN2(XRandR, XRandR)

XRandR::XRandR(QObject* parent): QObject(parent)
{
    QRandR::QRandR::self();
}

XRandR::~XRandR()
{

}

QString XRandR::name() const
{
    return QString("XRandR");
}

Config* XRandR::config() const
{
    QRandR::Screen* screen = QRandR::QRandR::self()->screen();
    QHash <RROutput, QRandR::Output* > outputs = screen->outputs();
    qDebug() << outputs.count();

    OutputList outputList;
    Q_FOREACH(QRandR::Output* xOutput, outputs) {
        Output *output = new Output((int) xOutput->id());
        qDebug() << xOutput->name();
        output->setName(xOutput->name());
        output->setConnected(xOutput->isConnected());
        output->setEnabled(xOutput->isEnabled());
        output->setPrimary(xOutput->isPrimary());
        output->setType("unknown");
        if (xOutput->crtc()) {
            output->setPos(xOutput->crtc()->rect().topLeft());
        }

        QHash <RRMode, QRandR::Mode* > xModes = xOutput->modes();

        ModeList modeList;
        Q_FOREACH(QRandR::Mode* xMode, xModes) {
            Mode *mode = new Mode((int)xMode->id());
            mode->setName(xMode->name());
            mode->setRefreshDate(xMode->rate());
            mode->setSize(xMode->size());

            modeList.insert(mode->id(), mode);
        }

        output->setModes(modeList);
        if (xOutput->mode()) {
            output->setCurrentMode((int)xOutput->mode()->id());
        }

        outputList.insert(output->id(), output);

    }

    Config *config = new Config();
    config->setOutputs(outputList);

    return config;
}

bool XRandR::isValid() const
{
    return true;
}

#include "xrandr.moc"