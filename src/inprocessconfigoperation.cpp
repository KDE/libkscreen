/*
 * Copyright (C) 2014  Daniel Vratil <dvratil@redhat.com>
 * Copyright 2015 Sebastian Kügler <sebas@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#include "inprocessconfigoperation.h"

#include "abstractbackend.h"
#include "configoperation_p.h"
#include "config.h"
#include "debug_p.h"
#include "output.h"
#include "backendmanager_p.h"
#include "configserializer_p.h"
#include "backendinterface.h"

#include <QX11Info>

#include <memory>

using namespace KScreen;

namespace KScreen
{
void pluginDeleter(QPluginLoader *p)
{
    if (p) {
        qCDebug(KSCREEN) << "Unloading" << p->fileName();
        p->unload();
        delete p;
    }
}

class InProcessConfigOperationPrivate : public ConfigOperationPrivate
{
    Q_OBJECT

public:
    InProcessConfigOperationPrivate(InProcessConfigOperation::Options options, InProcessConfigOperation *qq);

    void loadBackend();
    void loadEdid();

public:
    InProcessConfigOperation::Options options;
    ConfigPtr config;
    KScreen::AbstractBackend* backend;
    QPluginLoader *mLoader;

private:
    Q_DECLARE_PUBLIC(InProcessConfigOperation)
};

}

InProcessConfigOperationPrivate::InProcessConfigOperationPrivate(InProcessConfigOperation::Options options, InProcessConfigOperation* qq)
    : ConfigOperationPrivate(qq)
    , options(options)
{
}

void InProcessConfigOperation::start()
{
    //ConfigOperationPrivate::backendReady(backend);

    Q_D(InProcessConfigOperation);
    d->loadBackend();
}

InProcessConfigOperation::InProcessConfigOperation(Options options, QObject* parent)
    : ConfigOperation(new InProcessConfigOperationPrivate(options, this), parent)
{
}

InProcessConfigOperation::~InProcessConfigOperation()
{
}

KScreen::ConfigPtr InProcessConfigOperation::config() const
{
    Q_D(const InProcessConfigOperation);
    return d->config;
}

void InProcessConfigOperationPrivate::loadBackend()
{
    Q_Q(InProcessConfigOperation);
    qDebug() << "START!";
    QVariantMap arguments;
    const QString &name = qgetenv("KSCREEN_BACKEND").constData();
    auto beargs = QString::fromLocal8Bit(qgetenv("KSCREEN_BACKEND_ARGS"));
    qDebug() << "BEARGS: " << beargs;
    if (beargs.startsWith("TEST_DATA=")) {
         //"TEST_DATA=" = "multipleclone.json");
        arguments["TEST_DATA"] = beargs.remove("TEST_DATA=");
    }
    qCDebug(KSCREEN) << "Requested backend:" << name;
    const QString backendFilter = QString::fromLatin1("KSC_%1*").arg(name);
    const QStringList paths = QCoreApplication::libraryPaths();
    //qCDebug(KSCREEN) << "Lookup paths: " << paths;
    Q_FOREACH (const QString &path, paths) {
        const QDir dir(path + QLatin1String("/kf5/kscreen/"),
                       backendFilter,
                       QDir::SortFlags(QDir::QDir::NoSort),
                       QDir::NoDotAndDotDot | QDir::Files);
        const QFileInfoList finfos = dir.entryInfoList();
        Q_FOREACH (const QFileInfo &finfo, finfos) {
            // Skip "Fake" backend unless explicitly specified via KSCREEN_BACKEND
            if (name.isEmpty() && (finfo.fileName().contains(QLatin1String("KSC_Fake")) || finfo.fileName().contains(QLatin1String("KSC_FakeUI")))) {
                continue;
            }

            // When on X11, skip the QScreen backend, instead use the XRandR backend,
            // if not specified in KSCREEN_BACKEND
            if (name.isEmpty() &&
                finfo.fileName().contains(QLatin1String("KSC_QScreen")) &&
                QX11Info::isPlatformX11()) {
                continue;
            }

            if (name.isEmpty() &&
                finfo.fileName().contains(QLatin1String("KSC_Wayland"))) {
                continue;
            }

            // When not on X11, skip the XRandR backend, and fall back to QScreen
            // if not specified in KSCREEN_BACKEND
            if (name.isEmpty() &&
                finfo.fileName().contains(QLatin1String("KSC_XRandR")) &&
                !QX11Info::isPlatformX11()) {
                continue;
            }

            //qCDebug(KSCREEN) << "Trying" << finfo.filePath();
            // Make sure we unload() and delete the loader whenever it goes out of scope here
            std::unique_ptr<QPluginLoader, void(*)(QPluginLoader *)> loader(new QPluginLoader(finfo.filePath()), pluginDeleter);
            QObject *instance = loader->instance();
            if (!instance) {
                qCDebug(KSCREEN) << loader->errorString();
                continue;
            }

            backend = qobject_cast<KScreen::AbstractBackend*>(instance);
            if (backend) {
                backend->init(arguments);
                if (!backend->isValid()) {
                    qCDebug(KSCREEN) << "Skipping" << backend->name() << "backend";
                    delete backend;
                    continue;
                }

                // This is the only case we don't want to unload() and delete the loader, instead
                // we store it and unload it when the backendloader terminates.
                mLoader = loader.release();
                qCDebug(KSCREEN) << "Loading" << backend->name() << "backend";
                loadEdid();
                //return backend;
            } else {
                qCDebug(KSCREEN) << finfo.fileName() << "does not provide valid KScreen backend";
            }
        }
    }

}

void InProcessConfigOperationPrivate::loadEdid()
{
    Q_Q(InProcessConfigOperation);
    config = backend->config();
    Q_FOREACH (auto output, config->outputs()) {
        const QByteArray edidData = backend->edid(output->id());
        output->setEdid(edidData);
    }


    q->emitResult();

}


#include "inprocessconfigoperation.moc"
