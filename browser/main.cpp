/*
    SPDX-License-Identifier: LGPL-2.1-or-later OR MIT
    SPDX-FileCopyrightText: 2021 Andreas Cord-Landwehr <cordlandwehr@kde.org>
*/

#include "bootmodel.h"
#include "clipboardproxy.h"
#include "fieldfilterproxymodel.h"
#include "journaldhelper.h"
#include "journalduniquequerymodel.h"
#include "journaldviewmodel.h"
#include "sessionconfig.h"
#include <QCommandLineParser>
#include <QDebug>
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <systemd/sd-journal.h>

int main(int argc, char *argv[])
{
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

    QGuiApplication app(argc, argv);
    app.setOrganizationName("KDE");

    qmlRegisterType<JournaldViewModel>("systemd", 1, 0, "JournaldViewModel");
    qmlRegisterType<JournaldUniqueQueryModel>("systemd", 1, 0, "JournaldUniqueQueryModel");
    qmlRegisterType<FieldFilterProxyModel>("systemd", 1, 0, "FieldFilterProxyModel");
    qmlRegisterType<ClipboardProxy>("systemd", 1, 0, "ClipboardProxy");
    qmlRegisterUncreatableType<BootModel>("systemd", 1, 0, "BootModel", "Backend only object");
    qmlRegisterUncreatableType<SessionConfig>("systemd", 1, 0, "SessionConfig", "Backend only object");

    QCommandLineParser parser;
    parser.setApplicationDescription("Journald Log Viewer");
    parser.addHelpOption();
    parser.addVersionOption();
    const QCommandLineOption pathOption("D", "Path to journald database folder", "path");
    parser.addOption(pathOption);
    parser.process(app);

    SessionConfig sessionConfig;
    BootModel bootModel;
    JournaldUniqueQueryModel unitModel;
    unitModel.setField(JournaldHelper::Field::SYSTEMD_UNIT);

    QObject::connect(&sessionConfig, &SessionConfig::modeChanged, &sessionConfig, [&sessionConfig, &bootModel, &unitModel](SessionConfig::Mode mode) {
        switch (mode) {
        case SessionConfig::Mode::SYSTEM:
            bootModel.setSystemJournal();
            unitModel.setSystemJournal();
            break;
        case SessionConfig::Mode::LOCALFOLDER:
            bootModel.setJournaldPath(sessionConfig.localJournalPath());
            unitModel.setJournaldPath(sessionConfig.localJournalPath());
            break;
        }
    });
    QObject::connect(&sessionConfig, &SessionConfig::localJournalPathChanged, &sessionConfig, [&sessionConfig, &bootModel, &unitModel]() {
        bootModel.setJournaldPath(sessionConfig.localJournalPath());
        unitModel.setJournaldPath(sessionConfig.localJournalPath());
    });

    if (parser.isSet(pathOption)) {
        const QString path = parser.value(pathOption);
        bootModel.setJournaldPath(path);
        unitModel.setJournaldPath(path);
        sessionConfig.setLocalJournalPath(path);
        sessionConfig.setMode(SessionConfig::Mode::LOCALFOLDER);
    }

    QQmlApplicationEngine engine;
    const QUrl url(QStringLiteral("qrc:/Main.qml"));
    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreated,
        &app,
        [url](QObject *obj, const QUrl &objUrl) {
            if (!obj && url == objUrl)
                QCoreApplication::exit(-1);
        },
        Qt::QueuedConnection);
    engine.rootContext()->setContextProperty("g_bootModel", &bootModel);
    engine.rootContext()->setContextProperty("g_unitModel", &unitModel);
    engine.rootContext()->setContextProperty("g_config", &sessionConfig);
    engine.load(url);

    return app.exec();
}
