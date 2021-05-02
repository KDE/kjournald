/*
    SPDX-License-Identifier: LGPL-2.1-or-later OR MIT
    SPDX-FileCopyrightText: 2021 Andreas Cord-Landwehr <cordlandwehr@kde.org>
*/

#ifndef IJOURNAL_H
#define IJOURNAL_H

#include "kjournald_export.h"
#include <QObject>
#include <QString>

class sd_journal;

/**
 * @brief Interface class for all journal types
 */
class KJOURNALD_EXPORT IJournal : public QObject
{
    Q_OBJECT
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
     *
     * This method shall be used to check of the journal is ready to be used
     */
    virtual bool isValid() const = 0;

Q_SIGNALS:
    /**
     * @brief signal is fired when new entries are added to the journal
     * @param bootId the ID for the boot to which entries are added
     */
    void journalUpdated(const QString &bootId);
};

#endif // IJOURNAL_H
