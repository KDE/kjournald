/*
    SPDX-License-Identifier: LGPL-2.1-or-later OR MIT
    SPDX-FileCopyrightText: 2021 Andreas Cord-Landwehr <cordlandwehr@kde.org>
*/

#include "journal.h"
#include "journal_p.h"
#include "loggingcategories.h"
#include <QDir>
#include <systemd/sd-journal.h>

Journal::Journal()
    : d(new JournalPrivate)
{
    int result;
    result = sd_journal_open(&d->mJournal, SD_JOURNAL_LOCAL_ONLY);
    if (result < 0) {
        qCCritical(journald) << "Failed to open journal:" << strerror(-result);
    }
}

Journal::Journal(const QString &path)
    : d(new JournalPrivate)
{
    if (!QDir().exists(path)) {
        qCCritical(journald) << "Journal directory does not exists, abort opening";
        return;
    }
    int result = sd_journal_open_directory(&d->mJournal, path.toStdString().c_str(), 0 /* no flags, directory defines type */);
    if (result < 0) {
        qCCritical(journald) << "Could not open journal:" << strerror(-result);
    }
}

Journal::~Journal()
{
    sd_journal_close(d->mJournal);
    d->mJournal = nullptr;
}

sd_journal *Journal::sdJournal() const
{
    return d->mJournal;
}

bool Journal::isValid() const
{
    return d->mJournal != nullptr;
}
