/*
    SPDX-License-Identifier: LGPL-2.1-or-later OR MIT
    SPDX-FileCopyrightText: 2021 Andreas Cord-Landwehr <cordlandwehr@kde.org>
*/

#ifndef JOURNALDVIEWMODEL_P_H
#define JOURNALDVIEWMODEL_P_H

#include "filter.h"
#include "ijournal.h"
#include "logentry.h"
#include <QAtomicInt>
#include <QColor>
#include <QDateTime>
#include <QHash>
#include <QString>
#include <QVector>
#include <memory>
#include <systemd/sd-journal.h>

class JournaldViewModelPrivate
{
public:
    enum class Direction {
        TOWARDS_HEAD,
        TOWARDS_TAIL,
    };

    enum class SeekCursorResult {
        CURSOR_MADE_CURRENT,
        ERROR,
    };

    /**
     * reapply all filters and seek journal at head
     * ensure to guard this call with beginModelReset and endModelReset
     */
    void resetJournal();

    /**
     * Seek head of journal and already position at first entry with
     * sd_journal_next().
     *
     * @return if head could be seeked (e.g. false if filter result to empty set)
     *
     * @note this call also updates all internal cursors (for window head/tail) as well
     * as the internal state if head/tail are reached.
     */
    bool seekHeadAndMakeCurrent();

    /**
     * Seek tail of journal and already position at first entry with
     * sd_journal_next().
     *
     * @note this call also updates all internal cursors (for window head/tail) as well
     * as the internal state if head/tail are reached.
     */
    bool seekTailAndMakeCurrent();

    /**
     * fetch data from current cursor position in forwards direction if @p forwards
     * is true, otherwards backwards in time
     * @note depending on the direction, the method relies on correctly initialized head and tail
     * cursors and upon calling sets the current entry to the respective cursor.
     *
     * @return if tail could be seeked (e.g. false if filter result to empty set)
     *
     * @note it is responsibility of the caller to ensure that data entries are not
     * placed twice into the journal. this means, only call this method after a model
     * reset and then only in the respective direction
     */
    QVector<LogEntry> readEntries(Direction direction);

    /**
     * @brief seekCursor in journal an handle issues
     * @return
     */
    SeekCursorResult seekCursor(const QString &cursor);

    std::unique_ptr<IJournal> mJournal;
    QList<LogEntry> mLog;
    Filter mFilter;
    bool mHeadCursorReached{false};
    bool mTailCursorReached{false};
    bool mModelResetActive{false};
    QAtomicInt mActiveFetchOperations{0};
    uint32_t mChunkSize{500};
};

#endif // JOURNALDVIEWMODEL_P_H
