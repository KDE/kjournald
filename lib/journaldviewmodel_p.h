/*
    SPDX-License-Identifier: LGPL-2.1-or-later OR MIT
    SPDX-FileCopyrightText: 2021 Andreas Cord-Landwehr <cordlandwehr@kde.org>
*/

#ifndef JOURNALDVIEWMODEL_P_H
#define JOURNALDVIEWMODEL_P_H

#include <QDateTime>
#include <QString>
#include <QVector>
#include <QHash>
#include <QColor>
#include <memory>
#include <optional>
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
    QColor unitColor(const QString &unit);
    void seekHead();

    bool canFetchMore{ true }; // indicates if end of journal is reached
    QString mJournalPath;
    sd_journal *mJournal{ nullptr };
    QVector<LogEntry> mLog;
    QStringList mSystemdUnitFilter;
    QStringList mBootFilter;
    std::optional<int> mPriorityFilter;
    bool mShowKernelMessages{ false };
    QHash<QString, QColor> mUnitToColorMap;
};

#endif // JOURNALDVIEWMODEL_P_H
