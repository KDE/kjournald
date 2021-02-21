/*
    SPDX-License-Identifier: LGPL-2.1-or-later OR MIT
    SPDX-FileCopyrightText: 2021 Andreas Cord-Landwehr <cordlandwehr@kde.org>
*/

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <systemd/sd-journal.h>
#include <QDebug>
#include "bootmodel.h"
#include "journaldhelper.h"
#include "journaldviewmodel.h"
#include "journalduniquequerymodel.h"
#include "fieldfilterproxymodel.h"

int main(int argc, char *argv[])
{
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

    QGuiApplication app(argc, argv);

    qmlRegisterType<JournaldViewModel>("systemd", 1, 0, "JournaldViewModel");
    qmlRegisterType<JournaldUniqueQueryModel>("systemd", 1, 0, "JournaldUniqueQueryModel");
    qmlRegisterType<FieldFilterProxyModel>("systemd", 1, 0, "FieldFilterProxyModel");
    qmlRegisterUncreatableType<BootModel>("systemd", 1, 0, "BootModel", "Backend only object");

    BootModel bootModel("/opt/workspace/journald-browser/TESTDATA/journal/");

    QQmlApplicationEngine engine;
    const QUrl url(QStringLiteral("qrc:/main.qml"));
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
                     &app, [url](QObject *obj, const QUrl &objUrl) {
        if (!obj && url == objUrl)
            QCoreApplication::exit(-1);
    }, Qt::QueuedConnection);
    engine.load(url);
    engine.rootContext()->setContextProperty("g_bootModel", &bootModel);

    return app.exec();
}
