/*
    SPDX-License-Identifier: LGPL-2.1-or-later OR MIT
    SPDX-FileCopyrightText: 2021 Andreas Cord-Landwehr <cordlandwehr@kde.org>
*/

#ifndef JOURNAL_H
#define JOURNAL_H

#include <QString>
#include <memory>
#include "ijournal.h"
#include "kjournald_export.h"

class JournalPrivate;
class sd_journal;

/**
 * @brief The Journal class encapsulates an sd_journal object
 *
 * @note The journald documentation specifically says that using the same sd_journal object in multiple
 * queries (or models in this case) might have side effects; even though there are none at the moment. Thus,
 * ensure that the same Journal object is only used for one model.
 */
class KJOURNALD_EXPORT Journal : public IJournal
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
    ~Journal() override;

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

private:
    std::unique_ptr<JournalPrivate> d;
};

#endif // JOURNAL_H
