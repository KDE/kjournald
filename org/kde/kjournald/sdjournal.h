/*
    SPDX-License-Identifier: LGPL-2.1-or-later OR MIT
    SPDX-FileCopyrightText: 2025 Andreas Cord-Landwehr <cordlandwehr@kde.org>
*/

#ifndef SDJOURNAL_H
#define SDJOURNAL_H

#include "kjournald_export.h"
#include "memory.h"
#include <QSocketNotifier>
#include <systemd/sd-journal.h>

class KJOURNALD_EXPORT SdJournal : public QObject
{
    Q_OBJECT
public:
    explicit SdJournal(const QString &path, int flags = 0);
    explicit SdJournal(int flags = 0);
    ~SdJournal() override;
    sd_journal *get();

    /**
     * @brief returns true if and only if the sd_journal pointer is valid
     *
     * This method shall be used to check of the journal is ready to be used
     */
    bool isValid() const;

    static bool areJournalFilesAvailable(const QString &path);

private Q_SLOTS:
    void handleFdUpdate();

Q_SIGNALS:
    /**
     * @brief signal is fired when new entries are added to the journal
     */
    void journalUpdated();

private:
    std::unique_ptr<sd_journal> mJournal;
    qintptr mFd{0};
    std::unique_ptr<QSocketNotifier> mJournalSocketNotifier;
};

#endif // SDJOURNAL_H
