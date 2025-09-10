/*
    SPDX-License-Identifier: LGPL-2.1-or-later OR MIT
    SPDX-FileCopyrightText: 2021-2025 Andreas Cord-Landwehr <cordlandwehr@kde.org>
*/

#ifndef IJOURNALPROVIDER_H
#define IJOURNALPROVIDER_H

#include "kjournald_export.h"
#include "sdjournal.h"
#include <QQmlEngine>
#include <QString>

class sd_journal;

/**
 * @brief Interface class for all journal types
 */
class KJOURNALD_EXPORT IJournalProvider : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("provider objects are backend provided")

public:
    /**
     * @brief Construct journal object for system journald DB
     */
    explicit IJournalProvider() = default;

    /**
     * @brief Destroys the journal wrapper
     */
    virtual ~IJournalProvider() = default;

    /**
     * @brief Open systemd journal for reading
     *
     * Generate an sd_journal pointer object for access to the configured journal.
     * Only configuration of the IJournalProvider derived object until accessing this method will be
     * taken into account.
     *
     * @note For simultanous access to the sd_journal from different threads or with different
     * filter matches, multiple sd_journal objects are required. See:
     * https://www.freedesktop.org/software/systemd/man/latest/sd_journal_open.html
     *
     * If error occured during opening, this method returns std::nullopt
     */
    [[nodiscard]] virtual std::unique_ptr<SdJournal> openJournal() const = 0;

    /**
     * @return ID for the current boot (b0) of the system, empty string if none is current
     */
    virtual QString currentBootId() const = 0;
};

#endif // IJOURNAL_H
