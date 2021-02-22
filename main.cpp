/*
    SPDX-License-Identifier: LGPL-2.1-or-later OR MIT
    SPDX-FileCopyrightText: 2021 Andreas Cord-Landwehr <cordlandwehr@kde.org>
*/

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QCommandLineParser>
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

    QCommandLineParser parser;
    parser.setApplicationDescription("Journald Log Viewer");
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addPositionalArgument("path", "Path to Journald DB");
    parser.process(app);

    const QStringList args = parser.positionalArguments();
    if (args.size() < 1) {
        qCritical() << "Path to DB missing";
        return 1;
    }
    //TODO check if path is reasonable
    const QString path = args.at(0);

    BootModel bootModel(path);

    QQmlApplicationEngine engine;
    const QUrl url(QStringLiteral("qrc:/main.qml"));
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
                     &app, [url](QObject *obj, const QUrl &objUrl) {
        if (!obj && url == objUrl)
            QCoreApplication::exit(-1);
    }, Qt::QueuedConnection);
    engine.rootContext()->setContextProperty("g_bootModel", &bootModel);
    engine.rootContext()->setContextProperty("g_path", path);
    engine.load(url);

    return app.exec();
}
