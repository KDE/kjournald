/*
    SPDX-License-Identifier: LGPL-2.1-or-later OR MIT
    SPDX-FileCopyrightText: 2021 Andreas Cord-Landwehr <cordlandwehr@kde.org>
*/

#ifndef JOURNAL_PRIVATE_H
#define JOURNAL_PRIVATE_H

#include <systemd/sd-journal.h>

class JournalPrivate
{
public:
    mutable sd_journal *mJournal{ nullptr };
};

#endif // JOURNAL_H
