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
