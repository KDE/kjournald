/*
    SPDX-License-Identifier: LGPL-2.1-or-later OR MIT
    SPDX-FileCopyrightText: 2021 Andreas Cord-Landwehr <cordlandwehr@kde.org>
*/

#ifndef IJOURNAL_H
#define IJOURNAL_H

#include "kjournald_export.h"
#include <QString>

class sd_journal;

/**
 * @brief Interface class for passing journal do model
 */
class KJOURNALD_EXPORT IJournal
{
public:
    /**
     * @brief Construct journal object for system journald DB
     */
    explicit IJournal() = default;

    /**
     * @brief Destroys the journal wrapper
     */
    virtual ~IJournal() = default;

    /**
     * @brief Getter for raw sd_journ´al pointer
     *
     * This pointer can be nullptr if an error during opening of journal occured. Test
     * with @s isValid() before using.
     */
    virtual sd_journal *sdJournal() const = 0;

    /**
     * @brief returns true if and only if the sd_journal pointer is valid
     */
    virtual bool isValid() const = 0;
};

#endif // IJOURNAL_H