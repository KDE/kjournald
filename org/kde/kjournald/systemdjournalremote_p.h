/*
    SPDX-License-Identifier: LGPL-2.1-or-later OR MIT
    SPDX-FileCopyrightText: 2021 Andreas Cord-Landwehr <cordlandwehr@kde.org>
*/

#ifndef SYSTEMDJOURNALREMOTE_PRIVATE_H
#define SYSTEMDJOURNALREMOTE_PRIVATE_H

#include <QFileSystemWatcher>
#include <QProcess>
#include <QString>
#include <QTemporaryDir>
#include <systemd/sd-journal.h>

class SystemdJournalRemote;

class SystemdJournalRemotePrivate
{
public:
    explicit SystemdJournalRemotePrivate(SystemdJournalRemote *q);
    bool sanityCheckForSystemdJournalRemoteExec() const;
    QString journalFile() const;

    QTemporaryDir mTemporyJournalDir;
    QFileSystemWatcher mTemporaryJournalDirWatcher;
    QProcess mJournalRemoteProcess;
    const QString mSystemdJournalRemoteExec = QLatin1String("/lib/systemd/systemd-journal-remote");
};

#endif
