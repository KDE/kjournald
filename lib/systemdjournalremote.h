/*
    SPDX-License-Identifier: LGPL-2.1-or-later OR MIT
    SPDX-FileCopyrightText: 2021 Andreas Cord-Landwehr <cordlandwehr@kde.org>
*/

#ifndef SYSTEMDJOURNALREMOTE_H
#define SYSTEMDJOURNALREMOTE_H

#include "ijournal.h"
#include "kjournald_export.h"
#include <QProcess>
#include <QString>
#include <memory>

class SystemdJournalRemotePrivate;
class sd_journal;
class QIODevice;

/**
 * @brief The SystemdJournalRemote provides access to a remote journal via the systemd-journal-remote tool
 *
 */
class KJOURNALD_EXPORT SystemdJournalRemote : public IJournal
{
    Q_OBJECT
    Q_PROPERTY(QString journalFile READ journalFile NOTIFY journalFileChanged)
public:
    /**
     * @brief Construct journal object form file containing logs in systemd's journal export format
     */
    SystemdJournalRemote(const QString &filePath);

    /**
     * @brief Construct journal object form file containing logs in systemd's journal export format
     */
    SystemdJournalRemote(const QString &url, const QString &port);

    /**
     * @brief Destroys the journal wrapper
     */
    ~SystemdJournalRemote() override;

    /**
     * @brief Path to journal file that temporarily stores data from remote journal
     *
     * @note the lifetime of this file is bound to the lifetime of the SystemJournalRemote object that
     * relays the remote data to the file.
     * @return path to the journald ".journal" file
     */
    QString journalFile() const;

    /**
     * @brief Getter for raw sd_journal pointer
     *
     * This pointer can be nullptr if an error during opening of journal occured. Test
     * with @s isValid() before using.
     */
    sd_journal *sdJournal() const override;

    /**
     * @brief returns true if and only if the sd_journal pointer is valid
     */
    bool isValid() const override;

    /**
     * @copydoc IJournal::currentBootId()
     */
    QString currentBootId() const override;

    /**
     * @brief Get file system usage of journal
     * @return size of journal in bytes
     */
    uint64_t usage() const;

    bool isSystemdRemoteAvailable() const;

Q_SIGNALS:
    void journalFileChanged();

private Q_SLOTS:
    void handleJournalFileCreated(const QString &path);
    void handleJournalRemoteProcessErrors(QProcess::ProcessError error);

private:
    std::unique_ptr<SystemdJournalRemotePrivate> d;

    friend SystemdJournalRemotePrivate;
};

#endif
