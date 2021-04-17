/*
    SPDX-License-Identifier: LGPL-2.1-or-later OR MIT
    SPDX-FileCopyrightText: 2021 Andreas Cord-Landwehr <cordlandwehr@kde.org>
*/

#include "test_remotejournal.h"
#include "../testdatalocation.h"
#include "journaldexportreader.h"
#include <localjournal.h>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QTest>
#include <QVector>

void TestRemoteJournal::journalBasicAccess()
{
    QFile exportData(JOURNAL_EXPORT_FORMAT_EXAMPLE);

    JournaldExportReader reader(&exportData);

    // first entry
    {
        reader.readNext();
        JournaldExportReader::LogEntry entry = reader.entry();
        QVERIFY(entry.contains("__CURSOR"));
        QCOMPARE(entry.value("__CURSOR"), "s=739ad463348b4ceca5a9e69c95a3c93f;i=4ece7;b=6c7c6013a26343b29e964691ff25d04c;m=4fc72436e;t=4c508a72423d9;x=d3e5610681098c10;p=system.journal");
        QVERIFY(entry.contains("__REALTIME_TIMESTAMP"));
        QCOMPARE(entry.value("__REALTIME_TIMESTAMP"), "1342540861416409");
        QVERIFY(entry.contains("__MONOTONIC_TIMESTAMP"));
        QCOMPARE(entry.value("__MONOTONIC_TIMESTAMP"), "21415215982");
        QVERIFY(entry.contains("_BOOT_ID"));
        QCOMPARE(entry.value("_BOOT_ID"), "6c7c6013a26343b29e964691ff25d04c");
        QVERIFY(entry.contains("_TRANSPORT"));
        QCOMPARE(entry.value("_TRANSPORT"), "syslog");
        QVERIFY(entry.contains("PRIORITY"));
        QCOMPARE(entry.value("PRIORITY"), "4");
        QVERIFY(entry.contains("SYSLOG_FACILITY"));
        QCOMPARE(entry.value("SYSLOG_FACILITY"), "3");
        QVERIFY(entry.contains("SYSLOG_IDENTIFIER"));
        QCOMPARE(entry.value("SYSLOG_IDENTIFIER"), "gdm-password]");
        QVERIFY(entry.contains("SYSLOG_PID"));
        QCOMPARE(entry.value("SYSLOG_PID"), "587");
        QVERIFY(entry.contains("MESSAGE"));
        QCOMPARE(entry.value("MESSAGE"), "AccountsService-DEBUG(+): ActUserManager: ignoring unspecified session '8' since it's not graphical: Success");
        QVERIFY(entry.contains("_PID"));
        QCOMPARE(entry.value("_PID"), "587");
        QVERIFY(entry.contains("_UID"));
        QCOMPARE(entry.value("_UID"), "0");
        QVERIFY(entry.contains("_GID"));
        QCOMPARE(entry.value("_GID"), "500");
        QVERIFY(entry.contains("_COMM"));
        QCOMPARE(entry.value("_COMM"), "gdm-session-wor");
        QVERIFY(entry.contains("_EXE"));
        QCOMPARE(entry.value("_EXE"), "/usr/libexec/gdm-session-worker");
        QVERIFY(entry.contains("_CMDLINE"));
        QCOMPARE(entry.value("_CMDLINE"), "gdm-session-worker [pam/gdm-password]");
        QVERIFY(entry.contains("_AUDIT_SESSION"));
        QCOMPARE(entry.value("_AUDIT_SESSION"), "2");
        QVERIFY(entry.contains("_AUDIT_LOGINUID"));
        QCOMPARE(entry.value("_AUDIT_LOGINUID"), "500");
        QVERIFY(entry.contains("_SYSTEMD_CGROUP"));
        QCOMPARE(entry.value("_SYSTEMD_CGROUP"), "/user/lennart/2");
        QVERIFY(entry.contains("_SYSTEMD_SESSION"));
        QCOMPARE(entry.value("_SYSTEMD_SESSION"), "2");
        QVERIFY(entry.contains("_SELINUX_CONTEXT"));
        QCOMPARE(entry.value("_SELINUX_CONTEXT"), "system_u:system_r:xdm_t:s0-s0:c0.c1023");
        QVERIFY(entry.contains("_SOURCE_REALTIME_TIMESTAMP"));
        QCOMPARE(entry.value("_SOURCE_REALTIME_TIMESTAMP"), "1342540861413961");
        QVERIFY(entry.contains("_MACHINE_ID"));
        QCOMPARE(entry.value("_MACHINE_ID"), "a91663387a90b89f185d4e860000001a");
        QVERIFY(entry.contains("_HOSTNAME"));
        QCOMPARE(entry.value("_HOSTNAME"), "epsilon");
    }

    // second entry
    {
        reader.readNext();
        JournaldExportReader::LogEntry entry = reader.entry();
        QVERIFY(entry.contains("_SYSTEMD_CGROUP"));
    }

    QVERIFY(reader.atEnd());
}

void TestRemoteJournal::journalBinaryMessageAccess()
{
    QFile exportData(JOURNAL_EXPORT_FORMAT_BINARY_EXAMPLE);

    JournaldExportReader reader(&exportData);

    // first entry
    {
        reader.readNext();
        JournaldExportReader::LogEntry entry = reader.entry();

        std::vector<std::pair<QString, QString>> testValues = {
            {"__CURSOR", "s=bcce4fb8ffcb40e9a6e05eee8b7831bf;i=5ef603;b=ec25d6795f0645619ddac9afdef453ee;m=545242e7049;t=50f1202"},
            {"__REALTIME_TIMESTAMP", "1423944916375353" },
            {"__MONOTONIC_TIMESTAMP", "5794517905481"},
            { "_BOOT_ID", "ec25d6795f0645619ddac9afdef453ee"},
            { "_TRANSPORT", "journal" },
            {"_UID", "1001"},
            {"_GID", "1001"},
            {"_CAP_EFFECTIVE", "0"},
            {"_SYSTEMD_OWNER_UID", "1001"},
            {"_SYSTEMD_SLICE", "user-1001.slice"},
            {"_MACHINE_ID", "5833158886a8445e801d437313d25eff"},
            {"_HOSTNAME", "bupkis"},
            {"_AUDIT_LOGINUID", "1001"},
            {"_SELINUX_CONTEXT", "unconfined_u:unconfined_r:unconfined_t:s0-s0:c0.c1023"},
            {"CODE_LINE", "1"},
            {"CODE_FUNC", "<module>"},
            {"SYSLOG_IDENTIFIER", "python3"},
            {"_COMM", "python3"},
            {"_EXE", "/usr/bin/python3.4"},
            {"_AUDIT_SESSION", "35898"},
            {"_SYSTEMD_CGROUP", "/user.slice/user-1001.slice/session-35898.scope"},
            {"_SYSTEMD_SESSION", "35898"},
            {"_SYSTEMD_UNIT", "session-35898.scope"},
//FIXME not supported yet and currently binary reading breaks stream
//            {"MESSAGE", "foo\nbar"},
//            {"CODE_FILE", "<string>"},
//            {"_PID", "16853"},
//            {"_CMDLINE", "python3 -c from systemd import journal; journal.send(\"foo\\nbar\")"},
//            {"_SOURCE_REALTIME_TIMESTAMP", "1423944916372858"},
        };

        for (const auto &testEntry : testValues) {
            qDebug() << testEntry;
            QVERIFY(entry.contains(testEntry.first));
            QCOMPARE(entry.value(testEntry.first), testEntry.second);
        }
    }

//    QVERIFY(reader.atEnd());
}

QTEST_GUILESS_MAIN(TestRemoteJournal);
