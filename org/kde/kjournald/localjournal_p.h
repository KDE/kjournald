/*
    SPDX-License-Identifier: LGPL-2.1-or-later OR MIT
    SPDX-FileCopyrightText: 2021 Andreas Cord-Landwehr <cordlandwehr@kde.org>
*/

#ifndef LOCALJOURNAL_PRIVATE_H
#define LOCALJOURNAL_PRIVATE_H

#include "localjournal.h"
#include <QSocketNotifier>
#include <QtGlobal>
#include <systemd/sd-journal.h>

class LocalJournalPrivate
{
public:
    LocalJournalPrivate();
    QString mPath;
    LocalJournal::Mode mMode{LocalJournal::Mode::System};
    bool mIsUser = false;
    QString mCurrentBootId;
    std::unique_ptr<QSocketNotifier> mJournalSocketNotifier;
};

#endif // LOCALJOURNAL_H
