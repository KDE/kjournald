/*
    SPDX-License-Identifier: LGPL-2.1-or-later OR MIT
    SPDX-FileCopyrightText: 2021 Andreas Cord-Landwehr <cordlandwehr@kde.org>
*/

#ifndef JOURNAL_H
#define JOURNAL_H

#include <QString>
#include <memory>

class JournalPrivate;
class sd_journal;

class Journal
{
public:
    /**
     * @brief Construct journal object for system journald DB
     */
    explicit Journal();

    /**
     * @brief Construct journal object from journald DB at path @p path
     */
    Journal(const QString &path);

    /**
     * @brief Destroys the journal wrapper
     */
    ~Journal();

    /**
     * @brief Getter for raw sd_journal pointer
     *
     * This pointer can be nullptr if an error during opening of journal occured. Test
     * with @s isValid() before using.
     */
    sd_journal *sdJournal() const;

    /**
     * @brief returns true if and only if the sd_journal pointer is valid
     */
    bool isValid() const;

private:
    std::unique_ptr<JournalPrivate> d;
};

#endif // JOURNAL_H
