/*
    SPDX-License-Identifier: LGPL-2.1-or-later OR MIT
    SPDX-FileCopyrightText: 2021 Andreas Cord-Landwehr <cordlandwehr@kde.org>
*/

#include "aboutproxy.h"
#include "bootmodel.h"
#include "clipboardproxy.h"
#include "fieldfilterproxymodel.h"
#include "filtercriteriamodel.h"
#include "flattenedfiltercriteriaproxymodel.h"
#include "journaldhelper.h"
#include "journalduniquequerymodel.h"
#include "journaldviewmodel.h"
#include "kjournald_version.h"
#include "sessionconfig.h"
#include <KAboutData>
#include <KLocalizedContext>
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
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif
    QApplication app(argc, argv);
    QGuiApplication::setWindowIcon(QIcon::fromTheme(QStringLiteral("utilities-log-viewer")));
    app.setOrganizationName("KDE");

    // use org.kde.desktop style unless another style is forced
    if (qEnvironmentVariableIsEmpty("QT_QUICK_CONTROLS_STYLE")) {
        QQuickStyle::setStyle(QStringLiteral("org.kde.desktop"));
    }

    KLocalizedString::setApplicationDomain("kjournald");
    KAboutData aboutData(QStringLiteral("kjournald"),
                         i18nc("@title Displayed program name", "KJournald Browser"),
                         KJOURNALD_VERSION_STRING,
                         i18nc("@title KAboutData: short program description", "Viewer for Journald logs"),
                         KAboutLicense::LGPL_V2_1,
                         i18nc("@info:credit", "(c) 2021 The KJournald Developers"),
                         i18nc("@title Short program description", "Viewer for Journald databases, which are generated by the Journald logging tool."));
    aboutData.setProgramLogo(app.windowIcon());
    aboutData.addAuthor(i18nc("@info:credit Developer name", "Andreas Cord-Landwehr"),
                        i18nc("@info:credit Role", "Original Author"),
                        QStringLiteral("cordlandwehr@kde.org"));
    aboutData.setTranslator(i18nc("NAME OF TRANSLATORS", "Your names"), i18nc("EMAIL OF TRANSLATORS", "Your emails"));
    KAboutData::setApplicationData(aboutData);

    qmlRegisterType<JournaldViewModel>("kjournald", 1, 0, "JournaldViewModel");
    qmlRegisterType<JournaldUniqueQueryModel>("kjournald", 1, 0, "JournaldUniqueQueryModel");
    qmlRegisterType<FieldFilterProxyModel>("kjournald", 1, 0, "FieldFilterProxyModel");
    qmlRegisterSingletonInstance("kjournald", 1, 0, "Clipboard", new ClipboardProxy);
    qmlRegisterType<AboutProxy>("kjournald", 1, 0, "AboutProxy");
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
    FilterCriteriaModel filterCriteriaModel;

    QObject::connect(&sessionConfig, &SessionConfig::modeChanged, &sessionConfig, [&sessionConfig, &bootModel, &filterCriteriaModel](SessionConfig::Mode mode) {
        switch (mode) {
        case SessionConfig::Mode::SYSTEM:
            bootModel.setSystemJournal();
            filterCriteriaModel.setSystemJournal();
            break;
        case SessionConfig::Mode::REMOTE:
            // remote is handle like a local access
            Q_FALLTHROUGH();
        case SessionConfig::Mode::LOCALFOLDER:
            bootModel.setJournaldPath(sessionConfig.localJournalPath());
            filterCriteriaModel.setJournaldPath(sessionConfig.localJournalPath());
            break;
        }
    });
    QObject::connect(&sessionConfig, &SessionConfig::localJournalPathChanged, &sessionConfig, [&sessionConfig, &bootModel, &filterCriteriaModel]() {
        bootModel.setJournaldPath(sessionConfig.localJournalPath());
        filterCriteriaModel.setJournaldPath(sessionConfig.localJournalPath());
    });

    if (parser.isSet(pathOption)) {
        const QString path = parser.value(pathOption);
        bootModel.setJournaldPath(path);
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
    engine.rootContext()->setContextProperty("g_config", &sessionConfig);
    engine.rootContext()->setContextProperty("g_filterModel", &filterCriteriaModel);

    engine.load(url);

    return app.exec();
}
