/*
    SPDX-License-Identifier: LGPL-2.1-or-later OR MIT
    SPDX-FileCopyrightText: 2021 Andreas Cord-Landwehr <cordlandwehr@kde.org>
*/

#include "test_uniquequery.h"
#include "../testdatalocation.h"
#include "journalduniquequerymodel.h"
#include <QAbstractItemModelTester>
#include <QDebug>
#include <QDir>
#include <QTemporaryDir>
#include <QTemporaryFile>
#include <QTest>
#include <QVector>

// note: this test request several data from a real example journald database
//       you can check them by using "journalctl -D journal" and requesting the values
//       that are checked here

void TestUniqueQuery::journalAccess()
{
    JournaldUniqueQueryModel model;
    QAbstractItemModelTester tester(&model, QAbstractItemModelTester::FailureReportingMode::Fatal);

    // test failure handling for invalid journal
    QTemporaryFile invalidJournal; // file is surely invalid
    QCOMPARE(model.setJournaldPath(invalidJournal.fileName()), false);

    // use extracted journal
    QCOMPARE(model.setJournaldPath(JOURNAL_LOCATION), true);
}

void TestUniqueQuery::boots()
{
    JournaldUniqueQueryModel model;
    QAbstractItemModelTester tester(&model, QAbstractItemModelTester::FailureReportingMode::Fatal);
    QCOMPARE(model.setJournaldPath(JOURNAL_LOCATION), true);

    model.setFieldString("_BOOT_ID");
    QCOMPARE(model.fieldString(), "_BOOT_ID");
    model.setField(JournaldHelper::Field::_BOOT_ID);
    QCOMPARE(model.fieldString(), "_BOOT_ID");
    QCOMPARE(model.rowCount(), 3);

    // check one example value
    QStringList values;
    for (int i = 0; i < model.rowCount(); ++i) {
        values.append(model.data(model.index(i, 0), JournaldUniqueQueryModel::FIELD).toString());
    }
    QVERIFY(values.contains("2dbe99dd855049af8f2865c5da2b8fda"));
}

void TestUniqueQuery::systemdUnits()
{
    JournaldUniqueQueryModel model;
    QAbstractItemModelTester tester(&model, QAbstractItemModelTester::FailureReportingMode::Fatal);
    QCOMPARE(model.setJournaldPath(JOURNAL_LOCATION), true);

    model.setFieldString("_SYSTEMD_UNIT");
    QCOMPARE(model.fieldString(), "_SYSTEMD_UNIT");
    model.setField(JournaldHelper::Field::_SYSTEMD_UNIT);
    QCOMPARE(model.fieldString(), "_SYSTEMD_UNIT");
    QCOMPARE(model.rowCount(), 17);

    QStringList values;
    for (int i = 0; i < model.rowCount(); ++i) {
        values.append(model.data(model.index(i, 0), JournaldUniqueQueryModel::FIELD).toString());
    }
    QVERIFY(values.contains("systemd-journald.service"));
}

QTEST_GUILESS_MAIN(TestUniqueQuery);
