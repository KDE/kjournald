/*
    SPDX-License-Identifier: LGPL-2.1-or-later OR MIT
    SPDX-FileCopyrightText: 2021 Andreas Cord-Landwehr <cordlandwehr@kde.org>
*/

#ifndef JOURNALDVIEWMODEL_P_H
#define JOURNALDVIEWMODEL_P_H

#include "ijournal.h"
#include <QAtomicInt>
#include <QColor>
#include <QDateTime>
#include <QHash>
#include <QRandomGenerator>
#include <QString>
#include <QVector>
#include <memory>
#include <optional>
#include <systemd/sd-journal.h>

struct LogEntry {
    QDateTime mDate;
    quint64 mMonotonicTimestamp{0};
    QString mId;
    QString mMessage;
    QString mSystemdUnit;
    QString mBootId;
    QString mExe;
    int mPriority{0};
    QString mCursor;
};

class JournaldViewModelPrivate
{
public:
    enum class Direction {
        TOWARDS_HEAD,
        TOWARDS_TAIL,
    };
    enum class COLOR_TYPE {
        LOG_ENTRY,
        UNIT,
    };

    QColor unitColor(const QString &unit, COLOR_TYPE = COLOR_TYPE::LOG_ENTRY);
    /**
     * reapply all filters and seek journal at head
     * ensure to guard this call with beginModelReset and endModelReset
     */
    void resetJournal();

    /**
     * Seek head of journal and already position at first entry with
     * sd_journal_next().
     *
     * @note this call also updates all internal cursors (for window head/tail) as well
     * as the internal state if head/tail are reached.
     */
    void seekHeadAndMakeCurrent();

    /**
     * Seek tail of journal and already position at first entry with
     * sd_journal_next().
     *
     * @note this call also updates all internal cursors (for window head/tail) as well
     * as the internal state if head/tail are reached.
     */
    void seekTailAndMakeCurrent();

    /**
     * fetch data from current cursor position in forwards direction if @p forwards
     * is true, otherwards backwards in time
     * @note depending on the direction, the method relies on correctly initialized head and tail
     * cursors and upon calling sets the current entry to the respective cursor.
     *
     * @note it is responsibility of the caller to ensure that data entries are not
     * placed twice into the journal. this means, only call this method after a model
     * reset and then only in the respective direction
     */
    QVector<LogEntry> readEntries(Direction direction);

    std::unique_ptr<IJournal> mJournal;
    QVector<LogEntry> mLog;
    QStringList mSystemdUnitFilter;
    QStringList mExeFilter;
    QStringList mBootFilter;
    std::optional<int> mPriorityFilter;
    bool mShowKernelMessages{false};
    QHash<QString, std::pair<QColor, QColor>> mUnitToColorMap;
    bool mHeadCursorReached{false};
    bool mTailCursorReached{false};
    QAtomicInt mActiveFetchOperations{0};

    static QRandomGenerator sFixedSeedGenerator;
};

#endif // JOURNALDVIEWMODEL_P_H
