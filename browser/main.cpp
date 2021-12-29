/*
    SPDX-License-Identifier: LGPL-2.1-or-later OR MIT
    SPDX-FileCopyrightText: 2021 Andreas Cord-Landwehr <cordlandwehr@kde.org>
*/

#include "bootmodel.h"
#include "clipboardproxy.h"
#include "fieldfilterproxymodel.h"
#include "filtercriteriamodel.h"
#include "flattenedfiltercriteriaproxymodel.h"
#include "journaldhelper.h"
#include "journalduniquequerymodel.h"
#include "journaldviewmodel.h"
#include "sessionconfig.h"
#include <KLocalizedString>
#include <QApplication>
#include <QCommandLineParser>
#include <QDebug>
#include <QIcon>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickStyle>
#include <QSortFilterProxyModel>
#include <systemd/sd-journal.h>

int main(int argc, char *argv[])
{
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication app(argc, argv);
    QGuiApplication::setWindowIcon(QIcon::fromTheme(QStringLiteral("utilities-log-viewer")));
    app.setOrganizationName("KDE");
    KLocalizedString::setApplicationDomain("kjournald");

    // use org.kde.desktop style unless another style is forced
    if (qEnvironmentVariableIsEmpty("QT_QUICK_CONTROLS_STYLE")) {
        QQuickStyle::setStyle(QStringLiteral("org.kde.desktop"));
    }

    qmlRegisterType<JournaldViewModel>("kjournald", 1, 0, "JournaldViewModel");
    qmlRegisterType<JournaldUniqueQueryModel>("kjournald", 1, 0, "JournaldUniqueQueryModel");
    qmlRegisterType<FieldFilterProxyModel>("kjournald", 1, 0, "FieldFilterProxyModel");
    qmlRegisterType<ClipboardProxy>("kjournald", 1, 0, "ClipboardProxy");
    qmlRegisterType<FlattenedFilterCriteriaProxyModel>("kjournald", 1, 0, "FlattenedFilterCriteriaProxyModel");
    qmlRegisterUncreatableType<FilterCriteriaModel>("kjournald", 1, 0, "FilterCriteriaModel", "Backend only object");
    qmlRegisterUncreatableType<BootModel>("kjournald", 1, 0, "BootModel", "Backend only object");
    qmlRegisterUncreatableType<SessionConfig>("kjournald", 1, 0, "SessionConfig", "Backend only object");

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
    QSortFilterProxyModel unitSortProxyModel;
    unitSortProxyModel.setSourceModel(&unitModel);
    unitSortProxyModel.setSortCaseSensitivity(Qt::CaseInsensitive);
    unitSortProxyModel.setSortRole(JournaldUniqueQueryModel::FIELD);
    QObject::connect(&unitModel, &JournaldUniqueQueryModel::modelReset, [&unitSortProxyModel]() {
        unitSortProxyModel.sort(0);
    });
    unitSortProxyModel.sort(0);

    JournaldUniqueQueryModel executableModel;
    executableModel.setField(JournaldHelper::Field::EXE);
    QSortFilterProxyModel executableSortProxyModel;
    executableSortProxyModel.setSourceModel(&executableModel);
    executableSortProxyModel.setSortCaseSensitivity(Qt::CaseInsensitive);
    executableSortProxyModel.setSortRole(JournaldUniqueQueryModel::FIELD);
    QObject::connect(&executableModel, &JournaldUniqueQueryModel::modelReset, [&executableSortProxyModel]() {
        executableSortProxyModel.sort(0);
    });
    executableSortProxyModel.sort(0);

    FilterCriteriaModel filterCriteriaModel;

    QObject::connect(&sessionConfig,
                     &SessionConfig::modeChanged,
                     &sessionConfig,
                     [&sessionConfig, &bootModel, &unitModel, &executableModel, &filterCriteriaModel](SessionConfig::Mode mode) {
                         switch (mode) {
                         case SessionConfig::Mode::SYSTEM:
                             bootModel.setSystemJournal();
                             unitModel.setSystemJournal();
                             executableModel.setSystemJournal();
                             filterCriteriaModel.setSystemJournal();
                             break;
                         case SessionConfig::Mode::REMOTE:
                             // remote is handle like a local access
                             Q_FALLTHROUGH();
                         case SessionConfig::Mode::LOCALFOLDER:
                             bootModel.setJournaldPath(sessionConfig.localJournalPath());
                             unitModel.setJournaldPath(sessionConfig.localJournalPath());
                             executableModel.setJournaldPath(sessionConfig.localJournalPath());
                             filterCriteriaModel.setJournaldPath(sessionConfig.localJournalPath());
                             break;
                         }
                     });
    QObject::connect(&sessionConfig,
                     &SessionConfig::localJournalPathChanged,
                     &sessionConfig,
                     [&sessionConfig, &bootModel, &unitModel, &executableModel, &filterCriteriaModel]() {
                         bootModel.setJournaldPath(sessionConfig.localJournalPath());
                         unitModel.setJournaldPath(sessionConfig.localJournalPath());
                         executableModel.setJournaldPath(sessionConfig.localJournalPath());
                         filterCriteriaModel.setJournaldPath(sessionConfig.localJournalPath());
                     });

    if (parser.isSet(pathOption)) {
        const QString path = parser.value(pathOption);
        bootModel.setJournaldPath(path);
        unitModel.setJournaldPath(path);
        executableModel.setJournaldPath(path);
        sessionConfig.setLocalJournalPath(path);
        sessionConfig.setMode(SessionConfig::Mode::LOCALFOLDER);
    }

    QQmlApplicationEngine engine;
    engine.rootContext()->setContextObject(new KLocalizedContext(&engine));
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
    engine.rootContext()->setContextProperty("g_unitSortProxyModel", &unitSortProxyModel);
    engine.rootContext()->setContextProperty("g_executableModel", &executableModel);
    engine.rootContext()->setContextProperty("g_executableSortProxyModel", &executableSortProxyModel);
    engine.rootContext()->setContextProperty("g_config", &sessionConfig);
    engine.rootContext()->setContextProperty("g_filterModel", &filterCriteriaModel);

    engine.load(url);

    return app.exec();
}
