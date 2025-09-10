/*
    SPDX-License-Identifier: LGPL-2.1-or-later OR MIT
    SPDX-FileCopyrightText: 2025 Andreas Cord-Landwehr <cordlandwehr@kde.org>
*/

#include "sdjournal.h"
#include "kjournaldlib_log_general.h"
#include <QDir>

SdJournal::SdJournal(const QString &path, int flags)
{
    if (!QDir().exists(path)) {
        qCCritical(KJOURNALDLIB_GENERAL) << "Journal directory does not exist, abort opening" << path;
        return;
    }
    if (QFileInfo(path).isDir()) {
        auto expectedJournal = owning_ptr_call<sd_journal>(sd_journal_open_directory, path.toStdString().c_str(), flags);
        if (expectedJournal.ret < 0) {
            qCCritical(KJOURNALDLIB_GENERAL) << "Could not open journal from directory" << path << ":" << strerror(-expectedJournal.ret);
        } else {
            mJournal = std::move(expectedJournal.value);
        }
    } else if (QFileInfo(path).isFile()) {
        const char **files = new const char *[1];
        QByteArray journalPath = path.toLocal8Bit();
        files[0] = journalPath.data();

        auto expectedJournal = owning_ptr_call<sd_journal>(sd_journal_open_files, files, flags);
        if (expectedJournal.ret < 0) {
            qCCritical(KJOURNALDLIB_GENERAL) << "Could not open journal from file" << path << ":" << strerror(-expectedJournal.ret);
        } else {
            mJournal = std::move(expectedJournal.value);
        }
        delete[] files;
    }
}

SdJournal::SdJournal(int flags)
{
    auto expectedJournal = owning_ptr_call<sd_journal>(sd_journal_open, flags);
    if (expectedJournal.ret < 0) {
        qCCritical(KJOURNALDLIB_GENERAL) << "Failed to open journal:" << strerror(-expectedJournal.ret);
    } else {
        mJournal = std::move(expectedJournal.value);
        mFd = sd_journal_get_fd(mJournal.get());
        if (mFd > 0) {
            mJournalSocketNotifier = std::make_unique<QSocketNotifier>(mFd, QSocketNotifier::Read);
            connect(mJournalSocketNotifier.get(), &QSocketNotifier::activated, this, &SdJournal::handleFdUpdate);
        } else {
            qCWarning(KJOURNALDLIB_GENERAL) << "Could not create FD" << strerror(-mFd);
            mFd = 0;
        }
    }
}

SdJournal::~SdJournal() = default;

sd_journal *SdJournal::get()
{
    return mJournal.get();
}

bool SdJournal::isValid() const
{
    return mJournal != nullptr;
}

void SdJournal::handleFdUpdate()
{
    // reset descriptor
    QFile file;
    file.open(mFd, QIODevice::ReadOnly);
    file.readAll();
    file.close();
    qCDebug(KJOURNALDLIB_GENERAL) << "Local journal FD updated";
    Q_EMIT journalUpdated();
}
