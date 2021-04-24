/*
    SPDX-License-Identifier: LGPL-2.1-or-later OR MIT
    SPDX-FileCopyrightText: 2021 Andreas Cord-Landwehr <cordlandwehr@kde.org>
*/

#include "test_localjournal.h"
#include "../testdatalocation.h"
#include <QDebug>
#include <QDir>
#include <QTemporaryDir>
#include <QTemporaryFile>
#include <QTest>
#include <QVector>
#include <localjournal.h>

// note: this test request several data from a real example journald database
//       you can check them by using "journalctl -D journal" and requesting the values
//       that are checked here

void TestLocalJournal::journalAccess()
{
    LocalJournal journal(JOURNAL_LOCATION);

    QCOMPARE(journal.usage(), 12845056);
}

QTEST_GUILESS_MAIN(TestLocalJournal);
