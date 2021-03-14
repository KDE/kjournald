/*
    SPDX-License-Identifier: LGPL-2.1-or-later OR MIT
    SPDX-FileCopyrightText: 2021 Andreas Cord-Landwehr <cordlandwehr@kde.org>
*/

#include "test_uniquequery.h"
#include <QTest>
#include <QVector>
#include <QDebug>
#include <QDir>
#include <QTemporaryDir>
#include <QTemporaryFile>
#include "journalduniquequerymodel.h"
#include "../testdatalocation.h"

// note: this test request several data from a real example journald database
//       you can check them by using "journalctl -D journal" and requesting the values
//       that are checked here

void TestUniqueQuery::journalAccess()
{
    JournaldUniqueQueryModel model;

    // test failure handling for invalid journal
    QTemporaryFile invalidJournal; // file is surely invalid
    QCOMPARE(model.setJournaldPath(invalidJournal.fileName()), false);

    // use extracted journal
    QCOMPARE(model.setJournaldPath(JOURNAL_LOCATION), true);
}

void TestUniqueQuery::boots()
{
    JournaldUniqueQueryModel model;
    QCOMPARE(model.setJournaldPath(JOURNAL_LOCATION), true);

    model.setField("_BOOT_ID");
    QCOMPARE(model.rowCount(), 3);
}

void TestUniqueQuery::systemdUnits()
{
    JournaldUniqueQueryModel model;
    QCOMPARE(model.setJournaldPath(JOURNAL_LOCATION), true);

    model.setField("_SYSTEMD_UNIT");
    QCOMPARE(model.rowCount(), 17);
}

QTEST_GUILESS_MAIN(TestUniqueQuery);
