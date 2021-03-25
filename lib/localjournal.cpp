/*
    SPDX-License-Identifier: LGPL-2.1-or-later OR MIT
    SPDX-FileCopyrightText: 2021 Andreas Cord-Landwehr <cordlandwehr@kde.org>
*/

#include "localjournal.h"
#include "localjournal_p.h"
#include "loggingcategories.h"
#include <QDir>
#include <systemd/sd-journal.h>

LocalJournal::LocalJournal()
    : d(new LocalJournalPrivate)
{
    int result;
    result = sd_journal_open(&d->mJournal, SD_JOURNAL_LOCAL_ONLY);
    if (result < 0) {
        qCCritical(journald) << "Failed to open journal:" << strerror(-result);
    } else {
        d->mFd = sd_journal_get_fd(d->mJournal);
        d->mJournalSocketNotifier = std::make_unique<QSocketNotifier>(d->mFd, QSocketNotifier::Read);
        connect(d->mJournalSocketNotifier.get(), &QSocketNotifier::activated, this, &LocalJournal::handleJournalDescriptorUpdate);
    }
}

LocalJournal::LocalJournal(const QString &path)
    : d(new LocalJournalPrivate)
{
    if (!QDir().exists(path)) {
        qCCritical(journald) << "Journal directory does not exist, abort opening" << path;
        return;
    }
    int result = sd_journal_open_directory(&d->mJournal, path.toStdString().c_str(), 0 /* no flags, directory defines type */);
    if (result < 0) {
        qCCritical(journald) << "Could not open journal:" << strerror(-result);
    }
}

LocalJournal::~LocalJournal()
{
    sd_journal_close(d->mJournal);
    d->mJournal = nullptr;
}

sd_journal *LocalJournal::sdJournal() const
{
    return d->mJournal;
}

bool LocalJournal::isValid() const
{
    return d->mJournal != nullptr;
}

void LocalJournal::handleJournalDescriptorUpdate()
{
    // reset descriptor
    QFile file;
    file.open(d->mFd, QIODevice::ReadOnly);
    file.readAll();
    file.close();
    qCDebug(journald()) << "FD updated";
    Q_EMIT journalUpdated();
}
