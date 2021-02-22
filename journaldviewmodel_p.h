/*
    SPDX-License-Identifier: LGPL-2.1-or-later OR MIT
    SPDX-FileCopyrightText: 2021 Andreas Cord-Landwehr <cordlandwehr@kde.org>
*/

#ifndef JOURNALDVIEWMODEL_P_H
#define JOURNALDVIEWMODEL_P_H

#include <QDateTime>
#include <QString>
#include <QVector>
#include <memory>
#include <systemd/sd-journal.h>

struct LogEntry {
    QDateTime mDate;
    QString mMessage;
    QString mSystemdUnit;
    QString mBootId;
    int mPriority;
};

class JournaldViewModelPrivate
{
public:
    ~JournaldViewModelPrivate();
    void closeJournal();
    bool openJournal();
    bool openJournalFromPath(const QString &directory);

    sd_journal *mJournal{nullptr};
    QVector<LogEntry> mLog;
    QStringList mSystemdUnitFilter;
    QStringList mBootFilter;
};

#endif // JOURNALDVIEWMODEL_P_H
