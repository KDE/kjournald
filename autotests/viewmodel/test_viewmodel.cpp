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
#include "journaldviewmodel_p.h"
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
    // used unfiltered journal and check first two lines
    JournaldViewModel model;
    QCOMPARE(model.setJournaldPath(JOURNAL_LOCATION), true);
    QVERIFY(model.rowCount() > 0);

    std::vector<LogEntry> expectedData{
        {QDateTime::fromString("2021-03-13T16:23:01.464", Qt::ISODateWithMs), QString(), "System clock time unset or jumped backwards, restoring from recorded timestamp: Sat 2021-03-13 15:23:01 UTC", "systemd-timesyncd.service", "68f2e61d061247d8a8ba0b8d53a97a52", 6},
        {QDateTime::fromString("2021-03-13T16:23:01.592", Qt::ISODateWithMs), QString(), "uvcvideo: Found UVC 1.00 device FHD Camera Microphone (1bcf:28c4)", QString(), "68f2e61d061247d8a8ba0b8d53a97a52", 6}
    };

    for (int i = 0; i < expectedData.size(); ++i) {
        QCOMPARE(model.data(model.index(i, 0), JournaldViewModel::DATE), expectedData.at(i).mDate);
        QCOMPARE(model.data(model.index(i, 0), JournaldViewModel::MESSAGE_ID), expectedData.at(i).mId);
        QCOMPARE(model.data(model.index(i, 0), JournaldViewModel::MESSAGE), expectedData.at(i).mMessage);
        QCOMPARE(model.data(model.index(i, 0), JournaldViewModel::PRIORITY), expectedData.at(i).mPriority);
        QCOMPARE(model.data(model.index(i, 0), JournaldViewModel::SYSTEMD_UNIT), expectedData.at(i).mSystemdUnit);
        QCOMPARE(model.data(model.index(i, 0), JournaldViewModel::BOOT_ID), expectedData.at(i).mBootId);
    }
}

void TestViewModel::bootFilter()
{
    JournaldViewModel model;
    QCOMPARE(model.setJournaldPath(JOURNAL_LOCATION), true);

    // select only second boot
    model.setBootFilter({mBoots.at(1)});
    QVERIFY(model.rowCount() > 0);
    QCOMPARE(model.data(model.index(0, 0), JournaldViewModel::BOOT_ID), mBoots.at(1));

    // select only first boot
    model.setBootFilter({mBoots.at(0)});
    QVERIFY(model.rowCount() > 0);
    QCOMPARE(model.data(model.index(0, 0), JournaldViewModel::BOOT_ID), mBoots.at(0));

    // select first and second boot
    bool firstBootFound = false;
    bool secondBootFound = false;
    model.setBootFilter({mBoots.at(0), mBoots.at(1)});
    while (model.canFetchMore(QModelIndex())) {
        model.fetchMore(QModelIndex());
    }
    QVERIFY(model.rowCount() > 0);
    for (int i = 0; i < model.rowCount(); ++i)  {
        const QString boot = model.data(model.index(i, 0), JournaldViewModel::BOOT_ID).toString();
        firstBootFound |= boot == mBoots.at(0);
        secondBootFound |= boot == mBoots.at(1);
        QVERIFY(boot == mBoots.at(0) || boot == mBoots.at(1));
    }
    QVERIFY(firstBootFound);
    QVERIFY(secondBootFound);
}

QTEST_GUILESS_MAIN(TestViewModel);
