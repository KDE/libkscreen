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

#include <QtTest>
#include <QObject>
#include <QElapsedTimer>

#include "../src/config.h"
#include "../src/output.h"
#include "../src/mode.h"
#include "../src/edid.h"
#include "../src/getconfigoperation.h"
#include "../src/backendmanager_p.h"

Q_LOGGING_CATEGORY(KSCREEN_QSCREEN, "kscreen.qscreen")

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
    void verifyFeatures();
    void commonUsagePattern();
    void cleanupTestCase();

private:
    QProcess m_process;
    ConfigPtr m_config;
    QString m_backend;
};

void testQScreenBackend::initTestCase()
{
    qputenv("KSCREEN_LOGGING", "false");
    qputenv("KSCREEN_BACKEND", "qscreen");
    qputenv("KSCREEN_BACKEND_INPROCESS", "1");
    KScreen::BackendManager::instance()->shutdownBackend();

    m_backend = QString::fromLocal8Bit(qgetenv("KSCREEN_BACKEND"));

    QElapsedTimer t;
    t.start();
    auto *op = new GetConfigOperation();
    op->exec();
    m_config = op->config();
    const int n = t.nsecsElapsed();
    qDebug() << "Test took: " << n << "ns";
}

void testQScreenBackend::verifyConfig()
{
    QVERIFY(!m_config.isNull());
    if (!m_config) {
        QSKIP("QScreenbackend invalid", SkipAll);
    }
}

void testQScreenBackend::verifyScreen()
{
    ScreenPtr screen = m_config->screen();

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
    foreach (const KScreen::OutputPtr &op, m_config->outputs()) {
        if (op->isPrimary()) {
            primaryFound = true;
        }
    }
    qDebug() << "Primary found? " << primaryFound;
    QVERIFY(primaryFound);
    if (m_backend == QLatin1String("screen")) {
        QCOMPARE(m_config->outputs().count(), QGuiApplication::screens().count());
    }

    const KScreen::OutputPtr primary = m_config->primaryOutput();
    QVERIFY(primary->isEnabled());
    QVERIFY(primary->isConnected());
    //qDebug() << "Primary geometry? " << primary->geometry();
    //qDebug() << " prim modes: " << primary->modes();


    QList<int> ids;
    foreach (const KScreen::OutputPtr &output, m_config->outputs()) {
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

        // Pass, but leave a note, when the x server doesn't report physical size
        if (!output->sizeMm().isValid()) {
            QEXPECT_FAIL("", "The X server doesn't return a sensible physical output size", Continue);
            QVERIFY(output->sizeMm() != QSize());
        }
        QVERIFY(output->edid() != nullptr);
        QCOMPARE(output->rotation(), Output::None);
        QVERIFY(!ids.contains(output->id()));
        ids << output->id();
    }
}

void testQScreenBackend::verifyModes()
{
    const KScreen::OutputPtr primary = m_config->primaryOutput();
    QVERIFY(primary);
    QVERIFY(primary->modes().count() > 0);

    foreach (const KScreen::OutputPtr &output, m_config->outputs()) {
        foreach (const KScreen::ModePtr &mode, output->modes()) {
            qDebug() << "   Mode   : " << mode->name();
            QVERIFY(!mode->name().isEmpty());
            QVERIFY(mode->refreshRate() > 0);
            QVERIFY(mode->size() != QSize());
        }
    }
}

void testQScreenBackend::commonUsagePattern()
{
    auto *op = new GetConfigOperation();
    op->exec();

    const KScreen::OutputList outputs = op->config()->outputs();

    QVariantList outputList;
    Q_FOREACH(const KScreen::OutputPtr &output, outputs) {
        if (!output->isConnected()) {
            continue;
        }

        QVariantMap info;
        info[QStringLiteral("id")] = output->id();
        info[QStringLiteral("primary")] = output->isPrimary();
        info[QStringLiteral("enabled")] = output->isEnabled();
        info[QStringLiteral("rotation")] = output->rotation();

        QVariantMap pos;
        pos[QStringLiteral("x")] = output->pos().x();
        pos[QStringLiteral("y")] = output->pos().y();
        info[QStringLiteral("pos")] = pos;

        if (output->isEnabled()) {
            const KScreen::ModePtr mode = output->currentMode();
            if (!mode) {
                //qWarning() << "CurrentMode is null" << output->name();
                return;
            }

            QVariantMap modeInfo;
            modeInfo[QStringLiteral("refresh")] = mode->refreshRate();

            QVariantMap modeSize;
            modeSize[QStringLiteral("width")] = mode->size().width();
            modeSize[QStringLiteral("height")] = mode->size().height();
            modeInfo[QStringLiteral("size")] = modeSize;

            info[QStringLiteral("mode")] = modeInfo;
        }

        outputList.append(info);
    }
}

void testQScreenBackend::cleanupTestCase()
{
    KScreen::BackendManager::instance()->shutdownBackend();
    qApp->exit(0);
}

void testQScreenBackend::verifyFeatures()
{
    GetConfigOperation *op = new GetConfigOperation();
    op->exec();
    auto config = op->config();
    QVERIFY(config->supportedFeatures().testFlag(Config::Feature::None));
    QVERIFY(!config->supportedFeatures().testFlag(Config::Feature::Writable));
    QVERIFY(!config->supportedFeatures().testFlag(Config::Feature::PrimaryDisplay));
}


QTEST_MAIN(testQScreenBackend)

#include "testqscreenbackend.moc"
