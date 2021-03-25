/*
    SPDX-License-Identifier: LGPL-2.1-or-later OR MIT
    SPDX-FileCopyrightText: 2021 Andreas Cord-Landwehr <cordlandwehr@kde.org>
*/

#ifndef LOCALJOURNAL_PRIVATE_H
#define LOCALJOURNAL_PRIVATE_H

#include <QSocketNotifier>
#include <QtGlobal>
#include <memory>
#include <systemd/sd-journal.h>

class LocalJournalPrivate
{
public:
    mutable sd_journal *mJournal{nullptr};
    qintptr mFd{0};
    std::unique_ptr<QSocketNotifier> mJournalSocketNotifier;
};

#endif // LOCALJOURNAL_H
