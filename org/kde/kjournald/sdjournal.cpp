/*
    SPDX-License-Identifier: LGPL-2.1-or-later OR MIT
    SPDX-FileCopyrightText: 2025 Andreas Cord-Landwehr <cordlandwehr@kde.org>
*/

#include "sdjournal.h"
#include "kjournaldlib_log_general.h"
#include <QDir>
#include <QLatin1StringView>

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
        } else if (areJournalFilesAvailable(path)) {
            mJournal = std::move(expectedJournal.value);
        } else {
            qCritical() << "Plausibility parser did not detect any journald db file at location";
            return;
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
    if (mFd == 0) {
        return;
    }
    // reset descriptor
    QFile file;
    if (file.open(mFd, QIODevice::ReadOnly)) {
        file.readAll();
        file.close();
    } else {
        qCWarning(KJOURNALDLIB_GENERAL) << "Could not open journal FD";
    }
    qCDebug(KJOURNALDLIB_GENERAL) << "Local journal FD updated";
    Q_EMIT journalUpdated();
}

bool SdJournal::areJournalFilesAvailable(const QString &path)
{
    // this method iterates all files under given path and does plausibility check for available files
    // even though https://systemd.io/JOURNAL_FILE_FORMAT/ says that data format is not stable,
    // we expect the very basic setup to not change any time soon; also, no existing sd-journal API provides
    // information about available journal files as of systemd v258

    // implement logic of sd_journal_open_directory() which looks two folder levels deep
    const QDir firstDir = QDir(path);
    QFileInfoList files;
    if (firstDir.exists()) {
        files << firstDir.entryInfoList({QLatin1String{"*.journal"}}, QDir::Filter::Files);
        const auto subDirs = firstDir.entryInfoList(QDir::Filter::Dirs | QDir::Filter::NoDotAndDotDot);
        for (const auto &subDirInfo : subDirs) {
            const QDir subDir(subDirInfo.absoluteFilePath());
            files << subDir.entryInfoList({QLatin1String{"*.journal"}}, QDir::Filter::Files);
        }
    }

    for (const auto &fileInfo : std::as_const(files)) {
        // first 8 bytes of those journal files must start with specific ASCII identifier
        QFile file{fileInfo.absoluteFilePath()};
        if (file.open(QIODevice::ReadOnly)) {
            if (QLatin1StringView(file.read(8)) == QLatin1StringView{"LPKSHHRH"}) {
                return true;
            }
        }
    }
    return false;
}

#include "moc_sdjournal.cpp"
