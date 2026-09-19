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

using namespace Qt::StringLiterals;

void TestRemoteJournal::exportFormatReaderBasicAccess()
{
    QFile exportData(JOURNAL_EXPORT_FORMAT_EXAMPLE);

    JournaldExportReader reader(&exportData);

    // first entry
    {
        reader.readNext();
        JournaldExportReader::LogEntry entry = reader.entry();

        std::vector<std::pair<QString, QString>> testValues = {
            {"__CURSOR"_L1,
             "s=739ad463348b4ceca5a9e69c95a3c93f;i=4ece7;b=6c7c6013a26343b29e964691ff25d04c;m=4fc72436e;t=4c508a72423d9;x=d3e5610681098c10;p=system.journal"_L1},
            {"__REALTIME_TIMESTAMP"_L1, "1342540861416409"_L1},
            {"__MONOTONIC_TIMESTAMP"_L1, "21415215982"_L1},
            {"_BOOT_ID"_L1, "6c7c6013a26343b29e964691ff25d04c"_L1},
            {"_TRANSPORT"_L1, "syslog"_L1},
            {"PRIORITY"_L1, "4"_L1},
            {"SYSLOG_FACILITY"_L1, "3"_L1},
            {"SYSLOG_IDENTIFIER"_L1, "gdm-password]"_L1},
            {"SYSLOG_PID"_L1, "587"_L1},
            {"MESSAGE"_L1, "AccountsService-DEBUG(+): ActUserManager: ignoring unspecified session '8' since it's not graphical: Success"_L1},
            {"_PID"_L1, "587"_L1},
            {"_UID"_L1, "0"_L1},
            {"_GID"_L1, "500"_L1},
            {"_COMM"_L1, "gdm-session-wor"_L1},
            {"_EXE"_L1, "/usr/libexec/gdm-session-worker"_L1},
            {"_CMDLINE"_L1, "gdm-session-worker [pam/gdm-password]"_L1},
            {"_AUDIT_SESSION"_L1, "2"_L1},
            {"_AUDIT_LOGINUID"_L1, "500"_L1},
            {"_SYSTEMD_CGROUP"_L1, "/user/lennart/2"_L1},
            {"_SYSTEMD_SESSION"_L1, "2"_L1},
            {"_SELINUX_CONTEXT"_L1, "system_u:system_r:xdm_t:s0-s0:c0.c1023"_L1},
            {"_SOURCE_REALTIME_TIMESTAMP"_L1, "1342540861413961"_L1},
            {"_MACHINE_ID"_L1, "a91663387a90b89f185d4e860000001a"_L1},
            {"_HOSTNAME"_L1, "epsilon"_L1},
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
        QVERIFY(entry.contains("_SYSTEMD_CGROUP"_L1));
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
            {"__CURSOR"_L1, "s=4801b45403ee41f9bfc72b56ef154ecf;i=1799;b=750d24b817364f5ebc286c0b32df2ad0;m=a4d22d016;t=5c8678d812639;x=8420cef2a679132b"_L1},
            {"__REALTIME_TIMESTAMP"_L1, "1627721964791353"_L1},
            {"__MONOTONIC_TIMESTAMP"_L1, "44243800086"_L1},
            {"_BOOT_ID"_L1, "750d24b817364f5ebc286c0b32df2ad0"_L1},
            {"_TRANSPORT"_L1, "journal"_L1},
            {"_UID"_L1, "1000"_L1},
            {"_GID"_L1, "1000"_L1},
            {"_CAP_EFFECTIVE"_L1, "0"_L1},
            {"_SELINUX_CONTEXT"_L1, "unconfined\n"_L1}, // FIXME until here it work, empty line not noticed as binary blob
            {"_AUDIT_LOGINUID"_L1, "1000"_L1},
            {"_SYSTEMD_OWNER_UID"_L1, "1000"_L1},
            {"_SYSTEMD_UNIT"_L1, "user@1000.service"_L1},
            {"_SYSTEMD_SLICE"_L1, "user-1000.slice"_L1},
            {"_MACHINE_ID"_L1, "83a52f20bd334d7f82cb6c7db0b85681"_L1},
            {"_HOSTNAME"_L1, "behemoth"_L1},
            {"_SYSTEMD_USER_SLICE"_L1, "app.slice"_L1},
            {"_AUDIT_SESSION"_L1, "3"_L1},
            {"_SYSTEMD_CGROUP"_L1, "/user.slice/user-1000.slice/user@1000.service/app.slice/app-org.kde.yakuake-c0faec5b95cf49f6b49d3eb582fa7991.scope"_L1},
            {"_SYSTEMD_USER_UNIT"_L1, "app-org.kde.yakuake-c0faec5b95cf49f6b49d3eb582fa7991.scope"_L1},
            {"_SYSTEMD_INVOCATION_ID"_L1, "d8ff5db7d38e4274a5744b388a816ac6"_L1},
            {"MESSAGE"_L1, "foo\nbar"_L1},
            {"CODE_FILE"_L1, "<string>"_L1},
            {"CODE_LINE"_L1, "1"_L1},
            {"CODE_FUNC"_L1, "<module>"_L1},
            {"SYSLOG_IDENTIFIER"_L1, "python3"_L1},
            {"_COMM"_L1, "python3"_L1},
            {"_EXE"_L1, "/usr/bin/python3.9"_L1},
            {"_CMDLINE"_L1, "python3 -c from systemd import journal; journal.send(\"foo\\nbar\")"_L1},
            {"_PID"_L1, "19336"_L1},
            {"_SOURCE_REALTIME_TIMESTAMP"_L1, "1627721964791314"_L1},
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

    SystemdJournalRemote provider(JOURNAL_EXPORT_FORMAT_EXAMPLE);
    auto journal = provider.openJournal();
    QVERIFY(journal);

    if (!provider.isSystemdRemoteAvailable()) {
        QSKIP("Systemd remote is not correctly installed");
    }

    QTRY_COMPARE_WITH_TIMEOUT(journal->isValid(), true, 5000);

    if (!provider.isSystemdRemoteAvailable()) {
        qWarning() << "Skip further test operations due to systemd remote not being available";
        return;
    }

    QCOMPARE(sd_journal_seek_head(journal->get()), 0);

    { // read first entry
        QCOMPARE(sd_journal_next(journal->get()), 1); // 1 advanced line

        // {"__REALTIME_TIMESTAMP", "1342540861416409"},
        // {"__MONOTONIC_TIMESTAMP", "21415215982"},
        std::vector<std::pair<QString, QString>> testValues = {
            {"_BOOT_ID"_L1, "6c7c6013a26343b29e964691ff25d04c"_L1},
            {"_TRANSPORT"_L1, "syslog"_L1},
            {"PRIORITY"_L1, "4"_L1},
            {"SYSLOG_FACILITY"_L1, "3"_L1},
            {"SYSLOG_IDENTIFIER"_L1, "gdm-password]"_L1},
            {"SYSLOG_PID"_L1, "587"_L1},
            {"MESSAGE"_L1, "AccountsService-DEBUG(+): ActUserManager: ignoring unspecified session '8' since it's not graphical: Success"_L1},
            {"_PID"_L1, "587"_L1},
            {"_UID"_L1, "0"_L1},
            {"_GID"_L1, "500"_L1},
            {"_COMM"_L1, "gdm-session-wor"_L1},
            {"_EXE"_L1, "/usr/libexec/gdm-session-worker"_L1},
            {"_CMDLINE"_L1, "gdm-session-worker [pam/gdm-password]"_L1},
            {"_AUDIT_SESSION"_L1, "2"_L1},
            {"_AUDIT_LOGINUID"_L1, "500"_L1},
            {"_SYSTEMD_CGROUP"_L1, "/user/lennart/2"_L1},
            {"_SYSTEMD_SESSION"_L1, "2"_L1},
            {"_SELINUX_CONTEXT"_L1, "system_u:system_r:xdm_t:s0-s0:c0.c1023"_L1},
            {"_SOURCE_REALTIME_TIMESTAMP"_L1, "1342540861413961"_L1},
            {"_MACHINE_ID"_L1, "a91663387a90b89f185d4e860000001a"_L1},
            {"_HOSTNAME"_L1, "epsilon"_L1},
        };

        // read and test real time
        QCOMPARE(sd_journal_get_realtime_usec(journal->get(), &time), 0);
        QCOMPARE(time, 1342540861416409);

        // read and test monotonic time
        QCOMPARE(sd_journal_get_monotonic_usec(journal->get(), &time, &bootId), 0);
        QCOMPARE(time, 21415215982);

        // read and test fields
        for (const auto &testEntry : testValues) {
            QByteArray field = testEntry.first.toLocal8Bit();
            QCOMPARE(sd_journal_get_data(journal->get(), field.data(), (const void **)&data, &length), 0);
            QCOMPARE(QString::fromUtf8((const char *)data, length), testEntry.first + "="_L1 + testEntry.second);
        }
    }

    {
        QCOMPARE(sd_journal_next(journal->get()), 1);

        // {"__REALTIME_TIMESTAMP", "1342540861421465"},
        // {"__MONOTONIC_TIMESTAMP", "21415221039"},

        std::vector<std::pair<QString, QString>> testValues = {
            {"_BOOT_ID"_L1, "6c7c6013a26343b29e964691ff25d04c"_L1},
            {"_TRANSPORT"_L1, "syslog"_L1},
            {"PRIORITY"_L1, "6"_L1},
            {"SYSLOG_FACILITY"_L1, "9"_L1},
            {"SYSLOG_IDENTIFIER"_L1, "/USR/SBIN/CROND"_L1},
            {"SYSLOG_PID"_L1, "8278"_L1},
            {"MESSAGE"_L1, "(root) CMD (run-parts /etc/cron.hourly)"_L1},
            {"_PID"_L1, "8278"_L1},
            {"_UID"_L1, "0"_L1},
            {"_GID"_L1, "0"_L1},
            {"_COMM"_L1, "run-parts"_L1},
            {"_EXE"_L1, "/usr/bin/bash"_L1},
            {"_CMDLINE"_L1, "/bin/bash /bin/run-parts /etc/cron.hourly"_L1},
            {"_AUDIT_SESSION"_L1, "8"_L1},
            {"_AUDIT_LOGINUID"_L1, "0"_L1},
            {"_SYSTEMD_CGROUP"_L1, "/user/root/8"_L1},
            {"_SYSTEMD_SESSION"_L1, "8"_L1},
            {"_SELINUX_CONTEXT"_L1, "system_u:system_r:crond_t:s0-s0:c0.c1023"_L1},
            {"_SOURCE_REALTIME_TIMESTAMP"_L1, "1342540861416351"_L1},
            {"_MACHINE_ID"_L1, "a91663387a90b89f185d4e860000001a"_L1},
            {"_HOSTNAME"_L1, "epsilon"_L1},
        };

        // read and test real time
        QCOMPARE(sd_journal_get_realtime_usec(journal->get(), &time), 0);
        QCOMPARE(time, 1342540861421465);

        // read and test monotonic time
        QCOMPARE(sd_journal_get_monotonic_usec(journal->get(), &time, &bootId), 0);
        QCOMPARE(time, 21415221039);

        // read and test fields
        for (const auto &testEntry : testValues) {
            QByteArray field = testEntry.first.toLocal8Bit();
            QCOMPARE(sd_journal_get_data(journal->get(), field.data(), (const void **)&data, &length), 0);
            QCOMPARE(QString::fromUtf8(static_cast<const char *>(data), length), QString("%1=%2"_L1).arg(testEntry.first, testEntry.second));
        }
    }
}

void TestRemoteJournal::systemdJournalRemoteJournalFromLocalhost()
{
    // spawning systemd-journal-gatwayd to provide http access
    QProcess systemdJournalGatwaydProcess;
    const QString journaldGatwaydPath = "/lib/systemd/systemd-journal-gatewayd"_L1;

    if (!QFile::exists(journaldGatwaydPath)) {
        qCritical() << "Skipping remote journal test, gateway process not available:" << journaldGatwaydPath;
        return;
    }

    systemdJournalGatwaydProcess.setProcessChannelMode(QProcess::ForwardedChannels);
    systemdJournalGatwaydProcess.start(journaldGatwaydPath, QStringList() << "-D"_L1 << JOURNAL_LOCATION);
    QVERIFY(systemdJournalGatwaydProcess.waitForStarted());

    SystemdJournalRemote provider("http://127.0.0.1"_L1, "19531"_L1);
    QTRY_COMPARE_WITH_TIMEOUT(provider.isJournalCreated(), true, 5000);
    auto journal = provider.openJournal();
    QVERIFY(journal);
    QCOMPARE(journal->isValid(), true);
    QCOMPARE(sd_journal_seek_head(journal->get()), 0);

    systemdJournalGatwaydProcess.terminate();
    systemdJournalGatwaydProcess.waitForFinished(5000);
    if (systemdJournalGatwaydProcess.state() == QProcess::Running) {
        systemdJournalGatwaydProcess.kill();
        systemdJournalGatwaydProcess.waitForFinished();
    }
    QVERIFY(systemdJournalGatwaydProcess.state() == QProcess::NotRunning);
}

QTEST_GUILESS_MAIN(TestRemoteJournal);

#include "moc_test_remotejournal.cpp"
