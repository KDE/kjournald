/*
    SPDX-License-Identifier: LGPL-2.1-or-later OR MIT
    SPDX-FileCopyrightText: 2021 Andreas Cord-Landwehr <cordlandwehr@kde.org>
*/

#include "localjournal.h"
#include "kjournaldlib_log_general.h"
#include "localjournal_p.h"
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
        qCWarning(KJOURNALDLIB_GENERAL) << "Could not obtain ID of current boot";
    }
}

LocalJournal::LocalJournal(Mode mode)
    : d(new LocalJournalPrivate)
{
    d->mMode = mode;
}

LocalJournal::LocalJournal(const QString &path)
    : d(new LocalJournalPrivate)
{
    d->mPath = path;
}

LocalJournal::~LocalJournal() = default;

std::unique_ptr<SdJournal> LocalJournal::openJournal() const
{
    if (!d->mPath.isEmpty()) {
        qCDebug(KJOURNALDLIB_GENERAL) << "create sd_journal instance from path" << d->mPath;
        return std::make_unique<SdJournal>(d->mPath);
    } else {
        int flags = SD_JOURNAL_LOCAL_ONLY;
        if (d->mMode == Mode::System) {
            flags |= SD_JOURNAL_SYSTEM;
        } else {
            d->mIsUser = true;
            flags |= SD_JOURNAL_CURRENT_USER;
        }
        qCDebug(KJOURNALDLIB_GENERAL) << "create sd_journal instance from system journal" << flags;
        return std::make_unique<SdJournal>(flags);
    }
}

QString LocalJournal::currentBootId() const
{
    return d->mCurrentBootId;
}

uint64_t LocalJournal::usage() const
{
    auto journal = openJournal();
    if (!journal->isValid()) {
        return 0;
    }
    uint64_t size{0};
    int res = sd_journal_get_usage(journal->get(), &size);
    if (res < 0) {
        qCCritical(KJOURNALDLIB_GENERAL) << "Could not obtain journal size:" << strerror(-res);
    }
    return size;
}

bool LocalJournal::isUser() const
{
    return d->mIsUser;
}
