/*
    SPDX-License-Identifier: LGPL-2.1-or-later OR MIT
    SPDX-FileCopyrightText: 2021 Andreas Cord-Landwehr <cordlandwehr@kde.org>
*/

#include "test_remotejournal.h"
#include "../testdatalocation.h"
#include "journaldexportreader.h"
#include <systemdjournalremote.h>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QTest>
#include <QVector>
#include <systemd/sd-journal.h>

void TestRemoteJournal::exportFormatReaderBasicAccess()
{
    QFile exportData(JOURNAL_EXPORT_FORMAT_EXAMPLE);

    JournaldExportReader reader(&exportData);

    // first entry
    {
        reader.readNext();
        JournaldExportReader::LogEntry entry = reader.entry();

        std::vector<std::pair<QString, QString>> testValues = {
            {"__CURSOR", "s=739ad463348b4ceca5a9e69c95a3c93f;i=4ece7;b=6c7c6013a26343b29e964691ff25d04c;m=4fc72436e;t=4c508a72423d9;x=d3e5610681098c10;p=system.journal"},
            {"__REALTIME_TIMESTAMP", "1342540861416409"},
            {"__MONOTONIC_TIMESTAMP", "21415215982"},
            {"_BOOT_ID", "6c7c6013a26343b29e964691ff25d04c"},
            {"_TRANSPORT","syslog"},
            {"PRIORITY","4"},
            {"SYSLOG_FACILITY","3"},
            {"SYSLOG_IDENTIFIER","gdm-password]"},
            {"SYSLOG_PID","587"},
            {"MESSAGE","AccountsService-DEBUG(+): ActUserManager: ignoring unspecified session '8' since it's not graphical: Success"},
            {"_PID","587"},
            {"_UID","0"},
            {"_GID","500"},
            {"_COMM","gdm-session-wor"},
            {"_EXE","/usr/libexec/gdm-session-worker"},
            {"_CMDLINE","gdm-session-worker [pam/gdm-password]"},
            {"_AUDIT_SESSION","2"},
            {"_AUDIT_LOGINUID","500"},
            {"_SYSTEMD_CGROUP","/user/lennart/2"},
            {"_SYSTEMD_SESSION","2"},
            {"_SELINUX_CONTEXT","system_u:system_r:xdm_t:s0-s0:c0.c1023"},
            {"_SOURCE_REALTIME_TIMESTAMP","1342540861413961"},
            {"_MACHINE_ID","a91663387a90b89f185d4e860000001a"},
            {"_HOSTNAME","epsilon"},
        };
        for (const auto &testEntry : testValues) {
            QVERIFY(entry.contains(testEntry.first));
            QCOMPARE(entry.value(testEntry.first), testEntry.second);
        }
    }

    // second entry
    {
        reader.readNext();
        JournaldExportReader::LogEntry entry = reader.entry();
        QVERIFY(entry.contains("_SYSTEMD_CGROUP"));
    }

    QVERIFY(reader.atEnd());
}

void TestRemoteJournal::exportFormatReaderBinaryMessageAccess()
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

void TestRemoteJournal::systemdJournalRemoteJournalFromFile()
{
    // out variables for reading
    const char *data;
    size_t length;
    uint64_t time;
    sd_id128_t bootId;

    SystemdJournalRemote journal(JOURNAL_EXPORT_FORMAT_EXAMPLE);
    QCOMPARE(sd_journal_seek_head(journal.sdJournal()), 0);

    { // read first entry
        QCOMPARE(sd_journal_next(journal.sdJournal()), 1); // 1 advanced line

        // {"__REALTIME_TIMESTAMP", "1342540861416409"},
        // {"__MONOTONIC_TIMESTAMP", "21415215982"},
        std::vector<std::pair<QString, QString>> testValues = {
            {"_BOOT_ID", "6c7c6013a26343b29e964691ff25d04c"},
            {"_TRANSPORT","syslog"},
            {"PRIORITY","4"},
            {"SYSLOG_FACILITY","3"},
            {"SYSLOG_IDENTIFIER","gdm-password]"},
            {"SYSLOG_PID","587"},
            {"MESSAGE","AccountsService-DEBUG(+): ActUserManager: ignoring unspecified session '8' since it's not graphical: Success"},
            {"_PID","587"},
            {"_UID","0"},
            {"_GID","500"},
            {"_COMM","gdm-session-wor"},
            {"_EXE","/usr/libexec/gdm-session-worker"},
            {"_CMDLINE","gdm-session-worker [pam/gdm-password]"},
            {"_AUDIT_SESSION","2"},
            {"_AUDIT_LOGINUID","500"},
            {"_SYSTEMD_CGROUP","/user/lennart/2"},
            {"_SYSTEMD_SESSION","2"},
            {"_SELINUX_CONTEXT","system_u:system_r:xdm_t:s0-s0:c0.c1023"},
            {"_SOURCE_REALTIME_TIMESTAMP","1342540861413961"},
            {"_MACHINE_ID","a91663387a90b89f185d4e860000001a"},
            {"_HOSTNAME","epsilon"},
        };

        // read and test real time
        QCOMPARE(sd_journal_get_realtime_usec(journal.sdJournal(), &time), 0);
        QCOMPARE(time, 1342540861416409);

        // read and test monotonic time
        QCOMPARE(sd_journal_get_monotonic_usec(journal.sdJournal(), &time, &bootId), 0);
        QCOMPARE(time, 21415215982);

        // read and test fields
        for (const auto &testEntry : testValues) {
            QByteArray field = testEntry.first.toLocal8Bit();
            QCOMPARE(sd_journal_get_data(journal.sdJournal(), field.data(), (const void **)&data, &length), 0);
            QCOMPARE(QString::fromUtf8((const char *)data, length), testEntry.first + "=" + testEntry.second);
        }
    }

    {
        QCOMPARE(sd_journal_next(journal.sdJournal()), 1);

        // {"__REALTIME_TIMESTAMP", "1342540861421465"},
        // {"__MONOTONIC_TIMESTAMP", "21415221039"},

        std::vector<std::pair<QString, QString>> testValues = {
            {"_BOOT_ID", "6c7c6013a26343b29e964691ff25d04c"},
            {"_TRANSPORT", "syslog"},
            {"PRIORITY", "6"},
            {"SYSLOG_FACILITY", "9"},
            {"SYSLOG_IDENTIFIER", "/USR/SBIN/CROND"},
            {"SYSLOG_PID", "8278"},
            {"MESSAGE", "(root) CMD (run-parts /etc/cron.hourly)"},
            {"_PID", "8278"},
            {"_UID", "0"},
            {"_GID", "0"},
            {"_COMM", "run-parts"},
            {"_EXE", "/usr/bin/bash"},
            {"_CMDLINE", "/bin/bash /bin/run-parts /etc/cron.hourly"},
            {"_AUDIT_SESSION", "8"},
            {"_AUDIT_LOGINUID", "0"},
            {"_SYSTEMD_CGROUP", "/user/root/8"},
            {"_SYSTEMD_SESSION", "8"},
            {"_SELINUX_CONTEXT", "system_u:system_r:crond_t:s0-s0:c0.c1023"},
            {"_SOURCE_REALTIME_TIMESTAMP", "1342540861416351"},
            {"_MACHINE_ID", "a91663387a90b89f185d4e860000001a"},
            {"_HOSTNAME", "epsilon"},
        };

        // read and test real time
        QCOMPARE(sd_journal_get_realtime_usec(journal.sdJournal(), &time), 0);
        QCOMPARE(time, 1342540861421465);

        // read and test monotonic time
        QCOMPARE(sd_journal_get_monotonic_usec(journal.sdJournal(), &time, &bootId), 0);
        QCOMPARE(time, 21415221039);

        // read and test fields
        for (const auto &testEntry : testValues) {
            QByteArray field = testEntry.first.toLocal8Bit();
            QCOMPARE(sd_journal_get_data(journal.sdJournal(), field.data(), (const void **)&data, &length), 0);
            QCOMPARE(QString::fromUtf8((const char *)data, length), testEntry.first + "=" + testEntry.second);
        }
    }
}

QTEST_GUILESS_MAIN(TestRemoteJournal);
