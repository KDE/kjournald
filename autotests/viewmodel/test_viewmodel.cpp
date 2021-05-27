/*
    SPDX-License-Identifier: LGPL-2.1-or-later OR MIT
    SPDX-FileCopyrightText: 2021 Andreas Cord-Landwehr <cordlandwehr@kde.org>
*/

#include "test_viewmodel.h"
#include "../testdatalocation.h"
#include "journaldviewmodel.h"
#include "journaldviewmodel_p.h"
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

void TestViewModel::journalAccess()
{
    JournaldViewModel model;
    QAbstractItemModelTester tester(&model, QAbstractItemModelTester::FailureReportingMode::Fatal);

    // test failure handling for invalid journal
    QTemporaryFile invalidJournal; // file is surely invalid
    QCOMPARE(model.setJournaldPath(invalidJournal.fileName()), false);
    QCOMPARE(model.rowCount(), 0);

    // use extracted journal
    QCOMPARE(model.setJournaldPath(JOURNAL_LOCATION), true);
    QVERIFY(model.rowCount() > 0);
}

void TestViewModel::rowAccess()
{
    // used unfiltered journal and check first two lines
    JournaldViewModel model;
    QAbstractItemModelTester tester(&model, QAbstractItemModelTester::FailureReportingMode::Fatal);
    QCOMPARE(model.setJournaldPath(JOURNAL_LOCATION), true);
    QVERIFY(model.rowCount() > 0);

    std::vector<LogEntry> expectedData{{QDateTime::fromString("2021-03-13T16:23:01.464", Qt::ISODateWithMs),
                                        4050458,
                                        QString(),
                                        "System clock time unset or jumped backwards, restoring from recorded timestamp: Sat 2021-03-13 15:23:01 UTC",
                                        "systemd-timesyncd.service",
                                        "68f2e61d061247d8a8ba0b8d53a97a52",
                                        6},
                                       {QDateTime::fromString("2021-03-13T16:23:01.592", Qt::ISODateWithMs),
                                        4178254,
                                        QString(),
                                        "klogd started: BusyBox v1.31.1 ()",
                                        QString("busybox-klogd.service"),
                                        "68f2e61d061247d8a8ba0b8d53a97a52",
                                        5}};

    for (int i = 0; i < expectedData.size(); ++i) {
        QCOMPARE(model.data(model.index(i, 0), JournaldViewModel::DATE), expectedData.at(i).mDate);
        QCOMPARE(model.data(model.index(i, 0), JournaldViewModel::MONOTONIC_TIMESTAMP), expectedData.at(i).mMonotonicTimestamp);
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
    QAbstractItemModelTester tester(&model, QAbstractItemModelTester::FailureReportingMode::Fatal);
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
    for (int i = 0; i < model.rowCount(); ++i) {
        const QString boot = model.data(model.index(i, 0), JournaldViewModel::BOOT_ID).toString();
        firstBootFound |= boot == mBoots.at(0);
        secondBootFound |= boot == mBoots.at(1);
        QVERIFY(boot == mBoots.at(0) || boot == mBoots.at(1));
    }
    QVERIFY(firstBootFound);
    QVERIFY(secondBootFound);
}

void TestViewModel::unitFilter()
{
    JournaldViewModel model;
    QAbstractItemModelTester tester(&model, QAbstractItemModelTester::FailureReportingMode::Fatal);
    QCOMPARE(model.setJournaldPath(JOURNAL_LOCATION), true);

    // select single service
    model.setSystemdUnitFilter({"systemd-networkd.service"});
    QVERIFY(model.rowCount() > 0);
    QCOMPARE(model.data(model.index(0, 0), JournaldViewModel::SYSTEMD_UNIT), "systemd-networkd.service");

    // test mulitple services
    QStringList testSystemdUnitNames{"init.scope", "dbus.service", "systemd-networkd.service"};
    QStringList notFoundUnits = testSystemdUnitNames;
    model.setSystemdUnitFilter(testSystemdUnitNames);

    while (model.canFetchMore(QModelIndex())) {
        model.fetchMore(QModelIndex());
    }
    QVERIFY(model.rowCount() > 0);
    for (int i = 0; i < model.rowCount(); ++i) {
        const QString unit = model.data(model.index(i, 0), JournaldViewModel::SYSTEMD_UNIT).toString();
        notFoundUnits.removeOne(unit);
        QVERIFY(testSystemdUnitNames.contains(unit));
    }
    QVERIFY(notFoundUnits.isEmpty());
}

void TestViewModel::showKernelMessages()
{
    const QString arbitraryKernelMessage = "usb 1-1.1: 3:1: cannot get freq at ep 0x86";

    JournaldViewModel model;
    QAbstractItemModelTester tester(&model, QAbstractItemModelTester::FailureReportingMode::Fatal);
    QCOMPARE(model.setJournaldPath(JOURNAL_LOCATION), true);

    // check that not contains Kernel message
    model.setKernelFilter(false);
    QVERIFY(model.rowCount() > 0);
    while (model.canFetchMore(QModelIndex())) {
        model.fetchMore(QModelIndex());
    }
    for (int i = 0; i < model.rowCount(); ++i) {
        const QString message = model.data(model.index(i, 0), JournaldViewModel::MESSAGE).toString();
        QVERIFY(arbitraryKernelMessage != message);
    }

    // check that Kernel messages are containted
    model.setKernelFilter(true);
    QVERIFY(model.rowCount() > 0);
    while (model.canFetchMore(QModelIndex())) {
        model.fetchMore(QModelIndex());
    }
    bool found{false};
    for (int i = 0; i < model.rowCount(); ++i) {
        const QString message = model.data(model.index(i, 0), JournaldViewModel::MESSAGE).toString();
        if (arbitraryKernelMessage == message) {
            found = true;
        }
    }
    QVERIFY(found);
}

QTEST_GUILESS_MAIN(TestViewModel);
