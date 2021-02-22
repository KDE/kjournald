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
    if (!QDir().exists(journalPath)) {
        qCCritical(journald) << "Journal directory does not exists, abort opening";
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

JournaldViewModel::JournaldViewModel(QObject *parent)
    : QAbstractItemModel(parent)
    , d(new JournaldViewModelPrivate)
{
    beginResetModel();
    d->openJournal();
    endResetModel();
}

JournaldViewModel::JournaldViewModel(const QString &journalPath, QObject *parent)
    : QAbstractItemModel(parent)
    , d(new JournaldViewModelPrivate)
{
    beginResetModel();
    d->openJournalFromPath(journalPath);
    endResetModel();
}

JournaldViewModel::~JournaldViewModel() = default;

void JournaldViewModel::setJournaldPath(const QString &path)
{
    beginResetModel();
    d->openJournalFromPath(path);
    seekHead();
    fetchMore(QModelIndex());
    endResetModel();
}

void JournaldViewModel::seekHead()
{
    int result{0};

    // reset all filters
    sd_journal_flush_matches(d->mJournal);

    for (const QString &unit : d->mSystemdUnitFilter) {
        QString filterExpression = "_SYSTEMD_UNIT=" + unit;
        result = sd_journal_add_match(d->mJournal, filterExpression.toStdString().c_str(), 0);
        if (result < 0) {
            qCritical() << "Failed to set journal filter:" << strerror(-result) << filterExpression;
        }
    }
    for (const QString &boot : d->mBootFilter) {
        QString filterExpression = "_BOOT_ID=" + boot;
        result = sd_journal_add_match(d->mJournal, filterExpression.toStdString().c_str(), 0);
        if (result < 0) {
            qCritical() << "Failed to set journal filter:" << strerror(-result) << filterExpression;
        }
    }

    result = sd_journal_seek_head(d->mJournal);
    if (result < 0) {
        qCritical() << "Failed to seek head:" << strerror(-result);
        return;
    }

    // clear all data which are in limbo with new head
    d->mLog.clear();
}

QHash<int, QByteArray> JournaldViewModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[JournaldViewModel::DATE] = "date";
    roles[JournaldViewModel::MESSAGE] = "message";
    roles[JournaldViewModel::PRIORITY] = "priority";
    roles[JournaldViewModel::_SYSTEMD_UNIT] = "systemdunit";
    roles[JournaldViewModel::_BOOT_ID] = "bootid";
    roles[JournaldViewModel::UNIT_COLOR] = "unitcolor";
    return roles;
}

void JournaldViewModel::loadSystemJournal()
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
        return d->mLog.at(index.row()).mMessage;
    case JournaldViewModel::Roles::DATE:
        return d->mLog.at(index.row()).mDate;
    case JournaldViewModel::Roles::_BOOT_ID:
        return d->mLog.at(index.row()).mBootId;
    case JournaldViewModel::Roles::_SYSTEMD_UNIT:
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
    // TODO
    return true;
}

void JournaldViewModel::fetchMore(const QModelIndex &parent)
{
    Q_UNUSED(parent);
    const int readChunkSize{500};
    int counter = 0;
    QVector<LogEntry> chunk;
    while (sd_journal_next(d->mJournal) > 0 && counter < readChunkSize) {
        ++counter;
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
    d->mSystemdUnitFilter = systemdUnitFilter;
    beginResetModel();
    seekHead();
    endResetModel();
}

void JournaldViewModel::setBootFilter(const QStringList &bootFilter)
{
    d->mBootFilter = bootFilter;
    beginResetModel();
    seekHead();
    endResetModel();
}
