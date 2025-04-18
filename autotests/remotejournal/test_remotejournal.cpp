/*
    SPDX-License-Identifier: LGPL-2.1-or-later OR MIT
    SPDX-FileCopyrightText: 2021 Andreas Cord-Landwehr <cordlandwehr@kde.org>
*/

#include "test_remotejournal.h"
#include "../testdatalocation.h"
#include "journaldexportreader.h"
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QProcess>
#include <QTest>
#include <QVector>
#include <systemd/sd-journal.h>
#include <systemdjournalremote.h>

void TestRemoteJournal::exportFormatReaderBasicAccess()
{
    QFile exportData(JOURNAL_EXPORT_FORMAT_EXAMPLE);

    JournaldExportReader reader(&exportData);

    // first entry
    {
        reader.readNext();
        JournaldExportReader::LogEntry entry = reader.entry();

        std::vector<std::pair<QString, QString>> testValues = {
            {"__CURSOR",
             "s=739ad463348b4ceca5a9e69c95a3c93f;i=4ece7;b=6c7c6013a26343b29e964691ff25d04c;m=4fc72436e;t=4c508a72423d9;x=d3e5610681098c10;p=system.journal"},
            {"__REALTIME_TIMESTAMP", "1342540861416409"},
            {"__MONOTONIC_TIMESTAMP", "21415215982"},
            {"_BOOT_ID", "6c7c6013a26343b29e964691ff25d04c"},
            {"_TRANSPORT", "syslog"},
            {"PRIORITY", "4"},
            {"SYSLOG_FACILITY", "3"},
            {"SYSLOG_IDENTIFIER", "gdm-password]"},
            {"SYSLOG_PID", "587"},
            {"MESSAGE", "AccountsService-DEBUG(+): ActUserManager: ignoring unspecified session '8' since it's not graphical: Success"},
            {"_PID", "587"},
            {"_UID", "0"},
            {"_GID", "500"},
            {"_COMM", "gdm-session-wor"},
            {"_EXE", "/usr/libexec/gdm-session-worker"},
            {"_CMDLINE", "gdm-session-worker [pam/gdm-password]"},
            {"_AUDIT_SESSION", "2"},
            {"_AUDIT_LOGINUID", "500"},
            {"_SYSTEMD_CGROUP", "/user/lennart/2"},
            {"_SYSTEMD_SESSION", "2"},
            {"_SELINUX_CONTEXT", "system_u:system_r:xdm_t:s0-s0:c0.c1023"},
            {"_SOURCE_REALTIME_TIMESTAMP", "1342540861413961"},
            {"_MACHINE_ID", "a91663387a90b89f185d4e860000001a"},
            {"_HOSTNAME", "epsilon"},
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
            {"__CURSOR", "s=4801b45403ee41f9bfc72b56ef154ecf;i=1799;b=750d24b817364f5ebc286c0b32df2ad0;m=a4d22d016;t=5c8678d812639;x=8420cef2a679132b"},
            {"__REALTIME_TIMESTAMP", "1627721964791353"},
            {"__MONOTONIC_TIMESTAMP", "44243800086"},
            {"_BOOT_ID", "750d24b817364f5ebc286c0b32df2ad0"},
            {"_TRANSPORT", "journal"},
            {"_UID", "1000"},
            {"_GID", "1000"},
            {"_CAP_EFFECTIVE", "0"},
            {"_SELINUX_CONTEXT", "unconfined\n"}, // FIXME until here it work, empty line not noticesd as binary blob
            {"_AUDIT_LOGINUID", "1000"},
            {"_SYSTEMD_OWNER_UID", "1000"},
            {"_SYSTEMD_UNIT", "user@1000.service"},
            {"_SYSTEMD_SLICE", "user-1000.slice"},
            {"_MACHINE_ID", "83a52f20bd334d7f82cb6c7db0b85681"},
            {"_HOSTNAME", "behemoth"},
            {"_SYSTEMD_USER_SLICE", "app.slice"},
            {"_AUDIT_SESSION", "3"},
            {"_SYSTEMD_CGROUP", "/user.slice/user-1000.slice/user@1000.service/app.slice/app-org.kde.yakuake-c0faec5b95cf49f6b49d3eb582fa7991.scope"},
            {"_SYSTEMD_USER_UNIT", "app-org.kde.yakuake-c0faec5b95cf49f6b49d3eb582fa7991.scope"},
            {"_SYSTEMD_INVOCATION_ID", "d8ff5db7d38e4274a5744b388a816ac6"},
            {"MESSAGE", "foo\nbar"},
            {"CODE_FILE", "<string>"},
            {"CODE_LINE", "1"},
            {"CODE_FUNC", "<module>"},
            {"SYSLOG_IDENTIFIER", "python3"},
            {"_COMM", "python3"},
            {"_EXE", "/usr/bin/python3.9"},
            {"_CMDLINE", "python3 -c from systemd import journal; journal.send(\"foo\\nbar\")"},
            {"_PID", "19336"},
            {"_SOURCE_REALTIME_TIMESTAMP", "1627721964791314"},
        };

        for (const auto &testEntry : testValues) {
            qDebug() << "check for entry:" << testEntry;
            QVERIFY(entry.contains(testEntry.first));
            QCOMPARE(entry.value(testEntry.first), testEntry.second);
        }
    }

    QVERIFY(reader.atEnd());
}

void TestRemoteJournal::systemdJournalRemoteJournalFromFile()
{
    // out variables for reading
    const char *data;
    size_t length;
    uint64_t time;
    sd_id128_t bootId;

    SystemdJournalRemote journal(JOURNAL_EXPORT_FORMAT_EXAMPLE);

    if (!journal.isSystemdRemoteAvailable()) {
        QSKIP("Systemd remote is not correctly installed");
    }

    QTRY_COMPARE_WITH_TIMEOUT(journal.isValid(), true, 5000);

    QCOMPARE(sd_journal_seek_head(journal.sdJournal()), 0);

    { // read first entry
        QCOMPARE(sd_journal_next(journal.sdJournal()), 1); // 1 advanced line

        // {"__REALTIME_TIMESTAMP", "1342540861416409"},
        // {"__MONOTONIC_TIMESTAMP", "21415215982"},
        std::vector<std::pair<QString, QString>> testValues = {
            {"_BOOT_ID", "6c7c6013a26343b29e964691ff25d04c"},
            {"_TRANSPORT", "syslog"},
            {"PRIORITY", "4"},
            {"SYSLOG_FACILITY", "3"},
            {"SYSLOG_IDENTIFIER", "gdm-password]"},
            {"SYSLOG_PID", "587"},
            {"MESSAGE", "AccountsService-DEBUG(+): ActUserManager: ignoring unspecified session '8' since it's not graphical: Success"},
            {"_PID", "587"},
            {"_UID", "0"},
            {"_GID", "500"},
            {"_COMM", "gdm-session-wor"},
            {"_EXE", "/usr/libexec/gdm-session-worker"},
            {"_CMDLINE", "gdm-session-worker [pam/gdm-password]"},
            {"_AUDIT_SESSION", "2"},
            {"_AUDIT_LOGINUID", "500"},
            {"_SYSTEMD_CGROUP", "/user/lennart/2"},
            {"_SYSTEMD_SESSION", "2"},
            {"_SELINUX_CONTEXT", "system_u:system_r:xdm_t:s0-s0:c0.c1023"},
            {"_SOURCE_REALTIME_TIMESTAMP", "1342540861413961"},
            {"_MACHINE_ID", "a91663387a90b89f185d4e860000001a"},
            {"_HOSTNAME", "epsilon"},
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

void TestRemoteJournal::systemdJournalRemoteJournalFromLocalhost()
{
    // spawning systemd-journal-gatwayd to provide http access
    QProcess systemdJournalGatwaydProcess;
    const QString journaldGatwaydPath = "/lib/systemd/systemd-journal-gatewayd";

    if (!QFile::exists(journaldGatwaydPath)) {
        qCritical() << "Skipping remote journal test, gateway process not available:" << journaldGatwaydPath;
        return;
    }

    systemdJournalGatwaydProcess.setProcessChannelMode(QProcess::ForwardedChannels);
    systemdJournalGatwaydProcess.start(journaldGatwaydPath, QStringList() << "-D" << JOURNAL_LOCATION);
    QVERIFY(systemdJournalGatwaydProcess.waitForStarted());

    // out variables for reading
    const char *data;
    size_t length;
    uint64_t time;
    sd_id128_t bootId;

    SystemdJournalRemote journal("http://127.0.0.1", "19531");
    QTRY_COMPARE_WITH_TIMEOUT(journal.isValid(), true, 5000);
    QCOMPARE(sd_journal_seek_head(journal.sdJournal()), 0);

    systemdJournalGatwaydProcess.terminate();
    systemdJournalGatwaydProcess.waitForFinished(5000);
    if (systemdJournalGatwaydProcess.state() == QProcess::Running) {
        systemdJournalGatwaydProcess.kill();
        systemdJournalGatwaydProcess.waitForFinished();
    }
    QVERIFY(systemdJournalGatwaydProcess.state() == QProcess::NotRunning);
}

QTEST_GUILESS_MAIN(TestRemoteJournal);
