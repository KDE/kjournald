/*
    SPDX-License-Identifier: LGPL-2.1-or-later OR MIT
    SPDX-FileCopyrightText: 2021 Andreas Cord-Landwehr <cordlandwehr@kde.org>
*/

#include "test_viewmodel.h"
#include <QTest>
#include <QVector>
#include <QDebug>
#include <QDir>
#include <QTemporaryDir>
#include <QTemporaryFile>
#include "journaldviewmodel.h"
#include "../testdatalocation.h"

// note: this test request several data from a real example journald database
//       you can check them by using "journalctl -D journal" and requesting the values
//       that are checked here

void TestViewModel::journalAccess()
{
    JournaldViewModel model;

    // test failure handling for invalid journal
    QTemporaryFile invalidJournal; // file is surely invalid
    QCOMPARE(model.setJournaldPath(invalidJournal.fileName()), false);
    QCOMPARE(model.journaldPath(), QString());

    // use extracted journal
    QCOMPARE(model.setJournaldPath(JOURNAL_LOCATION), true);
    QCOMPARE(model.journaldPath(), JOURNAL_LOCATION);
}

void TestViewModel::rowAccess()
{
    JournaldViewModel model;
    QCOMPARE(model.setJournaldPath(JOURNAL_LOCATION), true);

    // access unfilter log
    QVERIFY(model.rowCount() > 0);

//    model.setFieldString("_BOOT_ID");
//    QCOMPARE(model.fieldString(), "_BOOT_ID");
//    model.setField(JournaldHelper::Field::BOOT_ID);
//    QCOMPARE(model.fieldString(), "_BOOT_ID");
//    QCOMPARE(model.rowCount(), 3);

//    // check one example value
//    QStringList values;
//    for (int i = 0; i < model.rowCount(); ++i) {
//        values.append(model.data(model.index(i, 0), JournaldUniqueQueryModel::FIELD).toString());
//    }
//    QVERIFY(values.contains("2dbe99dd855049af8f2865c5da2b8fda"));
}

/*
void TestUniqueQuery::systemdUnits()
{
    JournaldUniqueQueryModel model;
    QCOMPARE(model.setJournaldPath(JOURNAL_LOCATION), true);

    model.setFieldString("_SYSTEMD_UNIT");
    QCOMPARE(model.fieldString(), "_SYSTEMD_UNIT");
    model.setField(JournaldHelper::Field::SYSTEMD_UNIT);
    QCOMPARE(model.fieldString(), "_SYSTEMD_UNIT");
    QCOMPARE(model.rowCount(), 17);

    QStringList values;
    for (int i = 0; i < model.rowCount(); ++i) {
        values.append(model.data(model.index(i, 0), JournaldUniqueQueryModel::FIELD).toString());
    }
    QVERIFY(values.contains("systemd-journald.service"));
}*/

QTEST_GUILESS_MAIN(TestViewModel);
