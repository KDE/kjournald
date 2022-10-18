/*
    SPDX-License-Identifier: LGPL-2.1-or-later OR MIT
    SPDX-FileCopyrightText: 2021 Andreas Cord-Landwehr <cordlandwehr@kde.org>
*/

#include "localjournal.h"
#include "localjournal_p.h"
#include "loggingcategories.h"
#include <QDir>
#include <systemd/sd-journal.h>

LocalJournalPrivate::LocalJournalPrivate()
{
    QFile file(QLatin1String("/proc/sys/kernel/random/boot_id"));
    if (file.open(QIODevice::ReadOnly | QFile::Text)) {
        QTextStream stream(&file);
        // example value: "918581c5-a27a-4ac6-9f37-c160fd20d1b5"
        mCurrentBootId = stream.readAll().trimmed().remove(QLatin1Char('-'));
    } else {
        qCWarning(journald) << "Could not obtain ID of current boot";
    }
}

LocalJournal::LocalJournal()
    : d(new LocalJournalPrivate)
{
    auto expectedJournal = owning_ptr_call<sd_journal>(sd_journal_open, SD_JOURNAL_LOCAL_ONLY);
    if (expectedJournal.ret < 0) {
        qCCritical(journald) << "Failed to open journal:" << strerror(-expectedJournal.ret);
    } else {
        d->mJournal = std::move(expectedJournal.value);
        d->mFd = sd_journal_get_fd(d->mJournal.get());
        if (d->mFd > 0) {
            d->mJournalSocketNotifier = std::make_unique<QSocketNotifier>(d->mFd, QSocketNotifier::Read);
            connect(d->mJournalSocketNotifier.get(), &QSocketNotifier::activated, this, &LocalJournal::handleJournalDescriptorUpdate);
        } else {
            qCWarning(journald) << "Could not create FD" << strerror(-d->mFd);
            d->mFd = 0;
        }
    }
}

LocalJournal::LocalJournal(const QString &path)
    : d(new LocalJournalPrivate)
{
    if (!QDir().exists(path)) {
        qCCritical(journald) << "Journal directory does not exist, abort opening" << path;
        return;
    }
    if (QFileInfo(path).isDir()) {
        auto expectedJournal = owning_ptr_call<sd_journal>(sd_journal_open_directory, path.toStdString().c_str(), 0 /* no flags, directory defines type */);
        if (expectedJournal.ret < 0) {
            qCCritical(journald) << "Could not open journal from directory" << path << ":" << strerror(-expectedJournal.ret);
        } else {
            d->mJournal = std::move(expectedJournal.value);
        }
    } else if (QFileInfo(path).isFile()) {
        const char **files = new const char *[1];
        QByteArray journalPath = path.toLocal8Bit();
        files[0] = journalPath.data();

        auto expectedJournal = owning_ptr_call<sd_journal>(sd_journal_open_files, files, 0 /* no flags, directory defines type */);
        if (expectedJournal.ret < 0) {
            qCCritical(journald) << "Could not open journal from file" << path << ":" << strerror(-expectedJournal.ret);
        } else {
            d->mJournal = std::move(expectedJournal.value);
        }
        delete[] files;
    }
}

LocalJournal::~LocalJournal() = default;

sd_journal *LocalJournal::sdJournal() const
{
    return d->mJournal.get();
}

bool LocalJournal::isValid() const
{
    return d->mJournal != nullptr;
}

QString LocalJournal::currentBootId() const
{
    return d->mCurrentBootId;
}

uint64_t LocalJournal::usage() const
{
    uint64_t size{0};
    int res = sd_journal_get_usage(d->mJournal.get(), &size);
    if (res < 0) {
        qCCritical(journald) << "Could not obtain journal size:" << strerror(-res);
    }
    return size;
}

void LocalJournal::handleJournalDescriptorUpdate()
{
    // reset descriptor
    QFile file;
    file.open(d->mFd, QIODevice::ReadOnly);
    file.readAll();
    file.close();
    qCDebug(journald()) << "Local journal FD updated";
    Q_EMIT journalUpdated(d->mCurrentBootId);
}
