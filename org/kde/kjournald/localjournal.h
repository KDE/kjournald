/*
    SPDX-License-Identifier: LGPL-2.1-or-later OR MIT
    SPDX-FileCopyrightText: 2021 Andreas Cord-Landwehr <cordlandwehr@kde.org>
*/

#ifndef LOCALJOURNAL_H
#define LOCALJOURNAL_H

#include "ijournal.h"
#include "kjournald_export.h"
#include <QString>
#include <memory>

class LocalJournalPrivate;
class sd_journal;

/**
 * @brief The LocalJournal class encapsulates a local sd_journal object
 *
 * Local journal in this sense means a journald database that can be access via direct file access, i.e.
 * where operations can be performed directly on the database files.
 *
 * @note The journald documentation specifically says that using the same sd_journal object in multiple
 * queries (or models in this case) might have side effects; even though there are none at the moment. Thus,
 * ensure that the same Journal object is only used for one model.
 */
class KJOURNALD_EXPORT LocalJournal : public IJournal
{
public:
    /**
     * @brief Construct journal object for system journald DB
     */
    explicit LocalJournal();

    /**
     * @brief Construct journal object from journald DB at path @p path
     * This path can be a directory or a file.
     */
    LocalJournal(const QString &path);

    /**
     * @brief Destroys the journal wrapper
     */
    ~LocalJournal() override;

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

private Q_SLOTS:
    void handleJournalDescriptorUpdate();

private:
    std::unique_ptr<LocalJournalPrivate> d;
};

#endif // JOURNAL_H
