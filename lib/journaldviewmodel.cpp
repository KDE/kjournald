/*
    SPDX-License-Identifier: LGPL-2.1-or-later OR MIT
    SPDX-FileCopyrightText: 2021 Andreas Cord-Landwehr <cordlandwehr@kde.org>
*/

#include "journaldviewmodel.h"
#include "journaldhelper.h"
#include "journaldviewmodel_p.h"
#include "localjournal.h"
#include "loggingcategories.h"
#include <QColor>
#include <QDebug>
#include <QDir>
#include <QMutex>
#include <QRandomGenerator>
#include <QTextDecoder>
#include <QThread>
#include <algorithm>
#include <iterator>

QRandomGenerator JournaldViewModelPrivate::sFixedSeedGenerator{1}; // used fixed seed to ensure that colors for same units never change

QColor JournaldViewModelPrivate::unitColor(const QString &unit, COLOR_TYPE colorType)
{
    const auto key = unit;
    if (!mUnitToColorMap.contains(key)) {
        int hue = sFixedSeedGenerator.bounded(255);
        QColor logEntry = QColor::fromHsl(hue, 200, 220);
        QColor unit = QColor::fromHsl(hue, 220, 150);
        mUnitToColorMap[key] = std::make_pair(logEntry, unit);
    }
    if (colorType == COLOR_TYPE::LOG_ENTRY) {
        return mUnitToColorMap.value(key).first;
    } else {
        return mUnitToColorMap.value(key).second;
    }
}

void JournaldViewModelPrivate::resetJournal()
{
    if (!mJournal->isValid()) {
        qCWarning(journald()) << "Skipping seek head, no valid journal open";
        return;
    }

    int result{0};

    // reset all filters
    sd_journal_flush_matches(mJournal->sdJournal());

    // in the following a logical expression with with the following content is created:
    // AND (boot_1 OR boot...) AND (priority_1 OR prio...) AND ((?kernel-messages) OR (non-kernel-tranport) AND (unit_1 OR unit_2 OR unit...))
    // filter boots
    for (const QString &boot : qAsConst(mBootFilter)) {
        QString filterExpression = QLatin1String("_BOOT_ID=") + boot;
        result = sd_journal_add_match(mJournal->sdJournal(), filterExpression.toLocal8Bit().constData(), 0);
        if (result < 0) {
            qCCritical(journald) << "Failed to set journal filter:" << strerror(-result) << filterExpression;
        }
    }
    if (mPriorityFilter.has_value()) {
        for (int i = 0; i <= mPriorityFilter; ++i) {
            QString filterExpression = QLatin1String("PRIORITY=") + QString::number(i);
            result = sd_journal_add_match(mJournal->sdJournal(), filterExpression.toLocal8Bit().constData(), 0);
            if (result < 0) {
                qCCritical(journald()) << "Failed to set journal filter:" << strerror(-result) << filterExpression;
            }
        }
    }

    // boot and priority filter shall always be enforced
    result = sd_journal_add_conjunction(mJournal->sdJournal());
    Q_ASSERT(result >= 0);

    // see journal-fields documentation regarding list of valid transports
    QStringList kernelTransports{QLatin1String("audit"), QLatin1String("driver"), QLatin1String("kernel")};
    QStringList nonKernelTransports{QLatin1String("syslog"), QLatin1String("journal"), QLatin1String("stdout")};
    if (mShowKernelMessages) {
        for (const QString &transport : kernelTransports) {
            QString filterExpression = QLatin1String("_TRANSPORT=") + transport;
            result = sd_journal_add_match(mJournal->sdJournal(), filterExpression.toLocal8Bit().constData(), 0);
            if (result < 0) {
                qCCritical(journald) << "Failed to set journal filter:" << strerror(-result) << filterExpression;
            }
        }
    }
    result = sd_journal_add_disjunction(mJournal->sdJournal());
    Q_ASSERT(result >= 0);

    // special case handling where for messages that are missing a _TRANSPORT entry and otherwise might be missing in log output
    if (!mShowKernelMessages) {
        for (const QString &transport : nonKernelTransports) {
            QString filterExpression = QLatin1String("_TRANSPORT=") + transport;
            result = sd_journal_add_match(mJournal->sdJournal(), filterExpression.toLocal8Bit().constData(), 0);
            if (result < 0) {
                qCCritical(journald) << "Failed to set journal filter:" << strerror(-result) << filterExpression;
            }
        }
    }

    // filter units
    for (const QString &unit : qAsConst(mSystemdUnitFilter)) {
        QString filterExpression = QLatin1String("_SYSTEMD_UNIT=") + unit;
        result = sd_journal_add_match(mJournal->sdJournal(), filterExpression.toLocal8Bit().constData(), 0);
        if (result < 0) {
            qCCritical(journald) << "Failed to set journal filter:" << strerror(-result) << filterExpression;
        }
    }

    // filter executable
    for (const QString &executable : qAsConst(mExeFilter)) {
        QString filterExpression = QLatin1String("_EXE=") + executable;
        result = sd_journal_add_match(mJournal->sdJournal(), filterExpression.toLocal8Bit().constData(), 0);
        if (result < 0) {
            qCCritical(journald) << "Failed to set journal filter:" << strerror(-result) << filterExpression;
        }
    }

    mTailCursorReached = false;
    seekHeadAndMakeCurrent();
    // clear all data which are in limbo with new head
    mLog.clear();
}

QVector<LogEntry> JournaldViewModelPrivate::readEntries(Direction direction)
{
    static QMutex mutex;
    QMutexLocker locker(&mutex);

    const int readChunkSize{500};
    int result{0};
    QVector<LogEntry> chunk;
    if (!mJournal->isValid()) {
        qCWarning(journald) << "Skipping data fetch, no valid journal opened";
        return chunk;
    }
    if (direction == Direction::TOWARDS_TAIL) {
        if (mLog.size() > 0) {
            QString cursor = mLog.last().mCursor;
            // note: seek cursor does not make it current, but a subsequenct sd_journal_next is required
            result = sd_journal_seek_cursor(mJournal->sdJournal(), cursor.toLocal8Bit().constData());
            if (result < 0) {
                qCWarning(journald()) << "seeking cursor but could not be found" << strerror(-result);
            }
            result = sd_journal_next(mJournal->sdJournal());
            if (result == 0) {
                mTailCursorReached = true;
                return {};
            }
            result = sd_journal_test_cursor(mJournal->sdJournal(), cursor.toLocal8Bit().constData());
            if (result <= 0) {
                qCCritical(journald()) << "current position does not match expected cursor:" << cursor;
                if (result < 0) {
                    qCCritical(journald()) << "cursor test failed:" << strerror(-result);
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
            seekHeadAndMakeCurrent();
        }
    } else if (direction == Direction::TOWARDS_HEAD) {
        if (mLog.size() > 0) {
            QString cursor = mLog.first().mCursor;
            result = sd_journal_seek_cursor(mJournal->sdJournal(), cursor.toLocal8Bit().constData());
            if (result < 0) {
                qCWarning(journald()) << "seeking cursor but could not be found" << strerror(-result);
            }
            result = sd_journal_previous(mJournal->sdJournal());
            if (result == 0) {
                mHeadCursorReached = true;
                return {};
            }
            result = sd_journal_test_cursor(mJournal->sdJournal(), cursor.toLocal8Bit().constData());
            if (result <= 0) {
                qCCritical(journald()) << "current position does not match expected cursor:" << cursor;
                if (result < 0) {
                    qCCritical(journald()) << "cursor test failed:" << strerror(-result);
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
            seekTailAndMakeCurrent();
        }
    } else {
        qCCritical(journald()) << "Jumping into the journal's middle, not supported";
        return {};
    }

    // at this point, the journal is guaranteed to point to the first valid entry
    for (int counter = 0; counter < readChunkSize; ++counter) {
        char *data;
        size_t length;
        uint64_t time;
        int result{1};
        LogEntry entry;
        result = sd_journal_get_realtime_usec(mJournal->sdJournal(), &time);
        if (result == 0) {
            entry.mDate.setMSecsSinceEpoch(time / 1000);
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
                qCDebug(journald) << "obtained journal until tail, stop reading";
                break;
            }
        } else {
            if (sd_journal_previous(mJournal->sdJournal()) <= 0) {
                mHeadCursorReached = true;
                qCDebug(journald) << "obtained journal until head, stop reading";
                break;
            }
        }
    }
    return chunk;
}

void JournaldViewModelPrivate::seekHeadAndMakeCurrent()
{
    qCDebug(journald) << "seek head and make current";
    int result = sd_journal_seek_head(mJournal->sdJournal());
    if (result < 0) {
        qCCritical(journald) << "Failed to seek head:" << strerror(-result);
        return;
    }
    if (sd_journal_next(mJournal->sdJournal()) <= 0) {
        qCWarning(journald) << "could not make head entry current";
    }
    mHeadCursorReached = true;
}

void JournaldViewModelPrivate::seekTailAndMakeCurrent()
{
    qCDebug(journald) << "seek tail and make current";
    int result = sd_journal_seek_tail(mJournal->sdJournal());
    if (result < 0) {
        qCCritical(journald) << "Failed to seek head:" << strerror(-result);
        return;
    }
    if (sd_journal_previous(mJournal->sdJournal()) <= 0) {
        qCWarning(journald) << "could not make tail entry current";
    }
    mTailCursorReached = true;
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

bool JournaldViewModel::setJournal(std::unique_ptr<IJournal> journal)
{
    bool success{true};
    beginResetModel();
    d->mLog.clear();
    d->mJournal = std::move(journal);
    success = d->mJournal->isValid();
    if (success) {
        d->resetJournal();
        fetchMoreLogEntries();
    }
    endResetModel();
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
    roles[JournaldViewModel::BOOT_ID] = "bootid";
    roles[JournaldViewModel::UNIT_COLOR] = "unitcolor";
    roles[JournaldViewModel::UNIT_COLOR_DARK] = "unitcolordark";
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
    case Qt::DisplayRole:
        Q_FALLTHROUGH();
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
    case JournaldViewModel::Roles::PRIORITY:
        return d->mLog.at(index.row()).mPriority;
    case JournaldViewModel::Roles::EXE:
        return d->mLog.at(index.row()).mExe;
    case JournaldViewModel::Roles::UNIT_COLOR:
        return d->unitColor(d->mLog.at(index.row()).mSystemdUnit);
    case JournaldViewModel::Roles::UNIT_COLOR_DARK:
        return d->unitColor(d->mLog.at(index.row()).mSystemdUnit, JournaldViewModelPrivate::COLOR_TYPE::UNIT);
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
    return !(d->mHeadCursorReached && d->mTailCursorReached);
}

void JournaldViewModel::fetchMore(const QModelIndex &parent)
{
    fetchMoreLogEntries();
}

void JournaldViewModel::fetchMoreLogEntries()
{
    // guard against possible multi-threaded fetching access from QML engine while a fetch is not yet completed
    int instance = d->mActiveFetchOperations.fetchAndAddRelaxed(1);
    if (instance != 0) {
        qWarning() << "Skipping fetch operation, already one in progress";
        return;
    }
    // increase window in both directions, since QAbstractIdemModel::fetchMore cannot
    // provide any indication of the direction. yet, this is not a real problem,
    // because by design usually the head or tail are already reached because that is
    // where we begin reading the log

    { // append to log
        QVector<LogEntry> chunk = d->readEntries(JournaldViewModelPrivate::Direction::TOWARDS_TAIL);
        if (chunk.size() > 0) {
            beginInsertRows(QModelIndex(), d->mLog.size(), d->mLog.size() + chunk.size() - 1);
            d->mLog.append(chunk);
            endInsertRows();
            qCDebug(journald) << "read towards tail" << chunk.size();
        }
    }

    { // prepend to log
        QVector<LogEntry> chunk = d->readEntries(JournaldViewModelPrivate::Direction::TOWARDS_HEAD);
        if (chunk.size() > 0) {
            beginInsertRows(QModelIndex(), 0, chunk.size() - 1);
            d->mLog = chunk << d->mLog; // TODO find more performant way than constructing a new vector every time
            endInsertRows();
            qCDebug(journald) << "read towards head" << chunk.size();
        }
    }
    d->mActiveFetchOperations = 0;
}

void JournaldViewModel::seekHead()
{
    beginResetModel();
    d->mLog.clear();
    if (d->mJournal && d->mJournal->isValid()) {
        d->seekHeadAndMakeCurrent();
        QVector<LogEntry> chunk = d->readEntries(JournaldViewModelPrivate::Direction::TOWARDS_TAIL);
        d->mLog = chunk;
    } else {
        qCCritical(journald()) << "Cannot seek head of invalid journal";
    }
    endResetModel();
}

void JournaldViewModel::seekTail()
{
    beginResetModel();
    d->mLog.clear();
    if (d->mJournal && d->mJournal->isValid()) {
        d->seekTailAndMakeCurrent();
        QVector<LogEntry> chunk = d->readEntries(JournaldViewModelPrivate::Direction::TOWARDS_HEAD);
        d->mLog = chunk;
    } else {
        qCCritical(journald()) << "Cannot seek head of invalid journal";
    }
    endResetModel();
}

void JournaldViewModel::setSystemdUnitFilter(const QStringList &systemdUnitFilter)
{
    beginResetModel();
    d->mSystemdUnitFilter = systemdUnitFilter;
    d->resetJournal();
    fetchMoreLogEntries();
    endResetModel();
}

void JournaldViewModel::setBootFilter(const QStringList &bootFilter)
{
    beginResetModel();
    d->mBootFilter = bootFilter;
    d->resetJournal();
    fetchMoreLogEntries();
    endResetModel();
}

void JournaldViewModel::setExeFilter(const QStringList &exeFilter)
{
    beginResetModel();
    d->mExeFilter = exeFilter;
    d->resetJournal();
    fetchMoreLogEntries();
    endResetModel();
}

void JournaldViewModel::setPriorityFilter(int priority)
{
    if (d->mPriorityFilter.has_value() && d->mPriorityFilter.value() == priority) {
        return;
    }
    beginResetModel();
    d->mPriorityFilter = priority;
    d->resetJournal();
    fetchMoreLogEntries();
    endResetModel();
}

void JournaldViewModel::resetPriorityFilter()
{
    beginResetModel();
    d->mPriorityFilter.reset();
    d->resetJournal();
    fetchMoreLogEntries();
    endResetModel();
}

void JournaldViewModel::setKernelFilter(bool showKernelMessages)
{
    if (d->mShowKernelMessages == showKernelMessages) {
        return;
    }
    beginResetModel();
    d->mShowKernelMessages = showKernelMessages;
    d->resetJournal();
    fetchMoreLogEntries();
    endResetModel();
    Q_EMIT kernelFilterChanged();
}

bool JournaldViewModel::isKernelFilterEnabled() const
{
    return d->mShowKernelMessages;
}

int JournaldViewModel::search(const QString &searchString, int startRow)
{
    int row = startRow;
    while (row < d->mLog.size()) {
        if (d->mLog.at(row).mMessage.contains(searchString)) {
            return row;
        }
        ++row;
        if (row == d->mLog.size() && canFetchMore(QModelIndex())) { // if end is reached, try to fetch more
            fetchMore(QModelIndex());
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
