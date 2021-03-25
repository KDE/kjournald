/*
    SPDX-License-Identifier: LGPL-2.1-or-later OR MIT
    SPDX-FileCopyrightText: 2021 Andreas Cord-Landwehr <cordlandwehr@kde.org>
*/

#include "journaldviewmodel.h"
#include "journaldviewmodel_p.h"
#include "localjournal.h"
#include "loggingcategories.h"
#include <QColor>
#include <QDebug>
#include <QDir>
#include <QRandomGenerator>

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
    for (const QString &boot : mBootFilter) {
        QString filterExpression = "_BOOT_ID=" + boot;
        result = sd_journal_add_match(mJournal->sdJournal(), filterExpression.toStdString().c_str(), 0);
        if (result < 0) {
            qCCritical(journald) << "Failed to set journal filter:" << strerror(-result) << filterExpression;
        }
    }
    if (mPriorityFilter.has_value()) {
        for (int i = 0; i <= mPriorityFilter; ++i) {
            QString filterExpression = "PRIORITY=" + QString::number(i);
            result = sd_journal_add_match(mJournal->sdJournal(), filterExpression.toStdString().c_str(), 0);
            if (result < 0) {
                qCCritical(journald()) << "Failed to set journal filter:" << strerror(-result) << filterExpression;
            }
        }
    }

    // boot and priority filter shall always be enforced
    result = sd_journal_add_conjunction(mJournal->sdJournal());

    // see journal-fields documentation regarding list of valid transports
    QStringList kernelTransports{"audit", "driver", "kernel"};
    QStringList nonKernelTransports{"syslog", "journal", "stdout"};
    if (mShowKernelMessages) {
        for (const QString &transport : kernelTransports) {
            QString filterExpression = "_TRANSPORT=" + transport;
            result = sd_journal_add_match(mJournal->sdJournal(), filterExpression.toStdString().c_str(), 0);
            if (result < 0) {
                qCCritical(journald) << "Failed to set journal filter:" << strerror(-result) << filterExpression;
            }
        }
    }
    result = sd_journal_add_disjunction(mJournal->sdJournal());

    // special case handling where for messages that are missing a _TRANSPORT entry and otherwise might be missing in log output
    if (!mShowKernelMessages) {
        for (const QString &transport : nonKernelTransports) {
            QString filterExpression = "_TRANSPORT=" + transport;
            result = sd_journal_add_match(mJournal->sdJournal(), filterExpression.toStdString().c_str(), 0);
            if (result < 0) {
                qCCritical(journald) << "Failed to set journal filter:" << strerror(-result) << filterExpression;
            }
        }
    }

    // filter units
    for (const QString &unit : mSystemdUnitFilter) {
        QString filterExpression = "_SYSTEMD_UNIT=" + unit;
        result = sd_journal_add_match(mJournal->sdJournal(), filterExpression.toStdString().c_str(), 0);
        if (result < 0) {
            qCCritical(journald) << "Failed to set journal filter:" << strerror(-result) << filterExpression;
        }
    }

    result = sd_journal_seek_head(mJournal->sdJournal());
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
        d->seekHead();
        fetchMore(QModelIndex());
    }
    endResetModel();
    connect(d->mJournal.get(), &IJournal::journalUpdated, this, [=]() {
        d->canFetchMore = true;
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
    roles[JournaldViewModel::MESSAGE_ID] = "id";
    roles[JournaldViewModel::MESSAGE] = "message";
    roles[JournaldViewModel::PRIORITY] = "priority";
    roles[JournaldViewModel::SYSTEMD_UNIT] = "systemdunit";
    roles[JournaldViewModel::BOOT_ID] = "bootid";
    roles[JournaldViewModel::UNIT_COLOR] = "unitcolor";
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
    case JournaldViewModel::Roles::MESSAGE_ID:
        return QString(d->mLog.at(index.row()).mId);
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
    if (!d->mJournal->isValid()) {
        qCWarning(journald) << "Skipping data fetch, no valid journal opened";
        return;
    }
    Q_UNUSED(parent);
    qCDebug(journald) << "Fetch more data";
    const int readChunkSize{500};
    QVector<LogEntry> chunk;
    for (int counter = 0; counter < readChunkSize; ++counter) {
        // obtain more data, 1 for success, 0 if reached end
        if (sd_journal_next(d->mJournal->sdJournal()) <= 0) {
            d->canFetchMore = false;
            break;
        }
        const char *data;
        size_t length;
        uint64_t time;
        int result{1};

        LogEntry entry;
        result = sd_journal_get_realtime_usec(d->mJournal->sdJournal(), &time);
        if (result == 0) {
            entry.mDate.setMSecsSinceEpoch(time / 1000);
        }

        result = sd_journal_get_data(d->mJournal->sdJournal(), "MESSAGE", (const void **)&data, &length);
        if (result == 0) {
            entry.mMessage = QString::fromUtf8((const char *)data, length).section(QChar::fromLatin1('='), 1);
        }
        result = sd_journal_get_data(d->mJournal->sdJournal(), "MESSAGE_ID", (const void **)&data, &length);
        if (result == 0) {
            entry.mId = QString::fromUtf8((const char *)data, length).section(QChar::fromLatin1('='), 1);
        }
        result = sd_journal_get_data(d->mJournal->sdJournal(), "_SYSTEMD_UNIT", (const void **)&data, &length);
        if (result == 0) {
            entry.mSystemdUnit = QString::fromUtf8((const char *)data, length).section(QChar::fromLatin1('='), 1);
        }
        result = sd_journal_get_data(d->mJournal->sdJournal(), "_BOOT_ID", (const void **)&data, &length);
        if (result == 0) {
            entry.mBootId = QString::fromUtf8((const char *)data, length).section(QChar::fromLatin1('='), 1);
        }
        result = sd_journal_get_data(d->mJournal->sdJournal(), "PRIORITY", (const void **)&data, &length);
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
