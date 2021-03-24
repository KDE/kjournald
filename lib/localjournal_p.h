/*
    SPDX-License-Identifier: LGPL-2.1-or-later OR MIT
    SPDX-FileCopyrightText: 2021 Andreas Cord-Landwehr <cordlandwehr@kde.org>
*/

#ifndef LOCALJOURNAL_PRIVATE_H
#define LOCALJOURNAL_PRIVATE_H

#include <systemd/sd-journal.h>

class LocalJournalPrivate
{
public:
    mutable sd_journal *mJournal{nullptr};
};

#endif // LOCALJOURNAL_H
