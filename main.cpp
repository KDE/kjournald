/*
    SPDX-License-Identifier: LGPL-2.1-or-later OR MIT
    SPDX-FileCopyrightText: Andreas Cord-Landwehr <cordlandwehr@kde.org>
*/

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <systemd/sd-journal.h>
#include <QDebug>
#include "journaldhelper.h"
#include "journaldviewmodel.h"
#include "journalduniquequerymodel.h"

int main(int argc, char *argv[])
{
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

    QGuiApplication app(argc, argv);

    qmlRegisterType<JournaldViewModel>("systemd", 1, 0, "JournaldViewModel");
    qmlRegisterType<JournaldUniqueQueryModel>("systemd", 1, 0, "JournaldUniqueQueryModel");

//    JournaldHelper helper;
//    qDebug() << helper.queryUnique(JournaldHelper::Field::_BOOT_ID);
//    qDebug() << helper.queryUnique(JournaldHelper::Field::_SYSTEMD_UNIT);

    QQmlApplicationEngine engine;
    const QUrl url(QStringLiteral("qrc:/main.qml"));
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
                     &app, [url](QObject *obj, const QUrl &objUrl) {
        if (!obj && url == objUrl)
            QCoreApplication::exit(-1);
    }, Qt::QueuedConnection);
    engine.load(url);

    return app.exec();
}
