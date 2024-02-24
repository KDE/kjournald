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
#include "localjournal.h"
#include <QColor>
#include <QDebug>
#include <QDir>
#include <QMutex>
#include <QRandomGenerator>
#include <QThread>
#include <algorithm>
#include <iterator>

void JournaldViewModelPrivate::resetJournal()
{
    if (!mJournal->isValid()) {
        qCWarning(KJOURNALDLIB_GENERAL) << "Skipping seek head, no valid journal open";
        return;
    }

    int result{0};

    // reset all filters
    sd_journal_flush_matches(mJournal->sdJournal());

    qCDebug(KJOURNALDLIB_FILTERTRACE) << "flush_matches()";

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
    //     AND (transport=kernel) OR (unit_1 OR unit_2 OR ...) OR (exe=x OR exe=y OR ...)
    // And for non-kernel transport option:
    //     (boot=123 OR boot=...)
    //     AND (priority=1 OR priority=...)
    //     AND (transport=not-kernel)
    //     AND (unit_1 OR unit_2 OR ...) OR (exe=x OR exe=y OR ...)

    // filter boots
    for (const QString &boot : qAsConst(mBootFilter)) {
        QString filterExpression = QLatin1String("_BOOT_ID=") + boot;
        result = sd_journal_add_match(mJournal->sdJournal(), filterExpression.toUtf8().constData(), 0);
        qCDebug(KJOURNALDLIB_FILTERTRACE).nospace() << "add_match(" << filterExpression << ")";
        if (result < 0) {
            qCCritical(KJOURNALDLIB_GENERAL) << "Failed to set journal filter:" << strerror(-result) << filterExpression;
        }
    }
    if (mPriorityFilter.has_value()) {
        for (int i = 0; i <= mPriorityFilter; ++i) {
            QString filterExpression = QLatin1String("PRIORITY=") + QString::number(i);
            result = sd_journal_add_match(mJournal->sdJournal(), filterExpression.toUtf8().constData(), 0);
            qCDebug(KJOURNALDLIB_FILTERTRACE).nospace() << "add_match(" << filterExpression << ")";
            if (result < 0) {
                qCCritical(KJOURNALDLIB_GENERAL) << "Failed to set journal filter:" << strerror(-result) << filterExpression;
            }
        }
        qCDebug(KJOURNALDLIB_GENERAL) << "Use priority filter level:" << mPriorityFilter.value();
    } else {
        qCDebug(KJOURNALDLIB_GENERAL) << "Skip setting priority filter";
    }

    // boot and priority filter shall always be enforced
    result = sd_journal_add_conjunction(mJournal->sdJournal());
    qCDebug(KJOURNALDLIB_FILTERTRACE).nospace() << "add_conjunction()";
    Q_ASSERT(result >= 0);

    // see journal-fields documentation regarding list of valid transports
    // note: in case of kernel messages being activated, this filter automatically activates all kernel transport
    //       because kernel output will not match any further service/exe filter
    QStringList kernelTransports{QLatin1String("audit"), QLatin1String("driver"), QLatin1String("kernel")};
    QStringList nonKernelTransports{QLatin1String("syslog"), QLatin1String("journal"), QLatin1String("stdout")};
    if (mShowKernelMessages) {
        for (const QString &transport : kernelTransports) {
            QString filterExpression = QLatin1String("_TRANSPORT=") + transport;
            result = sd_journal_add_match(mJournal->sdJournal(), filterExpression.toUtf8().constData(), 0);
            qCDebug(KJOURNALDLIB_FILTERTRACE).nospace() << "add_match(" << filterExpression << ")";
            if (result < 0) {
                qCCritical(KJOURNALDLIB_GENERAL) << "Failed to set journal filter:" << strerror(-result) << filterExpression;
            }
        }
        result = sd_journal_add_disjunction(mJournal->sdJournal());
        Q_ASSERT(result >= 0);
    } else {
        for (const QString &transport : nonKernelTransports) {
            QString filterExpression = QLatin1String("_TRANSPORT=") + transport;
            result = sd_journal_add_match(mJournal->sdJournal(), filterExpression.toUtf8().constData(), 0);
            qCDebug(KJOURNALDLIB_FILTERTRACE).nospace() << "add_match(" << filterExpression << ")";
            if (result < 0) {
                qCCritical(KJOURNALDLIB_GENERAL) << "Failed to set journal filter:" << strerror(-result) << filterExpression;
            }
        }
        result = sd_journal_add_conjunction(mJournal->sdJournal());
        Q_ASSERT(result >= 0);
    }

    // filter units
    for (const QString &unit : qAsConst(mSystemdUnitFilter)) {
        QString filterExpression = QLatin1String("_SYSTEMD_UNIT=") + unit;
        result = sd_journal_add_match(mJournal->sdJournal(), filterExpression.toUtf8().constData(), 0);
        qCDebug(KJOURNALDLIB_FILTERTRACE).nospace() << "add_match(" << filterExpression << ")";
        if (result < 0) {
            qCCritical(KJOURNALDLIB_GENERAL) << "Failed to set journal filter:" << strerror(-result) << filterExpression;
        }
    }

    result = sd_journal_add_disjunction(mJournal->sdJournal());
    Q_ASSERT(result >= 0);

    // filter executable
    for (const QString &executable : qAsConst(mExeFilter)) {
        QString filterExpression = QLatin1String("_EXE=") + executable;
        result = sd_journal_add_match(mJournal->sdJournal(), filterExpression.toUtf8().constData(), 0);
        qCDebug(KJOURNALDLIB_FILTERTRACE).nospace() << "add_match(" << filterExpression << ")";
        if (result < 0) {
            qCCritical(KJOURNALDLIB_GENERAL) << "Failed to set journal filter:" << strerror(-result) << filterExpression;
        }
    }

    qCDebug(KJOURNALDLIB_FILTERTRACE).nospace() << "Filter DONE";
    mTailCursorReached = false;
    seekHeadAndMakeCurrent();
    // clear all data which are in limbo with new head
    mLog.clear();
}

QVector<LogEntry> JournaldViewModelPrivate::readEntries(Direction direction)
{
    static QMutex mutex;
    QMutexLocker locker(&mutex);

    int result{0};
    QVector<LogEntry> chunk;
    if (!mJournal->isValid()) {
        qCWarning(KJOURNALDLIB_GENERAL) << "Skipping data fetch, no valid journal opened";
        return chunk;
    }
    if (direction == Direction::TOWARDS_TAIL) {
        if (mLog.size() > 0) {
            QString cursor = mLog.last().mCursor;
            // note: seek cursor does not make it current, but a subsequent sd_journal_next is required
            result = sd_journal_seek_cursor(mJournal->sdJournal(), cursor.toUtf8().constData());
            if (result < 0) {
                qCWarning(KJOURNALDLIB_GENERAL) << "seeking cursor but could not be found" << strerror(-result);
            }
            result = sd_journal_next(mJournal->sdJournal());
            if (result == 0) {
                mTailCursorReached = true;
                return {};
            }
            result = sd_journal_test_cursor(mJournal->sdJournal(), cursor.toUtf8().constData());
            if (result <= 0) {
                qCCritical(KJOURNALDLIB_GENERAL) << "current position does not match expected cursor:" << cursor;
                if (result < 0) {
                    qCCritical(KJOURNALDLIB_GENERAL) << "cursor test failed:" << strerror(-result);
                }
                return {};
            }
            // read first entry after cursor
            result = sd_journal_next(mJournal->sdJournal());
            if (result == 0) {
                mTailCursorReached = true;
                return {};
            }
        } else {
            if (!seekHeadAndMakeCurrent()) {
                return {};
            }
        }
    } else if (direction == Direction::TOWARDS_HEAD) {
        if (mLog.size() > 0) {
            QString cursor = mLog.first().mCursor;
            result = sd_journal_seek_cursor(mJournal->sdJournal(), cursor.toUtf8().constData());
            if (result < 0) {
                qCWarning(KJOURNALDLIB_GENERAL) << "seeking cursor but could not be found" << strerror(-result);
            }
            result = sd_journal_previous(mJournal->sdJournal());
            if (result == 0) {
                mHeadCursorReached = true;
                return {};
            }
            result = sd_journal_test_cursor(mJournal->sdJournal(), cursor.toUtf8().constData());
            if (result <= 0) {
                qCCritical(KJOURNALDLIB_GENERAL) << "current position does not match expected cursor:" << cursor;
                if (result < 0) {
                    qCCritical(KJOURNALDLIB_GENERAL) << "cursor test failed:" << strerror(-result);
                }
                return {};
            }
            // read first entry before cursor
            result = sd_journal_previous(mJournal->sdJournal());
            if (result == 0) {
                mHeadCursorReached = true;
                return {};
            }
        } else {
            if (!seekTailAndMakeCurrent()) {
                return {};
            }
        }
    } else {
        qCCritical(KJOURNALDLIB_GENERAL) << "Jumping into the journal's middle, not supported";
        return {};
    }

    // at this point, the journal is guaranteed to point to the first valid entry
    for (int counter = 0; counter < mChunkSize; ++counter) {
        char *data{nullptr};
        size_t length;
        uint64_t time;
        int result{1};
        LogEntry entry;
        result = sd_journal_get_realtime_usec(mJournal->sdJournal(), &time);
        if (result == 0) {
            entry.mDate = QDateTime::fromMSecsSinceEpoch(time / 1000, Qt::UTC);
        }
        sd_id128_t bootId; // currently unused
        result = sd_journal_get_monotonic_usec(mJournal->sdJournal(), &time, &bootId);
        if (result == 0) {
            entry.mMonotonicTimestamp = time;
        }
        result = sd_journal_get_data(mJournal->sdJournal(), "MESSAGE", (const void **)&data, &length);
        if (result == 0) {
            entry.mMessage = QString::fromUtf8(data, length).section(QChar::fromLatin1('='), 1);
        }
        result = sd_journal_get_data(mJournal->sdJournal(), "MESSAGE_ID", (const void **)&data, &length);
        if (result == 0) {
            entry.mId = QString::fromUtf8(data, length).section(QChar::fromLatin1('='), 1);
        }
        result = sd_journal_get_data(mJournal->sdJournal(), "_SYSTEMD_UNIT", (const void **)&data, &length);
        if (result == 0) {
            entry.mSystemdUnit = JournaldHelper::cleanupString(QString::fromUtf8(data, length).section(QChar::fromLatin1('='), 1));
        }
        result = sd_journal_get_data(mJournal->sdJournal(), "_BOOT_ID", (const void **)&data, &length);
        if (result == 0) {
            entry.mBootId = QString::fromUtf8(data, length).section(QChar::fromLatin1('='), 1);
        }
        result = sd_journal_get_data(mJournal->sdJournal(), "_EXE", (const void **)&data, &length);
        if (result == 0) {
            entry.mExe = QString::fromUtf8(data, length).section(QChar::fromLatin1('='), 1);
        }
        result = sd_journal_get_data(mJournal->sdJournal(), "PRIORITY", (const void **)&data, &length);
        if (result == 0) {
            entry.mPriority = QString::fromUtf8(data, length).section(QChar::fromLatin1('='), 1).toInt();
        }
        result = sd_journal_get_cursor(mJournal->sdJournal(), &data);
        if (result == 0) {
            entry.mCursor = QString::fromUtf8(data);
            free(data);
        }

        if (direction == Direction::TOWARDS_TAIL) {
            chunk.append(std::move(entry));
        } else {
            chunk.prepend(std::move(entry));
        }

        // obtain more data, 1 for success, 0 if reached end
        if (direction == Direction::TOWARDS_TAIL) {
            result = sd_journal_next(mJournal->sdJournal());
            if (result == 0) {
                mTailCursorReached = true;
                qCDebug(KJOURNALDLIB_GENERAL) << "obtained journal until tail, stop reading";
                break;
            }
        } else {
            if (sd_journal_previous(mJournal->sdJournal()) <= 0) {
                mHeadCursorReached = true;
                qCDebug(KJOURNALDLIB_GENERAL) << "obtained journal until head, stop reading";
                break;
            }
        }
    }

    return chunk;
}

bool JournaldViewModelPrivate::seekHeadAndMakeCurrent()
{
    qCDebug(KJOURNALDLIB_GENERAL) << "seek head and make current";
    int result = sd_journal_seek_head(mJournal->sdJournal());
    if (result < 0) {
        qCCritical(KJOURNALDLIB_GENERAL) << "Failed to seek head:" << strerror(-result);
        return false;
    }
    if (sd_journal_next(mJournal->sdJournal()) <= 0) {
        qCWarning(KJOURNALDLIB_GENERAL) << "could not make head entry current";
        return false;
    }
    mHeadCursorReached = true;
    return true;
}

bool JournaldViewModelPrivate::seekTailAndMakeCurrent()
{
    qCDebug(KJOURNALDLIB_GENERAL) << "seek tail and make current";
    int result = sd_journal_seek_tail(mJournal->sdJournal());
    if (result < 0) {
        qCCritical(KJOURNALDLIB_GENERAL) << "Failed to seek head:" << strerror(-result);
        return false;
    }
    if (sd_journal_previous(mJournal->sdJournal()) <= 0) {
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
    setSystemJournal();
}

JournaldViewModel::JournaldViewModel(const QString &path, QObject *parent)
    : QAbstractItemModel(parent)
    , d(new JournaldViewModelPrivate)
{
    setJournaldPath(path);
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

bool JournaldViewModel::setJournal(std::unique_ptr<IJournal> journal)
{
    // with this setter the problem starts
    bool success{true};
    guardedBeginResetModel();
    d->mLog.clear();
    d->mJournal = std::move(journal);
    success = d->mJournal->isValid();
    if (success) {
        d->resetJournal();
    }
    guardedEndResetModel();
    fetchMoreLogEntries();
    connect(d->mJournal.get(), &IJournal::journalUpdated, this, [=](const QString &bootId) {
        if (!d->mBootFilter.contains(bootId)) {
            return;
        }
        if (d->mTailCursorReached) {
            d->mTailCursorReached = false;
            fetchMoreLogEntries();
        }
    });
    return success;
}

bool JournaldViewModel::setJournaldPath(const QString &path)
{
    return setJournal(std::make_unique<LocalJournal>(path));
}

bool JournaldViewModel::setSystemJournal()
{
    return setJournal(std::make_unique<LocalJournal>());
}

QHash<int, QByteArray> JournaldViewModel::roleNames() const
{
    QHash<int, QByteArray> roles;
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
        return QVariant();
    }
    switch (role) {
    case JournaldViewModel::Roles::MESSAGE:
        // TODO add handling for arbitrary color codes
        return QString(d->mLog.at(index.row()).mMessage)
            .remove(QLatin1String("\u001B[96m"))
            .remove(QLatin1String("\u001B[0m"))
            .remove(QLatin1String("\u001B[93m"))
            .remove(QLatin1String("\u001B[31m"));
    case JournaldViewModel::Roles::MESSAGE_ID:
        return QString(d->mLog.at(index.row()).mId);
    case JournaldViewModel::Roles::DATE:
        return d->mLog.at(index.row()).mDate.date();
    case JournaldViewModel::Roles::DATETIME:
        return d->mLog.at(index.row()).mDate;
    case JournaldViewModel::Roles::MONOTONIC_TIMESTAMP:
        return d->mLog.at(index.row()).mMonotonicTimestamp;
    case JournaldViewModel::Roles::BOOT_ID:
        return d->mLog.at(index.row()).mBootId;
    case JournaldViewModel::Roles::SYSTEMD_UNIT:
        return d->mLog.at(index.row()).mSystemdUnit;
    case JournaldViewModel::Roles::SYSTEMD_UNIT_CHANGED_SUBSTRING: {
        QString unit = d->mLog.at(index.row()).mSystemdUnit;
        if (index.row() != 0) {
            unit.remove(d->mLog.at(index.row() - 1).mSystemdUnit);
        }
        return unit;
    }
    case JournaldViewModel::Roles::PRIORITY:
        return d->mLog.at(index.row()).mPriority;
    case JournaldViewModel::Roles::EXE:
        return d->mLog.at(index.row()).mExe;
    case JournaldViewModel::Roles::EXE_CHANGED_SUBSTRING: {
        QString exe = d->mLog.at(index.row()).mExe;
        if (index.row() != 0) {
            exe.remove(d->mLog.at(index.row() - 1).mExe);
        }
        return exe;
    }
    case JournaldViewModel::Roles::SYSTEMD_UNIT_COLOR_BACKGROUND:
        return Colorizer::color(d->mLog.at(index.row()).mSystemdUnit, Colorizer::COLOR_TYPE::BACKGROUND);
    case JournaldViewModel::Roles::SYSTEMD_UNIT_COLOR_FOREGROUND:
        return Colorizer::color(d->mLog.at(index.row()).mSystemdUnit, Colorizer::COLOR_TYPE::FOREGROUND);
    case JournaldViewModel::Roles::EXE_COLOR_BACKGROUND:
        return Colorizer::color(d->mLog.at(index.row()).mExe, Colorizer::COLOR_TYPE::BACKGROUND);
    case JournaldViewModel::Roles::EXE_COLOR_FOREGROUND:
        return Colorizer::color(d->mLog.at(index.row()).mExe, Colorizer::COLOR_TYPE::FOREGROUND);
    case JournaldViewModel::Roles::CURSOR:
        return d->mLog.at(index.row()).mCursor;
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

void JournaldViewModel::setSystemdUnitFilter(const QStringList &systemdUnitFilter)
{
    guardedBeginResetModel();
    d->mSystemdUnitFilter = systemdUnitFilter;
    d->resetJournal();
    guardedEndResetModel();
    fetchMoreLogEntries();
}

QStringList JournaldViewModel::systemdUnitFilter() const
{
    return d->mSystemdUnitFilter;
}

void JournaldViewModel::setBootFilter(const QStringList &bootFilter)
{
    if (d->mBootFilter == bootFilter) {
        return;
    }
    guardedBeginResetModel();
    d->mBootFilter = bootFilter;
    d->resetJournal();
    guardedEndResetModel();
    fetchMoreLogEntries();
    Q_EMIT bootFilterChanged();
}

QStringList JournaldViewModel::bootFilter() const
{
    return d->mBootFilter;
}

void JournaldViewModel::setExeFilter(const QStringList &exeFilter)
{
    if (d->mExeFilter == exeFilter) {
        return;
    }
    guardedBeginResetModel();
    d->mExeFilter = exeFilter;
    d->resetJournal();
    guardedEndResetModel();
    fetchMoreLogEntries();
    Q_EMIT exeFilterChanged();
}

QStringList JournaldViewModel::exeFilter() const
{
    return d->mExeFilter;
}

void JournaldViewModel::setPriorityFilter(int priority)
{
    qCDebug(KJOURNALDLIB_GENERAL) << "Set priority filter to:" << priority;
    guardedBeginResetModel();
    if (priority >= 0) {
        d->mPriorityFilter = priority;
    } else {
        d->mPriorityFilter = std::nullopt;
    }
    d->resetJournal();
    guardedEndResetModel();
    fetchMoreLogEntries();
    Q_EMIT priorityFilterChanged();
}

void JournaldViewModel::resetPriorityFilter()
{
    guardedBeginResetModel();
    d->mPriorityFilter.reset();
    d->resetJournal();
    guardedEndResetModel();
    fetchMoreLogEntries();
    Q_EMIT priorityFilterChanged();
}

int JournaldViewModel::priorityFilter() const
{
    return d->mPriorityFilter.value_or(-1);
}

void JournaldViewModel::setKernelFilter(bool showKernelMessages)
{
    if (d->mShowKernelMessages == showKernelMessages) {
        return;
    }
    guardedBeginResetModel();
    d->mShowKernelMessages = showKernelMessages;
    d->resetJournal();
    guardedEndResetModel();
    fetchMoreLogEntries();
    Q_EMIT kernelFilterChanged();
}

bool JournaldViewModel::isKernelFilterEnabled() const
{
    return d->mShowKernelMessages;
}

int JournaldViewModel::search(const QString &searchString, int startRow, Direction direction)
{
    int row = startRow;

    if (direction == FORWARD) {
        while (row < d->mLog.size()) {
            if (d->mLog.at(row).mMessage.contains(searchString)) {
                qCDebug(KJOURNALDLIB_GENERAL) << "Found string in line" << row << d->mLog.at(row).mMessage;
                return row;
            }
            ++row;
            if (row == d->mLog.size() && canFetchMore(QModelIndex())) { // if end is reached, try to fetch more
                fetchMoreLogEntries(); // TODO improve performance by fetching only into scroll direction
            }
        }
    } else {
        while (row >= 0) {
            if (d->mLog.at(row).mMessage.contains(searchString)) {
                qCDebug(KJOURNALDLIB_GENERAL) << "Found string in line" << row << d->mLog.at(row).mMessage;
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
    if (datetime > d->mLog.last().mDate) {
        return d->mLog.size() - 1;
    }

    auto it = std::lower_bound(d->mLog.cbegin(), d->mLog.cend(), datetime, [](const LogEntry &entry, const QDateTime &needle) {
        return entry.mDate < needle;
    });

    if (it == d->mLog.cend()) {
        return -1;
    } else {
        std::size_t index = std::distance(d->mLog.cbegin(), it);
        return index;
    }
}

QString JournaldViewModel::formatTime(const QDateTime &datetime, bool utc) const
{
    if (utc) {
        return datetime.toUTC().time().toString(QLatin1String("HH:mm:ss.zzz"));
    } else {
        return datetime.time().toString(QLatin1String("HH:mm:ss.zzz"));
    }
}
