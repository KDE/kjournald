/*
    SPDX-License-Identifier: LGPL-2.1-or-later OR MIT
    SPDX-FileCopyrightText: 2026 Andreas Cord-Landwehr <cordlandwehr@kde.org>
*/

#include "test_journaldhelper.h"
#include "../testdatalocation.h"
#include "journaldhelper.h"
#include "sdjournal.h"
#include <QDebug>
#include <QTest>

// note: this test request several data from a real example journald database
//       you can check them by using "journalctl -D journal" and requesting the values
//       that are checked here

void TestJournaldHelper::queryUniquePerBoot()
{
    QLatin1StringView boot_0{"2dbe99dd855049af8f2865c5da2b8fda"};

    SdJournal journal{JOURNAL_LOCATION};
    QVERIFY(journal.isValid());

    { // single field access
        QStringList results = JournaldHelper::queryUnique(journal.get(), boot_0, JournaldHelper::Field::_SYSTEMD_UNIT);
        QVERIFY(results.count() > 0);
        QCOMPARE(results.removeDuplicates(), 0);
    }
    { // multi field access
        QMap<JournaldHelper::Field, QStringList> results =
            JournaldHelper::queryUnique(journal.get(),
                                        boot_0,
                                        {JournaldHelper::Field::_SYSTEMD_UNIT, JournaldHelper::Field::_SYSTEMD_USER_UNIT, JournaldHelper::Field::_EXE});
        QCOMPARE(results[JournaldHelper::Field::_SYSTEMD_UNIT].removeDuplicates(), 0);
        QVERIFY(results[JournaldHelper::Field::_SYSTEMD_UNIT].size() > 0);
        QVERIFY(results[JournaldHelper::Field::_SYSTEMD_UNIT].contains("user@1000.service")); // arbitrary entry

        QCOMPARE(results[JournaldHelper::Field::_SYSTEMD_USER_UNIT].removeDuplicates(), 0);
        QVERIFY(results[JournaldHelper::Field::_SYSTEMD_USER_UNIT].size() > 0);
        QVERIFY(results[JournaldHelper::Field::_SYSTEMD_USER_UNIT].contains("init.scope")); // arbitrary entry

        QCOMPARE(results[JournaldHelper::Field::_EXE].removeDuplicates(), 0);
        QVERIFY(results[JournaldHelper::Field::_EXE].size() > 0);
        QVERIFY(results[JournaldHelper::Field::_EXE].contains("/lib/systemd/systemd-resolved")); // arbitrary entry
    }
}

QTEST_GUILESS_MAIN(TestJournaldHelper);

#include "moc_test_journaldhelper.cpp"
