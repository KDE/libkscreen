/*************************************************************************************
 *  Copyright (C) 2012 by Alejandro Fiestas Olivares <afiestas@kde.org>              *
 *  Copyright     2012 by Sebastian Kügler <sebas@kde.org>                           *
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

#define QT_GUI_LIB

#include <QtTest/QtTest>
#include <QtCore/QObject>

#include "../src/config.h"
#include "../src/output.h"
#include "../src/mode.h"
#include "../src/edid.h"

Q_LOGGING_CATEGORY(KSCREEN_QSCREEN, "kscreen.qscreen");

using namespace KScreen;

class testQScreenBackend : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void verifyConfig();
    void verifyScreen();
    void verifyOutputs();
    void verifyModes();
    void commonUsagePattern();
    void cleanupTestCase();

private:
    QProcess m_process;
    Config *m_config;
    QString m_backend;
};

void testQScreenBackend::initTestCase()
{
   setenv("KSCREEN_BACKEND", "qscreen", 1);
//     setenv("KSCREEN_BACKEND", "xrandr", 1);
    m_backend = qgetenv("KSCREEN_BACKEND").constData();

    m_config = Config::current();
}

void testQScreenBackend::verifyConfig()
{
    QVERIFY(m_config != 0);
    if (!m_config) {
        QSKIP("QScreenbackend invalid", SkipAll);
    }
}

void testQScreenBackend::verifyScreen()
{
    Screen *screen = m_config->screen();

    QVERIFY(screen->minSize().width() <= screen->maxSize().width());
    QVERIFY(screen->minSize().height() <= screen->maxSize().height());

    QVERIFY(screen->minSize().width() <= screen->currentSize().width());
    QVERIFY(screen->minSize().height() <= screen->currentSize().height());

    QVERIFY(screen->maxSize().width() >= screen->currentSize().width());
    QVERIFY(screen->maxSize().height() >= screen->currentSize().height());
    QVERIFY(m_config->screen()->maxActiveOutputsCount() > 0);
}


void testQScreenBackend::verifyOutputs()
{

    bool primaryFound = false;
    foreach (const KScreen::Output* op, m_config->outputs()) {
        qDebug() << "CHecking at all";
        if (op->isPrimary()) {
            primaryFound = true;
        }
    }
    qDebug() << "Primary found? " << primaryFound;
    QVERIFY(primaryFound);
    if (m_backend == QStringLiteral("screen")) {
        QCOMPARE(m_config->outputs().count(), QGuiApplication::screens().count());
    }

    KScreen::Output *primary = m_config->primaryOutput();
    qDebug() << "ppp" << primary;
    QVERIFY(primary->isEnabled());
    QVERIFY(primary->isConnected());
    //qDebug() << "Primary geometry? " << primary->geometry();
    qDebug() << " prim modes: " << primary->modes();


    QList<int> ids;
    foreach (auto output, m_config->outputs()) {
        qDebug() << " _____________________ Output: " << output;
        qDebug() << "   output name: " << output->name();
        qDebug() << "   output modes: " << output->modes().count() << output->modes();
        qDebug() << "   output enabled: " << output->isEnabled();
        qDebug() << "   output connect: " << output->isConnected();
        qDebug() << "   output sizeMm : " << output->sizeMm();
        QVERIFY(!output->name().isEmpty());
        QVERIFY(output->id() > -1);
        QVERIFY(output->isConnected());
        QVERIFY(output->isEnabled());
        QVERIFY(output->geometry() != QRectF(1,1,1,1));
        QVERIFY(output->geometry() != QRectF());
        QVERIFY(output->sizeMm() != QSize());
        QVERIFY(output->edid() != 0);
        QCOMPARE(output->rotation(), Output::None);
        QVERIFY(!ids.contains(output->id()));
        ids << output->id();
    }
}

void testQScreenBackend::verifyModes()
{
    KScreen::Output *primary = m_config->primaryOutput();
    QVERIFY(primary);
    QVERIFY(primary->modes().count() > 0);

    foreach (auto output, m_config->outputs()) {
        foreach (auto mode, output->modes()) {
            qDebug() << "   Mode   : " << mode->name();
            QVERIFY(!mode->name().isEmpty());
            QVERIFY(mode->refreshRate() > 0);
            QVERIFY(mode->size() != QSize());
        }
    }
}

void testQScreenBackend::commonUsagePattern()
{
    KScreen::OutputList outputs = KScreen::Config::current()->outputs();

    QVariantList outputList;
    Q_FOREACH(KScreen::Output *output, outputs) {
        if (!output->isConnected()) {
            continue;
        }

        QVariantMap info;

        info["id"] = output->id();
        info["primary"] = output->isPrimary();
        info["enabled"] = output->isEnabled();
        info["rotation"] = output->rotation();

        QVariantMap pos;
        pos["x"] = output->pos().x();
        pos["y"] = output->pos().y();
        info["pos"] = pos;

        if (output->isEnabled()) {
            KScreen::Mode *mode = output->currentMode();
            if (!mode) {
                //qWarning() << "CurrentMode is null" << output->name();
                return;
            }

            QVariantMap modeInfo;
            modeInfo["refresh"] = mode->refreshRate();

            QVariantMap modeSize;
            modeSize["width"] = mode->size().width();
            modeSize["height"] = mode->size().height();
            modeInfo["size"] = modeSize;

            info["mode"] = modeInfo;
        }

        outputList.append(info);
    }
}

void testQScreenBackend::cleanupTestCase()
{
    delete m_config;
    qApp->exit(0);
}



QTEST_MAIN(testQScreenBackend)

#include "testqscreenbackend.moc"
