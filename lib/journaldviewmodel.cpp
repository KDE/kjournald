/*
    SPDX-License-Identifier: LGPL-2.1-or-later OR MIT
    SPDX-FileCopyrightText: 2021 Andreas Cord-Landwehr <cordlandwehr@kde.org>
*/

#include "journaldviewmodel.h"
#include "journaldviewmodel_p.h"
#include "loggingcategories.h"
#include <QDebug>
#include <QDir>
#include <QColor>
#include <QRandomGenerator>

JournaldViewModelPrivate::~JournaldViewModelPrivate()
{
    sd_journal_close(mJournal);
    mJournal = nullptr;
}

void JournaldViewModelPrivate::closeJournal()
{
    if (mJournal) {
        sd_journal_close(mJournal);
        mJournal = nullptr;
    }
}

bool JournaldViewModelPrivate::openJournal()
{
    closeJournal();
    // TODO allow custom selection of journal type
    int result = sd_journal_open(&mJournal, SD_JOURNAL_LOCAL_ONLY);
    if (result < 0) {
        qCCritical(journald) << "Could not open journal:" << strerror(-result);
        return false;
    }
    return true;
}

bool JournaldViewModelPrivate::openJournalFromPath(const QString &journalPath)
{
    closeJournal();
    qCDebug(journald) << "Open journal from path:" << journalPath;
    if (!QDir().exists(journalPath)) {
        qCCritical(journald) << "Journal directory does not exist, abort opening";
        return false;
    }
    int result = sd_journal_open_directory(&mJournal, journalPath.toStdString().c_str(), 0 /* no flags, directory defines type */);
    if (result < 0) {
        qCCritical(journald) << "Could not open journal:" << strerror(-result);
        return false;
    }

    return true;
}

QColor JournaldViewModelPrivate::unitColor(const QString &unit)
{
    if (!mUnitToColorMap.contains(unit)) {
        int hue = QRandomGenerator::global()->bounded(255);
        mUnitToColorMap[unit] = QColor::fromHsl(hue, 150, 220);
    }
    return mUnitToColorMap.value(unit);
}


void JournaldViewModelPrivate::seekHead()
{
    int result{ 0 };

    // reset all filters
    sd_journal_flush_matches(mJournal);

    for (const QString &unit : mSystemdUnitFilter) {
        QString filterExpression = "_SYSTEMD_UNIT=" + unit;
        result = sd_journal_add_match(mJournal, filterExpression.toStdString().c_str(), 0);
        if (result < 0) {
            qCCritical(journald) << "Failed to set journal filter:" << strerror(-result) << filterExpression;
        }
    }
    for (const QString &boot : mBootFilter) {
        QString filterExpression = "_BOOT_ID=" + boot;
        result = sd_journal_add_match(mJournal, filterExpression.toStdString().c_str(), 0);
        if (result < 0) {
            qCCritical(journald) << "Failed to set journal filter:" << strerror(-result) << filterExpression;
        }
    }
    if (mPriorityFilter.has_value()) {
        for (int i = 0; i <= mPriorityFilter; ++i) {
            QString filterExpression = "PRIORITY=" + QString::number(i);
            result = sd_journal_add_match(mJournal, filterExpression.toStdString().c_str(), 0);
            if (result < 0) {
                qCCritical(journald()) << "Failed to set journal filter:" << strerror(-result) << filterExpression;
            }
        }
    }
    if (mShowKernelMessages) {
        result = sd_journal_add_disjunction(mJournal);
        result = sd_journal_add_match(mJournal, "_TRANSPORT=kernel", 0);
    }

    result = sd_journal_seek_head(mJournal);
    if (result < 0) {
        qCCritical(journald) << "Failed to seek head:" << strerror(-result);
        return;
    }

    canFetchMore = true;
    // clear all data which are in limbo with new head
    mLog.clear();
}

JournaldViewModel::JournaldViewModel(QObject *parent)
    : QAbstractItemModel(parent)
    , d(new JournaldViewModelPrivate)
{
    beginResetModel();
    d->openJournal();
    endResetModel();
}

JournaldViewModel::JournaldViewModel(const QString &path, QObject *parent)
    : QAbstractItemModel(parent)
    , d(new JournaldViewModelPrivate)
{
    setJournaldPath(path);
}

JournaldViewModel::~JournaldViewModel() = default;

bool JournaldViewModel::setJournaldPath(const QString &path)
{
    bool success{ true };
    beginResetModel();
    success = d->openJournalFromPath(path);
    if (success) {
        d->seekHead();
        fetchMore(QModelIndex());
    }
    endResetModel();
    d->mJournalPath = path;
    Q_EMIT journaldPathChanged();
    return success;
}

QString JournaldViewModel::journaldPath() const
{
    return d->mJournalPath;
}

QHash<int, QByteArray> JournaldViewModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[JournaldViewModel::DATE] = "date";
    roles[JournaldViewModel::MESSAGE] = "message";
    roles[JournaldViewModel::PRIORITY] = "priority";
    roles[JournaldViewModel::SYSTEMD_UNIT] = "systemdunit";
    roles[JournaldViewModel::BOOT_ID] = "bootid";
    roles[JournaldViewModel::UNIT_COLOR] = "unitcolor";
    return roles;
}

void JournaldViewModel::setSystemJournal()
{
    beginResetModel();
    d->openJournal();
    endResetModel();
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
        return QString(d->mLog.at(index.row()).mMessage).remove("\u001B[96m").remove("\u001B[0m").remove("\u001B[93m").remove("\u001B[31m");
    case JournaldViewModel::Roles::DATE:
        return d->mLog.at(index.row()).mDate;
    case JournaldViewModel::Roles::BOOT_ID:
        return d->mLog.at(index.row()).mBootId;
    case JournaldViewModel::Roles::SYSTEMD_UNIT:
        return d->mLog.at(index.row()).mSystemdUnit;
    case JournaldViewModel::Roles::PRIORITY:
        return d->mLog.at(index.row()).mPriority;
    case JournaldViewModel::Roles::UNIT_COLOR:
        return d->unitColor(d->mLog.at(index.row()).mSystemdUnit);
    }
    return QVariant();
}

bool JournaldViewModel::canFetchMore(const QModelIndex &parent) const
{
    return d->canFetchMore;
}

void JournaldViewModel::fetchMore(const QModelIndex &parent)
{
    Q_UNUSED(parent);
    qCDebug(journald) << "Fetch more data";
    const int readChunkSize{500};
    QVector<LogEntry> chunk;
    for (int counter = 0; counter < readChunkSize; ++counter) {
        // obtain more data, 1 for success, 0 if reached end
        if (sd_journal_next(d->mJournal) <= 0) {
            d->canFetchMore = false;
            break;
        }
        const char *data;
        size_t length;
        uint64_t time;
        int result{1};

        LogEntry entry;
        result = sd_journal_get_realtime_usec(d->mJournal, &time);
        if (result == 0) {
            entry.mDate.setMSecsSinceEpoch(time / 1000);
        }

        result = sd_journal_get_data(d->mJournal, "MESSAGE", (const void **)&data, &length);
        if (result == 0) {
            entry.mMessage = QString::fromUtf8((const char *)data, length).section(QChar::fromLatin1('='), 1);
        }
        result = sd_journal_get_data(d->mJournal, "_SYSTEMD_UNIT", (const void **)&data, &length);
        if (result == 0) {
            entry.mSystemdUnit = QString::fromUtf8((const char *)data, length).section(QChar::fromLatin1('='), 1);
        }
        result = sd_journal_get_data(d->mJournal, "_BOOT_ID", (const void **)&data, &length);
        if (result == 0) {
            entry.mBootId = QString::fromUtf8((const char *)data, length).section(QChar::fromLatin1('='), 1);
        }
        result = sd_journal_get_data(d->mJournal, "PRIORITY", (const void **)&data, &length);
        if (result == 0) {
            entry.mPriority = QString::fromUtf8((const char *)data, length).section(QChar::fromLatin1('='), 1).toInt();
        }

        chunk.append(std::move(entry));
    }
    beginInsertRows(QModelIndex(), d->mLog.size(), d->mLog.size() + chunk.size());
    d->mLog.append(chunk);
    endInsertRows();
}

void JournaldViewModel::setSystemdUnitFilter(const QStringList &systemdUnitFilter)
{
    beginResetModel();
    d->mSystemdUnitFilter = systemdUnitFilter;
    d->seekHead();
    fetchMore(QModelIndex());
    endResetModel();
}

void JournaldViewModel::setBootFilter(const QStringList &bootFilter)
{
    beginResetModel();
    d->mBootFilter = bootFilter;
    d->seekHead();
    fetchMore(QModelIndex());
    endResetModel();
}

void JournaldViewModel::setPriorityFilter(int priority)
{
    if (d->mPriorityFilter.has_value() && d->mPriorityFilter.value() == priority) {
        return;
    }
    beginResetModel();
    d->mPriorityFilter = priority;
    d->seekHead();
    fetchMore(QModelIndex());
    endResetModel();
}

void JournaldViewModel::resetPriorityFilter()
{
    beginResetModel();
    d->mPriorityFilter.reset();
    d->seekHead();
    fetchMore(QModelIndex());
    endResetModel();
}

void JournaldViewModel::setKernelFilter(bool showKernelMessages)
{
    if (d->mShowKernelMessages == showKernelMessages) {
        return;
    }
    beginResetModel();
    d->mShowKernelMessages = showKernelMessages;
    d->seekHead();
    fetchMore(QModelIndex());
    endResetModel();
    Q_EMIT kernelFilterChanged();
}

bool JournaldViewModel::kernelFilter() const
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
        if (row == d->mLog.size() && d->canFetchMore) { // if end is reached, try to fetch more
             fetchMore(QModelIndex());
        }
    }
    return -1;
}

QString JournaldViewModel::formatTime(const QDateTime &datetime, bool utc) const
{
    if (utc) {
        return datetime.toUTC().time().toString("HH:mm:ss.zzz");
    } else {
        return datetime.time().toString("HH:mm:ss.zzz");
    }
}
