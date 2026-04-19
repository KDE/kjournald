/*
    SPDX-License-Identifier: LGPL-2.1-or-later OR MIT
    SPDX-FileCopyrightText: 2021 Andreas Cord-Landwehr <cordlandwehr@kde.org>
*/

#include "journaldviewmodel.h"
#include "colorizer.h"
#include "journaldhelper.h"
#include "journaldviewmodel_p.h"
#include "kjournaldlib_log_filtertrace.h"
#include "kjournaldlib_log_general.h"
#include "logentry.h"
#include <QColor>
#include <QDebug>
#include <QDir>
#include <QMutex>
#include <QRandomGenerator>
#include <QRegularExpression>
#include <QThread>
#include <algorithm>
#include <iterator>

void JournaldViewModelPrivate::resetJournal()
{
    if (!mJournal || !mJournal->isValid()) {
        qCWarning(KJOURNALDLIB_GENERAL) << "Skipping reset, no valid journal open";
        return;
    }

    int result{0};

    // reset all filters
    sd_journal_flush_matches(mJournal->get());

    qCDebug(KJOURNALDLIB_FILTERTRACE) << "flush_matches()";

    auto addConjunction = [](sd_journal *journal) -> void {
        int result{0};
        result = sd_journal_add_conjunction(journal);
        if (result < 0) {
            qCCritical(KJOURNALDLIB_FILTERTRACE).nospace() << "add_conjunction returned error";
        }
        Q_ASSERT(result >= 0);
    };

    auto addDisjunction = [](sd_journal *journal) -> void {
        int result{0};
        result = sd_journal_add_disjunction(journal);
        if (result < 0) {
            qCCritical(KJOURNALDLIB_FILTERTRACE).nospace() << "add_disjunction returned error";
        }
        Q_ASSERT(result >= 0);
    };

    auto addMatchesBootFilter = [](sd_journal *journal, const QStringList &boots) -> void {
        int result{0};
        for (const QString &boot : boots) {
            QString filterExpression = QLatin1String("_BOOT_ID=") + boot;
            result = sd_journal_add_match(journal, filterExpression.toUtf8().constData(), 0);
            qCDebug(KJOURNALDLIB_FILTERTRACE).nospace() << "add_match(" << filterExpression << ")";
            if (result < 0) {
                qCCritical(KJOURNALDLIB_GENERAL) << "Failed to set journal filter:" << strerror(-result) << filterExpression;
            }
        }
    };

    auto addMatchesPriorityFilter = [](sd_journal *journal, std::optional<quint8> priorityLimit) -> void {
        int result{0};
        if (priorityLimit.has_value()) {
            for (int i = 0; i <= *priorityLimit; ++i) {
                QString filterExpression = QLatin1String("PRIORITY=") + QString::number(i);
                result = sd_journal_add_match(journal, filterExpression.toUtf8().constData(), 0);
                qCDebug(KJOURNALDLIB_FILTERTRACE).nospace() << "add_match(" << filterExpression << ")";
                if (result < 0) {
                    qCCritical(KJOURNALDLIB_GENERAL) << "Failed to set journal filter:" << strerror(-result) << filterExpression;
                }
            }
            qCDebug(KJOURNALDLIB_GENERAL) << "Use priority filter level:" << *priorityLimit;
        } else {
            qCDebug(KJOURNALDLIB_GENERAL) << "Skip setting priority filter";
        }
    };

    auto addMatchesUserUnitFilter = [](sd_journal *journal, const QStringList &units) -> void {
        int result{0};
        for (const QString &unit : units) {
            QString filterExpression = QLatin1String("_SYSTEMD_USER_UNIT=") + unit;
            result = sd_journal_add_match(journal, filterExpression.toUtf8().constData(), 0);
            qCDebug(KJOURNALDLIB_FILTERTRACE).nospace() << "add_match(" << filterExpression << ")";
            if (result < 0) {
                qCCritical(KJOURNALDLIB_GENERAL) << "Failed to set journal filter:" << strerror(-result) << filterExpression;
            }
        }
    };

    auto addMatchesSystemUnitFilter = [](sd_journal *journal, const QStringList &units) -> void {
        int result{0};
        for (const QString &unit : units) {
            QString filterExpression = QLatin1String("_SYSTEMD_UNIT=") + unit;
            result = sd_journal_add_match(journal, filterExpression.toUtf8().constData(), 0);
            qCDebug(KJOURNALDLIB_FILTERTRACE).nospace() << "add_match(" << filterExpression << ")";
            if (result < 0) {
                qCCritical(KJOURNALDLIB_GENERAL) << "Failed to set journal filter:" << strerror(-result) << filterExpression;
            }
        }
    };

    auto addMatchesExeFilter = [](sd_journal *journal, const QStringList &exes) -> void {
        int result{0};
        for (const QString &executable : exes) {
            QString filterExpression = QLatin1String("_EXE=") + executable;
            result = sd_journal_add_match(journal, filterExpression.toUtf8().constData(), 0);
            qCDebug(KJOURNALDLIB_FILTERTRACE).nospace() << "add_match(" << filterExpression << ")";
            if (result < 0) {
                qCCritical(KJOURNALDLIB_GENERAL) << "Failed to set journal filter:" << strerror(-result) << filterExpression;
            }
        }
    };

    auto addMatchesTransportFilter = [](sd_journal *journal, const QStringList &transports) -> void {
        int result{0};
        for (const QString &transport : transports) {
            QString filterExpression = QLatin1String("_TRANSPORT=") + transport;
            result = sd_journal_add_match(journal, filterExpression.toUtf8().constData(), 0);
            qCDebug(KJOURNALDLIB_FILTERTRACE).nospace() << "add_match(" << filterExpression << ")";
            if (result < 0) {
                qCCritical(KJOURNALDLIB_GENERAL) << "Failed to set journal filter:" << strerror(-result) << filterExpression;
            }
        }
    };

    const QStringList kernelTransports{QLatin1String("audit"), QLatin1String("driver"), QLatin1String("kernel")};
    const QStringList nonKernelTransports{QLatin1String("syslog"), QLatin1String("journal"), QLatin1String("stdout")};

    // filter construction:
    // The Journald API does not provide arbitrary logical phrases, but a 4 level syntax,
    // see: https://www.freedesktop.org/software/systemd/man/sd_journal_add_match.html
    // 1. level: AND, via add_conjunction (separates terms via AND)
    // 2. level: OR, via add_disjunction (separates terms via OR or AND)
    // 3: level: AND, via multiple add_match(...) in one term with different fields that are considered as AND combination
    // 4: level: OR, via multiple add_match(...) in one term with same field that are considered as OR combination
    //
    // The following boolean expression is created as follow for kernel transport option:
    //     (boot=123 OR boot=...)
    //     AND (priority=1 OR priority=...)
    //     AND (transport=kernel)
    // OR
    //     (boot=123 OR boot=...)
    //     AND (priority=1 OR priority=...)
    //     AND (unit_1 OR unit_2 OR ...)
    // OR
    //     (boot=123 OR boot=...)
    //     AND (priority=1 OR priority=...)
    //     AND (exe=x OR exe=y OR ...)

    bool clauseAdded{false};
    // kernel filter is special in the sense that thouse message only shall be added
    // and in the absense of different category filters, a filtering of the correct
    // transport layer must be applied
    if (mFilter.areKernelMessagesEnabled()) {
        clauseAdded = true;
        addMatchesBootFilter(mJournal->get(), mFilter.bootFilter());
        addMatchesPriorityFilter(mJournal->get(), mFilter.priorityFilter());
        QStringList transportFilter = kernelTransports;
        if (mFilter.systemdUserUnitFilter().empty() && mFilter.systemdSystemUnitFilter().empty() && mFilter.exeFilter().empty()) {
            transportFilter.append(nonKernelTransports);
        }
        addMatchesTransportFilter(mJournal->get(), transportFilter);
    } else if (mFilter.systemdUserUnitFilter().empty() && mFilter.systemdSystemUnitFilter().empty() && mFilter.exeFilter().empty()) {
        clauseAdded = true;
        addMatchesBootFilter(mJournal->get(), mFilter.bootFilter());
        addMatchesPriorityFilter(mJournal->get(), mFilter.priorityFilter());
        addMatchesTransportFilter(mJournal->get(), nonKernelTransports);
    }
    if (clauseAdded && !mFilter.systemdUserUnitFilter().empty()) {
        addDisjunction(mJournal->get());
        clauseAdded = false;
    }
    if (!mFilter.systemdUserUnitFilter().empty()) {
        clauseAdded = true;
        addMatchesBootFilter(mJournal->get(), mFilter.bootFilter());
        addMatchesPriorityFilter(mJournal->get(), mFilter.priorityFilter());
        addMatchesUserUnitFilter(mJournal->get(), mFilter.systemdUserUnitFilter());
    }
    if (clauseAdded && !mFilter.systemdSystemUnitFilter().empty()) {
        addDisjunction(mJournal->get());
        clauseAdded = false;
    }
    if (!mFilter.systemdSystemUnitFilter().empty()) {
        clauseAdded = true;
        addMatchesBootFilter(mJournal->get(), mFilter.bootFilter());
        addMatchesPriorityFilter(mJournal->get(), mFilter.priorityFilter());
        addMatchesSystemUnitFilter(mJournal->get(), mFilter.systemdSystemUnitFilter());
    }
    if (clauseAdded && !mFilter.exeFilter().empty()) {
        addDisjunction(mJournal->get());
    }
    if (!mFilter.exeFilter().empty()) {
        addMatchesBootFilter(mJournal->get(), mFilter.bootFilter());
        addMatchesPriorityFilter(mJournal->get(), mFilter.priorityFilter());
        addMatchesExeFilter(mJournal->get(), mFilter.exeFilter());
    }

    qCDebug(KJOURNALDLIB_FILTERTRACE).nospace() << "Filter DONE";
    mTailCursorReached = false;
    seekHeadAndMakeCurrent();
    // clear all data which are in limbo with new head
    mLog.clear();
}

QList<LogEntry> JournaldViewModelPrivate::readEntries(Direction direction)
{
    if (!mJournal || !mJournal->isValid()) {
        qCWarning(KJOURNALDLIB_GENERAL) << "Skipping read entries, no valid journal open";
        return {};
    }

    static QMutex mutex;
    QMutexLocker locker(&mutex);

    QList<LogEntry> chunk;
    chunk.reserve(mChunkSize);

    // cursor position
    if (!mLog.isEmpty()) {
        QStringView cursor = (direction == Direction::TOWARDS_TAIL) ? mLog.last().cursor() : mLog.first().cursor();

        switch (seekCursor(cursor)) {
        case SeekCursorResult::CURSOR_MADE_CURRENT:
            if ((direction == Direction::TOWARDS_TAIL && sd_journal_next(mJournal->get()) == 0)
                || (direction == Direction::TOWARDS_HEAD && sd_journal_previous(mJournal->get()) == 0)) {
                if (direction == Direction::TOWARDS_TAIL) {
                    mTailCursorReached = true;
                } else {
                    mHeadCursorReached = true;
                }
                return {};
            }
            break;

        case SeekCursorResult::ERROR:
            qCCritical(KJOURNALDLIB_GENERAL) << "cursor test failed";
            return {};
        }
    } else {
        if ((direction == Direction::TOWARDS_TAIL && !seekHeadAndMakeCurrent()) || (direction == Direction::TOWARDS_HEAD && !seekTailAndMakeCurrent())) {
            return {};
        }
    }

    for (int i = 0; i < mChunkSize; ++i) {
        LogEntry entry;

        // read timestamps
        uint64_t time;
        if (sd_journal_get_realtime_usec(mJournal->get(), &time) == 0) {
            entry.setDate(QDateTime::fromMSecsSinceEpoch(time / 1000, QTimeZone::UTC));
        }

        sd_id128_t bootId;
        if (sd_journal_get_monotonic_usec(mJournal->get(), &time, &bootId) == 0) {
            entry.setMonotonicTimestamp(time);
        }

        // helpers for fast extraction of VALUE from "KEY=VALUE"
        auto extractValue = [](const void *data, size_t length) -> QString {
            const char *ptr = static_cast<const char *>(data);
            const char *eq = static_cast<const char *>(memchr(ptr, '=', length));
            if (!eq) {
                return QString();
            }
            return QString::fromUtf8(eq + 1, ptr + length - (eq + 1));
        };
        const void *data;
        size_t length;
        auto getField = [&](const char *name) -> QString {
            if (sd_journal_get_data(mJournal->get(), name, &data, &length) == 0) {
                return extractValue(data, length);
            }
            return {};
        };

        entry.setMessage(getField("MESSAGE"));
        entry.setId(getField("MESSAGE_ID"));
        entry.setBootId(getField("_BOOT_ID"));
        entry.setExe(getField("_EXE"));

        const QString priority = getField("PRIORITY");
        if (!priority.isEmpty()) {
            entry.setPriority(priority.toInt());
        }

        QString unit = getField("_SYSTEMD_USER_UNIT");
        if (unit.isEmpty()) {
            unit = getField("_SYSTEMD_UNIT");
        }

        if (!unit.isEmpty()) {
            unit = JournaldHelper::cleanupString(unit);
            entry.setUnit(unit);

            qsizetype at = unit.indexOf(QLatin1Char('@'));
            qsizetype dot = unit.lastIndexOf(QLatin1String(".service"));
            if (at != -1 && dot > at) {
                unit.replace(at, dot - at, QLatin1String("@"));
            }
            entry.setUnitTemplateGroup(unit);
        }

        // cursor
        char *cursor = nullptr;
        if (sd_journal_get_cursor(mJournal->get(), &cursor) == 0) {
            entry.setCursor(QString::fromUtf8(cursor));
            free(cursor);
        }

        chunk.append(std::move(entry)); // always append

        // advance journal
        int r = (direction == Direction::TOWARDS_TAIL) ? sd_journal_next(mJournal->get()) : sd_journal_previous(mJournal->get());

        if (r == 0) {
            if (direction == Direction::TOWARDS_TAIL)
                mTailCursorReached = true;
            else
                mHeadCursorReached = true;
            break;
        }
    }

    // reverse once instead of many prepends
    if (direction == Direction::TOWARDS_HEAD) {
        std::reverse(chunk.begin(), chunk.end());
    }

    return chunk;
}

JournaldViewModelPrivate::SeekCursorResult JournaldViewModelPrivate::seekCursor(QStringView cursor)
{
    int result{0};
    // note: seek cursor does not make it current, but a subsequent sd_journal_next is required
    result = sd_journal_seek_cursor(mJournal->get(), cursor.toUtf8().constData());
    if (result < 0) {
        qCWarning(KJOURNALDLIB_GENERAL) << "seeking cursor but could not be found" << strerror(-result);
        return SeekCursorResult::ERROR;
    }

    // make entry current
    // every result other than 1 is an error, because the cursor is expected to exist
    result = sd_journal_next(mJournal->get());
    if (result != 1) {
        qCCritical(KJOURNALDLIB_GENERAL) << "seeked entry for cursor could not be made current";
        return JournaldViewModelPrivate::SeekCursorResult::ERROR;
    }

    // fallback search logic for problematic versions of systemd: https://github.com/systemd/systemd/issues/31516
    // note the linear search is very expensive and should be the exception
    result = sd_journal_test_cursor(mJournal->get(), cursor.toUtf8().constData());
    if (result > 0) {
        return SeekCursorResult::CURSOR_MADE_CURRENT;
    } else {
        qCWarning(KJOURNALDLIB_GENERAL) << "current position does not match expected cursor, entering expensive linear search";
        //(requested, actual):" << cursor << actualCursor;
        sd_journal_seek_head(mJournal->get());
        while (sd_journal_next(mJournal->get()) > 0) {
            char *actualCursor{nullptr};
            const int actualCursorResult = sd_journal_get_cursor(mJournal->get(), &actualCursor);
            free(actualCursor);
            if (actualCursorResult >= 0) {
                return SeekCursorResult::CURSOR_MADE_CURRENT;
            }
        }
        qCCritical(KJOURNALDLIB_GENERAL) << "even with linear search, the cursor could not be found, giving up";
    }
    return SeekCursorResult::ERROR;
}

bool JournaldViewModelPrivate::seekHeadAndMakeCurrent()
{
    qCDebug(KJOURNALDLIB_GENERAL) << "seek head and make current";
    int result = sd_journal_seek_head(mJournal->get());
    if (result < 0) {
        qCCritical(KJOURNALDLIB_GENERAL) << "Failed to seek head:" << strerror(-result);
        return false;
    }
    if (sd_journal_next(mJournal->get()) <= 0) {
        qCWarning(KJOURNALDLIB_GENERAL) << "could not make head entry current";
        return false;
    }
    mHeadCursorReached = true;
    return true;
}

bool JournaldViewModelPrivate::seekTailAndMakeCurrent()
{
    qCDebug(KJOURNALDLIB_GENERAL) << "seek tail and make current";
    int result = sd_journal_seek_tail(mJournal->get());
    if (result < 0) {
        qCCritical(KJOURNALDLIB_GENERAL) << "Failed to seek head:" << strerror(-result);
        return false;
    }
    if (sd_journal_previous(mJournal->get()) <= 0) {
        qCWarning(KJOURNALDLIB_GENERAL) << "could not make tail entry current";
        return false;
    }
    mTailCursorReached = true;
    return true;
}

JournaldViewModel::JournaldViewModel(QObject *parent)
    : QAbstractItemModel(parent)
    , d(new JournaldViewModelPrivate)
{
}

JournaldViewModel::~JournaldViewModel() = default;

void JournaldViewModel::guardedBeginResetModel()
{
    Q_ASSERT_X(d->mModelResetActive == false, "JournaldViewModel::guardedBeginResetModel", "d->mModelResetActive==true");
    d->mModelResetActive = true;
    beginResetModel();
}

void JournaldViewModel::guardedEndResetModel()
{
    endResetModel();
    Q_ASSERT_X(d->mModelResetActive == true, "JournaldViewModel::guardedEndResetModel", "d->mModelResetActive==false");
    d->mModelResetActive = false;
}

void JournaldViewModel::setJournalProvider(IJournalProvider *provider)
{
    d->mJournalProvider = provider;
    Q_EMIT journalProviderChanged();

    guardedBeginResetModel();
    d->mLog.clear();
    if (provider) {
        d->mJournal = provider->openJournal();
    }
    d->mJournalAvailable = provider && d->mJournal && d->mJournal->isValid();
    if (d->mJournalAvailable) {
        d->resetJournal();
    }
    guardedEndResetModel();
    if (d->mJournalAvailable) {
        fetchMoreLogEntries();
        connect(d->mJournal.get(), &SdJournal::journalUpdated, this, [=]() {
            if (d->mTailCursorReached) {
                d->mTailCursorReached = false;
                fetchMoreLogEntries();
            }
        });
    }
    Q_EMIT availableChanged();
}

IJournalProvider *JournaldViewModel::journalProvider() const
{
    return d->mJournalProvider;
}

bool JournaldViewModel::isAvailable() const
{
    return d->mJournalAvailable;
}

QHash<int, QByteArray> JournaldViewModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[JournaldViewModel::ENTRY] = "entry";
    roles[JournaldViewModel::DATE] = "date";
    roles[JournaldViewModel::DATETIME] = "datetime";
    roles[JournaldViewModel::MONOTONIC_TIMESTAMP] = "monotonictimestamp";
    roles[JournaldViewModel::MESSAGE_ID] = "id";
    roles[JournaldViewModel::MESSAGE] = "message";
    roles[JournaldViewModel::PRIORITY] = "priority";
    roles[JournaldViewModel::SYSTEMD_UNIT] = "systemdunit";
    roles[JournaldViewModel::SYSTEMD_UNIT_CHANGED_SUBSTRING] = "systemdunit_changed_substring";
    roles[JournaldViewModel::EXE] = "exe";
    roles[JournaldViewModel::EXE_CHANGED_SUBSTRING] = "exe_changed_substring";
    roles[JournaldViewModel::BOOT_ID] = "bootid";
    roles[JournaldViewModel::SYSTEMD_UNIT_COLOR_BACKGROUND] = "systemdunitcolor_background";
    roles[JournaldViewModel::SYSTEMD_UNIT_COLOR_FOREGROUND] = "systemdunitcolor_foreground";
    roles[JournaldViewModel::EXE_COLOR_BACKGROUND] = "execolor_background";
    roles[JournaldViewModel::EXE_COLOR_FOREGROUND] = "execolor_foreground";
    roles[JournaldViewModel::CURSOR] = "cursor";
    return roles;
}

QVariant JournaldViewModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    // FIXME: Implement me!
    return QVariant();
}

QModelIndex JournaldViewModel::index(int row, int column, const QModelIndex &parent) const
{
    return createIndex(row, column);
}

QModelIndex JournaldViewModel::parent(const QModelIndex &index) const
{
    // no tree model, thus no parent
    return QModelIndex();
}

int JournaldViewModel::rowCount(const QModelIndex &parent) const
{
    // model represents a list and has has no children
    if (parent.isValid()) {
        return 0;
    }
    return d->mLog.size();
}

int JournaldViewModel::columnCount(const QModelIndex &parent) const
{
    return 1;
}

QVariant JournaldViewModel::data(const QModelIndex &index, int role) const
{
    if (index.row() < 0 || d->mLog.count() <= index.row()) {
        switch (role) {
        case JournaldViewModel::Roles::SYSTEMD_UNIT_COLOR_BACKGROUND:
        case JournaldViewModel::Roles::SYSTEMD_UNIT_COLOR_FOREGROUND:
        case JournaldViewModel::Roles::EXE_COLOR_BACKGROUND:
        case JournaldViewModel::Roles::EXE_COLOR_FOREGROUND:
            return QColor();
        default:
            return QVariant();
        }
    }
    switch (role) {
    case JournaldViewModel::Roles::ENTRY:
        return QVariant::fromValue(d->mLog.at(index.row()));
    case JournaldViewModel::Roles::MESSAGE:
        return QString(d->mLog.at(index.row()).message());
    case JournaldViewModel::Roles::MESSAGE_ID:
        return QString(d->mLog.at(index.row()).id());
    case JournaldViewModel::Roles::DATE:
        return d->mLog.at(index.row()).date().date();
    case JournaldViewModel::Roles::DATETIME:
        return d->mLog.at(index.row()).date();
    case JournaldViewModel::Roles::MONOTONIC_TIMESTAMP:
        return d->mLog.at(index.row()).monotonicTimestamp();
    case JournaldViewModel::Roles::BOOT_ID:
        return d->mLog.at(index.row()).bootId();
    case JournaldViewModel::Roles::SYSTEMD_UNIT:
        return d->mLog.at(index.row()).unit();
    case JournaldViewModel::Roles::SYSTEMD_UNIT_CHANGED_SUBSTRING: {
        QString unit = d->mLog.at(index.row()).unit();
        if (index.row() != 0) {
            unit.remove(d->mLog.at(index.row() - 1).unit());
        }
        return unit;
    }
    case JournaldViewModel::Roles::PRIORITY:
        return d->mLog.at(index.row()).priority();
    case JournaldViewModel::Roles::EXE:
        return d->mLog.at(index.row()).exe();
    case JournaldViewModel::Roles::EXE_CHANGED_SUBSTRING: {
        QString exe = d->mLog.at(index.row()).exe();
        if (index.row() != 0) {
            exe.remove(d->mLog.at(index.row() - 1).exe());
        }
        return exe;
    }
    case JournaldViewModel::Roles::SYSTEMD_UNIT_COLOR_BACKGROUND:
        if (d->mEnableServiceTemplateGrouping) {
            return Colorizer::color(d->mLog.at(index.row()).unitTemplateGroup(), Colorizer::COLOR_TYPE::BACKGROUND);
        } else {
            return Colorizer::color(d->mLog.at(index.row()).unit(), Colorizer::COLOR_TYPE::BACKGROUND);
        }
    case JournaldViewModel::Roles::SYSTEMD_UNIT_COLOR_FOREGROUND:
        if (d->mEnableServiceTemplateGrouping) {
            return Colorizer::color(d->mLog.at(index.row()).unitTemplateGroup(), Colorizer::COLOR_TYPE::FOREGROUND);
        } else {
            return Colorizer::color(d->mLog.at(index.row()).unit(), Colorizer::COLOR_TYPE::FOREGROUND);
        }
    case JournaldViewModel::Roles::EXE_COLOR_BACKGROUND:
        return Colorizer::color(d->mLog.at(index.row()).exe(), Colorizer::COLOR_TYPE::BACKGROUND);
    case JournaldViewModel::Roles::EXE_COLOR_FOREGROUND:
        return Colorizer::color(d->mLog.at(index.row()).exe(), Colorizer::COLOR_TYPE::FOREGROUND);
    case JournaldViewModel::Roles::CURSOR:
        return d->mLog.at(index.row()).cursor();
    }
    return QVariant();
}

QDateTime JournaldViewModel::datetime(int indexRow) const
{
    return data(index(indexRow, 0), JournaldViewModel::Roles::DATE).toDateTime();
}

bool JournaldViewModel::canFetchMore(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return false;
    }
    return !d->mModelResetActive && !(d->mHeadCursorReached && d->mTailCursorReached);
}

void JournaldViewModel::fetchMore(const QModelIndex &parent)
{
    if (parent.isValid()) {
        return;
    }
    if (d->mModelResetActive) {
        return;
    }
    fetchMoreLogEntries();
}

std::pair<int, int> JournaldViewModel::fetchMoreLogEntries()
{
    // guard against possible multi-threaded fetching access from QML engine while a fetch is not yet completed
    int instance = d->mActiveFetchOperations.fetchAndAddRelaxed(1);
    if (instance != 0) {
        qWarning() << "Skipping fetch operation, already one in progress";
        return std::pair(0, 0);
    }
    // increase window in both directions, since QAbstractIdemModel::fetchMore cannot
    // provide any indication of the direction. yet, this is not a real problem,
    // because by design usually the head or tail are already reached because that is
    // where we begin reading the log

    std::pair<int, int> fetchResult;
    { // append to log
        QVector<LogEntry> chunk = d->readEntries(JournaldViewModelPrivate::Direction::TOWARDS_TAIL);
        if (chunk.size() > 0) {
            beginInsertRows(QModelIndex(), d->mLog.size(), d->mLog.size() + chunk.size() - 1);
            d->mLog.append(chunk);
            endInsertRows();
            qCDebug(KJOURNALDLIB_GENERAL) << "read towards tail" << chunk.size();
            fetchResult.first = chunk.size();
        }
    }

    { // prepend to log
        QVector<LogEntry> chunk = d->readEntries(JournaldViewModelPrivate::Direction::TOWARDS_HEAD);
        if (chunk.size() > 0) {
            beginInsertRows(QModelIndex(), 0, chunk.size() - 1);
            d->mLog = chunk << d->mLog; // TODO find more performant way than constructing a new vector every time
            endInsertRows();
            qCDebug(KJOURNALDLIB_GENERAL) << "read towards head" << chunk.size();
            fetchResult.second = chunk.size();
        }
    }
    d->mActiveFetchOperations = 0;
    return fetchResult;
}

void JournaldViewModel::setFetchMoreChunkSize(quint32 size)
{
    if (size > 0) {
        d->mChunkSize = size;
    } else {
        qCWarning(KJOURNALDLIB_GENERAL) << "chunk size 0 is currently ignored";
    }
}

void JournaldViewModel::seekHead()
{
    guardedBeginResetModel();
    d->mLog.clear();
    if (d->mJournal && d->mJournal->isValid()) {
        d->seekHeadAndMakeCurrent();
        QVector<LogEntry> chunk = d->readEntries(JournaldViewModelPrivate::Direction::TOWARDS_TAIL);
        d->mLog = chunk;
    } else {
        qCCritical(KJOURNALDLIB_GENERAL) << "Cannot seek head of invalid journal";
    }
    guardedEndResetModel();
}

void JournaldViewModel::seekTail()
{
    guardedBeginResetModel();
    d->mLog.clear();
    if (d->mJournal && d->mJournal->isValid()) {
        d->seekTailAndMakeCurrent();
        QVector<LogEntry> chunk = d->readEntries(JournaldViewModelPrivate::Direction::TOWARDS_HEAD);
        d->mLog = chunk;
    } else {
        qCCritical(KJOURNALDLIB_GENERAL) << "Cannot seek head of invalid journal";
    }
    guardedEndResetModel();
}

void JournaldViewModel::setFilter(const Filter &filter)
{
    qCDebug(KJOURNALDLIB_FILTERTRACE) << "setfilter" << filter;
    // TODO add == operator and skip setting of same filter
    guardedBeginResetModel();
    d->mFilter = filter;
    d->resetJournal();
    guardedEndResetModel();
    fetchMoreLogEntries();
}

void JournaldViewModel::resetFilter()
{
    Filter defaultFilter;
    setFilter(defaultFilter);
}

Filter JournaldViewModel::filter() const
{
    return d->mFilter;
}

bool JournaldViewModel::groupTemplatedSystemdUnits()
{
    return d->mEnableServiceTemplateGrouping;
}

void JournaldViewModel::setGroupTemplatedSystemdUnits(bool enabled)
{
    if (enabled == d->mEnableServiceTemplateGrouping) {
        return;
    }
    qCDebug(KJOURNALDLIB_FILTERTRACE) << "reset view model due to template group change";
    d->mEnableServiceTemplateGrouping = enabled;
    Q_EMIT groupTemplatedSystemdUnitsChanged();
    Q_EMIT dataChanged(index(0, 0), index(d->mLog.size() - 1, 0), {Roles::SYSTEMD_UNIT_COLOR_BACKGROUND, Roles::SYSTEMD_UNIT_COLOR_FOREGROUND});
}

int JournaldViewModel::search(const QString &searchString, int startRow, bool caseSensitive, Direction direction)
{
    int row = startRow;
    const Qt::CaseSensitivity caseSensitivity = caseSensitive ? Qt::CaseSensitive : Qt::CaseInsensitive;

    if (direction == FORWARD) {
        while (row < d->mLog.size()) {
            if (d->mLog.at(row).message().contains(searchString, caseSensitivity)) {
                qCDebug(KJOURNALDLIB_GENERAL) << "Found string in line" << row << d->mLog.at(row).message();
                return row;
            }
            ++row;
            if (row == d->mLog.size() && canFetchMore(QModelIndex())) { // if end is reached, try to fetch more
                fetchMoreLogEntries(); // TODO improve performance by fetching only into scroll direction
            }
        }
    } else {
        while (row >= 0) {
            if (d->mLog.at(row).message().contains(searchString, caseSensitivity)) {
                qCDebug(KJOURNALDLIB_GENERAL) << "Found string in line" << row << d->mLog.at(row).message();
                return row;
            }
            --row;
            if (row == d->mLog.size() && canFetchMore(QModelIndex())) { // if end is reached, try to fetch more
                std::pair<int, int> fetchedRows = fetchMoreLogEntries(); // TODO improve performance by fetching only into scroll direction
                row += fetchedRows.first;
            }
        }
    }

    return -1;
}

int JournaldViewModel::closestIndexForData(const QDateTime &datetime)
{
    if (d->mLog.isEmpty()) {
        return -1;
    }
    if (datetime > d->mLog.last().date()) {
        return d->mLog.size() - 1;
    }

    auto it = std::lower_bound(d->mLog.cbegin(), d->mLog.cend(), datetime, [](const LogEntry &entry, const QDateTime &needle) {
        return entry.date() < needle;
    });

    if (it == d->mLog.cend()) {
        return -1;
    } else {
        std::size_t index = std::distance(d->mLog.cbegin(), it);
        return index;
    }
}

#include "moc_journaldviewmodel.cpp"
