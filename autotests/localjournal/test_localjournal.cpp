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

using namespace Qt::StringLiterals;

TestLocalJournal::TestLocalJournal()
    : mBoots{"68f2e61d061247d8a8ba0b8d53a97a52"_L1, "27acae2fe35a40ac93f9c7732c0b8e59"_L1, "2dbe99dd855049af8f2865c5da2b8fda"_L1}
{
}

void TestLocalJournal::journalAccess()
{
    LocalJournal journal(JOURNAL_LOCATION);

    QCOMPARE(journal.usage(), 12845056);
}

QTEST_GUILESS_MAIN(TestLocalJournal);

#include "moc_test_localjournal.cpp"
