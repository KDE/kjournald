/*
    SPDX-License-Identifier: LGPL-2.1-or-later OR MIT
    SPDX-FileCopyrightText: 2021 Andreas Cord-Landwehr <cordlandwehr@kde.org>
*/

#ifndef SYSTEMDJOURNALREMOTE_PRIVATE_H
#define SYSTEMDJOURNALREMOTE_PRIVATE_H

#include <QTemporaryFile>
#include <QProcess>
#include <QString>
#include <memory>
#include <systemd/sd-journal.h>

class SystemdJournalRemotePrivate
{
public:
    SystemdJournalRemotePrivate();
    bool sanityCheckForSystemdJournalRemoveExec() const;

    mutable sd_journal *mJournal{nullptr};
    QTemporaryFile mJournalFile;
    QProcess mJournalRemoteProcess;
    const QString mSystemdJournalRemoteExec = "/lib/systemd/systemd-journal-remote";
};

#endif
