/*
    SPDX-License-Identifier: LGPL-2.1-or-later OR MIT
    SPDX-FileCopyrightText: Andreas Cord-Landwehr <cordlandwehr@kde.org>
*/

#include "journalduniquequerymodel.h"
#include "journalduniquequerymodel_p.h"
#include "loggingcategories.h"
#include <QDebug>
#include <QDir>

JournaldUniqueQueryModelPrivate::~JournaldUniqueQueryModelPrivate()
{
    sd_journal_close(mJournal);
    mJournal = nullptr;
}

void JournaldUniqueQueryModelPrivate::closeJournal()
{
    if (mJournal) {
        sd_journal_close(mJournal);
        mJournal = nullptr;
    }
}

bool JournaldUniqueQueryModelPrivate::openJournal()
{
    closeJournal();
    //TODO allow custom selection of journal type
    int result = sd_journal_open(&mJournal, SD_JOURNAL_LOCAL_ONLY);
    if (result < 0) {
        qCCritical(journald) << "Could not open journal:" << strerror(-result);
        return false;
    }
    return true;
}

bool JournaldUniqueQueryModelPrivate::openJournalFromPath(const QString &journalPath)
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

void JournaldUniqueQueryModelPrivate::runQuery()
{
    if (!mJournal || mFieldString.isEmpty()) {
        return;
    }
    mEntries.clear();

    QVector<QString> dataList;
    const void *data;
    size_t length;
    int result = sd_journal_query_unique(mJournal, mFieldString.toStdString().c_str());
    if (result < 0) {
        qCritical() << "Failed to query journal:" << strerror(-result);
        return;
    }
    const int fieldLength = mFieldString.length() + 1;
    SD_JOURNAL_FOREACH_UNIQUE(mJournal, data, length) {
        QString dataStr = static_cast<const char*>(data);
        dataList << dataStr.remove(0, fieldLength);
    }
    mEntries = dataList;
}

JournaldUniqueQueryModel::JournaldUniqueQueryModel(QObject *parent)
    : QAbstractItemModel(parent)
    , d(new JournaldUniqueQueryModelPrivate)
{
    beginResetModel();
    d->openJournal();
    d->runQuery();
    endResetModel();
}

JournaldUniqueQueryModel::JournaldUniqueQueryModel(const QString &journalPath, QObject *parent)
    : QAbstractItemModel(parent)
    , d(new JournaldUniqueQueryModelPrivate)
{
    beginResetModel();
    d->openJournalFromPath(journalPath);
    d->runQuery();
    endResetModel();
}

JournaldUniqueQueryModel::~JournaldUniqueQueryModel() = default;

void JournaldUniqueQueryModel::setJournaldPath(const QString &path)
{
    beginResetModel();
    d->openJournalFromPath(path);
    d->runQuery();
    endResetModel();
}

void JournaldUniqueQueryModel::setField(const QString &fieldString)
{
    beginResetModel();
    d->mFieldString = fieldString;
    d->runQuery();
    endResetModel();
}

QHash<int,QByteArray> JournaldUniqueQueryModel::roleNames() const
{
    QHash<int,QByteArray> roles;
    roles[JournaldUniqueQueryModel::FIELD] = "field";
    return roles;
}

void JournaldUniqueQueryModel::loadSystemJournal()
{
    beginResetModel();
    d->openJournal();
    endResetModel();
}

QModelIndex JournaldUniqueQueryModel::index(int row, int column, const QModelIndex &parent) const
{
    return createIndex(row, column);
}

QModelIndex JournaldUniqueQueryModel::parent(const QModelIndex &index) const
{
    // no tree model, thus no parent
    return QModelIndex();
}

int JournaldUniqueQueryModel::rowCount(const QModelIndex &parent) const
{
    return d->mEntries.size();
}

int JournaldUniqueQueryModel::columnCount(const QModelIndex &parent) const
{
    return 1;
}

QVariant JournaldUniqueQueryModel::data(const QModelIndex &index, int role) const
{
    if (d->mEntries.count() <= index.row()) {
        return QVariant();
    }
    switch (role) {
    case Qt::DisplayRole:
        Q_FALLTHROUGH();
    case JournaldUniqueQueryModel::Roles::FIELD:
        return d->mEntries.at(index.row());
    }
    return QVariant();
}
