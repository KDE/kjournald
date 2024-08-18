/*
    SPDX-License-Identifier: LGPL-2.1-or-later OR MIT
    SPDX-FileCopyrightText: 2021 Andreas Cord-Landwehr <cordlandwehr@kde.org>
*/

#include "test_viewmodel.h"
#include "../containertesthelper.h"
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
#include <QDebug>

// note: this test request several data from a real example journald database
//       you can check them by using "journalctl -D journal" and requesting the values
//       that are checked here

void TestViewModel::journalAccess()
{
    JournaldViewModel model;
    QAbstractItemModelTester tester(&model, QAbstractItemModelTester::FailureReportingMode::QtTest);

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
    QAbstractItemModelTester tester(&model, QAbstractItemModelTester::FailureReportingMode::QtTest);
    QCOMPARE(model.setJournaldPath(JOURNAL_LOCATION), true);
    QVERIFY(model.rowCount() > 0);

    // journalctl -b -2 -D . -o json | head -n2
    std::vector<LogEntry> expectedData{{QDateTime(QDate(2021, 03, 13), QTime(15, 23, 1, 464), Qt::UTC),
                                        4050458,
                                        QString(),
                                        "System clock time unset or jumped backwards, restoring from recorded timestamp: Sat 2021-03-13 15:23:01 UTC",
                                        "systemd-timesyncd.service",
                                        "68f2e61d061247d8a8ba0b8d53a97a52",
                                        "/lib/systemd/systemd-timesyncd",
                                        6},
                                       {QDateTime(QDate(2021, 03, 13), QTime(15, 23, 1, 592), Qt::UTC),
                                        4178254,
                                        QString(),
                                        "klogd started: BusyBox v1.31.1 ()",
                                        QString("busybox-klogd.service"),
                                        "68f2e61d061247d8a8ba0b8d53a97a52",
                                        "/bin/busybox.nosuid",
                                        5}};

    for (int i = 0; i < expectedData.size(); ++i) {
        QCOMPARE(model.data(model.index(i, 0), JournaldViewModel::DATETIME).toDateTime(), expectedData.at(i).mDate);
        QCOMPARE(model.data(model.index(i, 0), JournaldViewModel::MONOTONIC_TIMESTAMP), expectedData.at(i).mMonotonicTimestamp);
        QCOMPARE(model.data(model.index(i, 0), JournaldViewModel::MESSAGE_ID), expectedData.at(i).mId);
        QCOMPARE(model.data(model.index(i, 0), JournaldViewModel::MESSAGE), expectedData.at(i).mMessage);
        QCOMPARE(model.data(model.index(i, 0), JournaldViewModel::PRIORITY), expectedData.at(i).mPriority);
        QCOMPARE(model.data(model.index(i, 0), JournaldViewModel::SYSTEMD_UNIT), expectedData.at(i).mSystemdUnit);
        QCOMPARE(model.data(model.index(i, 0), JournaldViewModel::BOOT_ID), expectedData.at(i).mBootId);
        QCOMPARE(model.data(model.index(i, 0), JournaldViewModel::EXE), expectedData.at(i).mExe);
    }
}

void TestViewModel::bootFilter()
{
    JournaldViewModel model;
    QAbstractItemModelTester tester(&model, QAbstractItemModelTester::FailureReportingMode::QtTest);
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
    // value of 20.000 is chosen arbitrarily from test with this journal archive
    while (model.canFetchMore(QModelIndex()) && model.rowCount() < 20'000) {
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
    QAbstractItemModelTester tester(&model, QAbstractItemModelTester::FailureReportingMode::QtTest);
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
        if (!testSystemdUnitNames.contains(unit)) {
            qDebug() << "unexpected unit:" << unit;
        }
        QVERIFY(testSystemdUnitNames.contains(unit));
    }
    QVERIFY(notFoundUnits.isEmpty());
}

void TestViewModel::showKernelMessages()
{
    // obtained string with:
    // journalctl -b -0 -k -D . -o cat | head -n1
    const QString arbitraryKernelMessage = "brcmfmac: brcmf_fw_alloc_request: using brcm/brcmfmac43455-sdio for chip BCM4345/6";

    JournaldViewModel model;
    QAbstractItemModelTester tester(&model, QAbstractItemModelTester::FailureReportingMode::QtTest);
    QCOMPARE(model.setJournaldPath(JOURNAL_LOCATION), true);

    // check that not contains Kernel message
    model.setKernelFilter(false);
    QVERIFY(model.rowCount() > 0);
    while (model.canFetchMore(QModelIndex())  && model.rowCount() < 2000) {
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

void TestViewModel::closestIndexForDateComputation()
{
    JournaldViewModel model;
    QAbstractItemModelTester tester(&model, QAbstractItemModelTester::FailureReportingMode::QtTest);
    QCOMPARE(model.setJournaldPath(JOURNAL_LOCATION), true);

    QDateTime firstLogEntryDateTime = model.data(model.index(0, 0), JournaldViewModel::DATETIME).toDateTime();
    QDateTime lastLogEntryDateTime = model.data(model.index(model.rowCount() - 1, 0), JournaldViewModel::DATETIME).toDateTime();

    // check for first row in model
    QCOMPARE(model.closestIndexForData(firstLogEntryDateTime), 0);

    // check one day before first entry
    QCOMPARE(model.closestIndexForData(firstLogEntryDateTime.addDays(-1)), 0);

    // check one day after last entry
    QCOMPARE(model.closestIndexForData(lastLogEntryDateTime.addDays(1)), model.rowCount() - 1);

    // check for last row in model
    // last few entries have the exact same date, we default to the first
    QCOMPARE(model.closestIndexForData(lastLogEntryDateTime), model.rowCount() - 1);
}

void TestViewModel::readFullJournal()
{
    JournaldViewModel model;
    QAbstractItemModelTester tester(&model, QAbstractItemModelTester::FailureReportingMode::QtTest);
    QCOMPARE(model.setJournaldPath(JOURNAL_LOCATION), true);
    model.setBootFilter({mBoots.at(0)});
    model.setKernelFilter(true);

    qDebug().noquote() << "check with:"
                       << "journalctl -b" << mBoots.at(0);

    // note: limit chosed big enough such that this fetches the full size of the journal, size is 47
    model.fetchMore(QModelIndex());

    // check agains previous implementation bug that lead to duplication of first and last log line with every
    // read attempt
    int rows = model.rowCount();
    model.fetchMore(QModelIndex());

    QCOMPARE(model.rowCount(), rows);
}

void TestViewModel::resetModelHeadAndTailCursorTest()
{
    JournaldViewModel model;
    QAbstractItemModelTester tester(&model, QAbstractItemModelTester::FailureReportingMode::QtTest);
    QCOMPARE(model.setJournaldPath(JOURNAL_LOCATION), true);

    struct Cursors {
        QString mHead;
        QString mTail;
    };

    std::vector<Cursors> cursors = {
        {"s=c485fef5d17c4272a4a539c4e4708f9e;i=191;b=68f2e61d061247d8a8ba0b8d53a97a52;m=3dce1a;t=5bd6c979f361b;x=766655f78763a257",
         "s=c485fef5d17c4272a4a539c4e4708f9e;i=52b;b=68f2e61d061247d8a8ba0b8d53a97a52;m=312c5fa4;t=5bd6cc9746a62;x=fd8582373d87d313"},
        {"s=df3342d6d57b442da21c78027d3991f8;i=191;b=27acae2fe35a40ac93f9c7732c0b8e59;m=3e2dc9;t=5bd6cc97083b1;x=d775661eeb273df0",
         "s=f1a33fced0cb4d2bb629fb2bd70d1326;i=48e;b=27acae2fe35a40ac93f9c7732c0b8e59;m=67f0d79;t=5bd6cd098ce95;x=c20fdd57f02ca4c5"}};

    // use model and set boot 0
    model.setBootFilter({mBoots.at(0)});
    // boot has 925 non-kernel log entries, fetching once with chunk size 500 gets all
    model.fetchMore(QModelIndex());

    // journalctl -b 68f2e61d061247d8a8ba0b8d53a97a52 -D . -o json|head -n1
    QCOMPARE(model.data(model.index(0, 0), JournaldViewModel::CURSOR), cursors.at(0).mHead);
    // journalctl -b 27acae2fe35a40ac93f9c7732c0b8e59 -D . -o json|tail # then look for first message without driver or kernel TRANSPORT
    QCOMPARE(model.data(model.index(model.rowCount() - 1, 0), JournaldViewModel::CURSOR), cursors.at(0).mTail);

    // invoke tail seeking
    model.seekTail();
    model.fetchMore(QModelIndex());
    QCOMPARE(model.data(model.index(0, 0), JournaldViewModel::CURSOR), cursors.at(0).mHead);
    QCOMPARE(model.data(model.index(model.rowCount() - 1, 0), JournaldViewModel::CURSOR), cursors.at(0).mTail);

    // invoke head seeking
    model.seekHead();
    model.fetchMore(QModelIndex());
    QCOMPARE(model.data(model.index(0, 0), JournaldViewModel::CURSOR), cursors.at(0).mHead);
    QCOMPARE(model.data(model.index(model.rowCount() - 1, 0), JournaldViewModel::CURSOR), cursors.at(0).mTail);

    // use model and set boot 1
    model.setBootFilter({mBoots.at(1)});
    model.fetchMore(QModelIndex());
    // journalctl -b 27acae2fe35a40ac93f9c7732c0b8e59 -D . -o json|head -n1
    QCOMPARE(model.data(model.index(0, 0), JournaldViewModel::CURSOR), cursors.at(1).mHead);
    // journalctl -b 27acae2fe35a40ac93f9c7732c0b8e59 -D . -o json|tail -n1
    QCOMPARE(model.data(model.index(model.rowCount() - 1, 0), JournaldViewModel::CURSOR), cursors.at(1).mTail);

    // invoke tail seeking
    model.seekTail();
    model.fetchMore(QModelIndex());
    QCOMPARE(model.data(model.index(0, 0), JournaldViewModel::CURSOR), cursors.at(1).mHead);
    QCOMPARE(model.data(model.index(model.rowCount() - 1, 0), JournaldViewModel::CURSOR), cursors.at(1).mTail);
    model.seekHead();
    model.fetchMore(QModelIndex());
    QCOMPARE(model.data(model.index(0, 0), JournaldViewModel::CURSOR), cursors.at(1).mHead);
    QCOMPARE(model.data(model.index(model.rowCount() - 1, 0), JournaldViewModel::CURSOR), cursors.at(1).mTail);
}

void TestViewModel::stringSearch()
{
    // obtained index lines with
    // journalctl _TRANSPORT=journal _TRANSPORT=syslog _TRANSPORT=stdout -b -2 --output-fields=_MESSAGE --quiet -D . |grep -n Socket
    // 19:Mär 13 16:23:01 raspberrypi4 systemd[1]: Listening on D-Bus System Message Bus Socket.
    // 23:Mär 13 16:23:01 raspberrypi4 systemd[1]: Reached target Sockets.
    // 291:Mär 13 16:23:01 raspberrypi4 kernel[206]: [    2.416393] systemd[1]: Listening on Syslog Socket.
    // 293:Mär 13 16:23:01 raspberrypi4 kernel[206]: [    2.423412] systemd[1]: Listening on Journal Audit Socket.
    // 294:Mär 13 16:23:01 raspberrypi4 kernel[206]: [    2.427158] systemd[1]: Listening on Journal Socket (/dev/log).
    // 295:Mär 13 16:23:01 raspberrypi4 kernel[206]: [    2.430803] systemd[1]: Listening on Journal Socket.
    // 296:Mär 13 16:23:01 raspberrypi4 kernel[206]: [    2.434436] systemd[1]: Listening on Network Service Netlink Socket.
    // 297:Mär 13 16:23:01 raspberrypi4 kernel[206]: [    2.438047] systemd[1]: Listening on udev Control Socket.
    // 298:Mär 13 16:23:01 raspberrypi4 kernel[206]: [    2.441297] systemd[1]: Listening on udev Kernel Socket.
    // 545:Mär 13 16:23:05 raspberrypi4 systemd[305]: Starting D-Bus User Message Bus Socket.
    // 546:Mär 13 16:23:05 raspberrypi4 systemd[305]: Listening on D-Bus User Message Bus Socket.
    // 547:Mär 13 16:23:05 raspberrypi4 systemd[305]: Reached target Sockets.
    // 734:Mär 13 16:36:56 raspberrypi4 systemd[305]: Stopped target Sockets.
    // 737:Mär 13 16:36:56 raspberrypi4 systemd[305]: Closed D-Bus User Message Bus Socket.
    // 798:Mär 13 16:36:57 raspberrypi4 systemd[1]: Stopped target Sockets.
    // 800:Mär 13 16:36:57 raspberrypi4 systemd[1]: Closed D-Bus System Message Bus Socket.
    // 805:Mär 13 16:36:57 raspberrypi4 systemd[1]: Closed Syslog Socket.

    // note: output of the above grep starts at index 1
    // note: all lines after 596 were redued manuall by 1, because journalclt contains a linebreak in that log output
    const std::vector<int> needleLines = {18, 22, 290, 292, 293, 294, 295, 296, 297, 544, 545, 546, 732, 735, 796, 798, 803};

    // simple check when full journal is alread read
    {
        JournaldViewModel model;
        model.setBootFilter({mBoots.at(0)}); // select boot -2 == 68f2e61d061247d8a8ba0b8d53a97a52
        QAbstractItemModelTester tester(&model, QAbstractItemModelTester::FailureReportingMode::QtTest);
        QCOMPARE(model.setJournaldPath(JOURNAL_LOCATION), true);
        int foundLine{-1};
        std::vector<int> results;
        do {
            foundLine = model.search("Socket", foundLine + 1);
            if (foundLine != -1) {
                results.push_back(foundLine);
            }

        } while (foundLine != -1);
        CONTAINER_EQUAL(needleLines, results);
    }

    // enforce multiple chunk reads
    {
        JournaldViewModel model;
        model.setFetchMoreChunkSize(10);
        model.setBootFilter({mBoots.at(0)}); // select boot -2 == 68f2e61d061247d8a8ba0b8d53a97a52
        QAbstractItemModelTester tester(&model, QAbstractItemModelTester::FailureReportingMode::QtTest);
        QCOMPARE(model.setJournaldPath(JOURNAL_LOCATION), true);
        int foundLine{-1};
        std::vector<int> results;
        do {
            foundLine = model.search("Socket", foundLine + 1);
            if (foundLine != -1) {
                results.push_back(foundLine);
            }

        } while (foundLine != -1);

        // debugging helper
        //    for (int i = 0; i < model.rowCount(); ++i) {
        //        qDebug() << i << ":" << model.data(model.index(i, 0), Qt::DisplayRole).toString();
        //    }

        CONTAINER_EQUAL(needleLines, results);
    }

    // test backward search
    {
        JournaldViewModel model;
        model.setBootFilter({mBoots.at(0)}); // select boot -2 == 68f2e61d061247d8a8ba0b8d53a97a52
        model.seekTail();
        QAbstractItemModelTester tester(&model, QAbstractItemModelTester::FailureReportingMode::QtTest);
        QCOMPARE(model.setJournaldPath(JOURNAL_LOCATION), true);
        model.fetchMore(QModelIndex());
        int foundLine = model.rowCount() - 1;
        std::vector<int> results;
        do {
            foundLine = model.search("Socket", foundLine - 1, JournaldViewModel::BACKWARD);
            if (foundLine != -1) {
                results.push_back(foundLine);
            }

        } while (foundLine != -1);
        std::sort(results.begin(), results.end());
        CONTAINER_EQUAL(needleLines, results);
    }
}

QTEST_GUILESS_MAIN(TestViewModel);
