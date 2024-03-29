/*
    SPDX-License-Identifier: LGPL-2.1-or-later OR MIT
    SPDX-FileCopyrightText: 2021 Andreas Cord-Landwehr <cordlandwehr@kde.org>
*/

#include "journalduniquequerymodel.h"
#include "journalduniquequerymodel_p.h"
#include "kjournald_export.h"
#include "kjournaldlib_log_general.h"
#include <QDebug>
#include <QDir>
#include <QString>
#include <memory>

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
    // TODO allow custom selection of journal type
    int result = sd_journal_open(&mJournal, SD_JOURNAL_LOCAL_ONLY);
    if (result < 0) {
        qCCritical(KJOURNALDLIB_GENERAL) << "Could not open journal:" << strerror(-result);
        return false;
    }
    return true;
}

bool JournaldUniqueQueryModelPrivate::openJournalFromPath(const QString &path)
{
    closeJournal();
    if (path.isEmpty() || !QDir().exists(path)) {
        qCCritical(KJOURNALDLIB_GENERAL) << "Journal directory does not exist, abort opening";
        return false;
    }
    const QFileInfo fileInfo = QFileInfo(path);
    if (fileInfo.isDir()) {
        int result = sd_journal_open_directory(&mJournal, path.toStdString().c_str(), 0 /* no flags, directory defines type */);
        if (result < 0) {
            qCCritical(KJOURNALDLIB_GENERAL) << "Could not open journal:" << strerror(-result);
            return false;
        }
    } else if (fileInfo.isFile()) {
        const char **files = new const char *[1];
        QByteArray journalPath = path.toLocal8Bit();
        files[0] = journalPath.data();

        int result = sd_journal_open_files(&mJournal, files, 0 /* no flags, directory defines type */);
        delete[] files;
        if (result < 0) {
            qCCritical(KJOURNALDLIB_GENERAL) << "Could not open journal:" << strerror(-result);
            return false;
        }
    }

    return true;
}

void JournaldUniqueQueryModelPrivate::runQuery()
{
    if (!mJournal || mFieldString.isEmpty()) {
        return;
    }
    mEntries.clear();

    QVector<std::pair<QString, bool>> dataList;
    const void *data;
    size_t length;
    int result = sd_journal_query_unique(mJournal, mFieldString.toStdString().c_str());
    if (result < 0) {
        qCritical() << "Failed to query journal:" << strerror(-result);
        return;
    }
    const int fieldLength = mFieldString.length() + 1;
    SD_JOURNAL_FOREACH_UNIQUE(mJournal, data, length)
    {
        QString dataStr = QString::fromLocal8Bit(static_cast<const char *>(data));
        dataStr = dataStr.remove(0, fieldLength);
        if (dataStr.endsWith(QLatin1String("\u0001"))) {
            dataStr = dataStr.left(dataStr.length() - QString(QLatin1String("\u0001")).length());
        }
        if (dataStr.endsWith(QLatin1String("\u0002"))) {
            dataStr = dataStr.left(dataStr.length() - QString(QLatin1String("\u0002")).length());
        }
        dataStr = JournaldHelper::cleanupString(dataStr);
        dataList << std::pair<QString, bool>{dataStr, true};
    }

    mEntries = dataList;
}

JournaldUniqueQueryModel::JournaldUniqueQueryModel(QObject *parent)
    : QAbstractItemModel(parent)
    , d(new JournaldUniqueQueryModelPrivate)
{
    d->openJournal();
    d->runQuery();
}

JournaldUniqueQueryModel::JournaldUniqueQueryModel(const QString &journalPath, QObject *parent)
    : QAbstractItemModel(parent)
    , d(new JournaldUniqueQueryModelPrivate)
{
    d->openJournalFromPath(journalPath);
    d->runQuery();
}

JournaldUniqueQueryModel::~JournaldUniqueQueryModel() = default;

bool JournaldUniqueQueryModel::setJournaldPath(const QString &path)
{
    bool success{true};
    beginResetModel();
    success = d->openJournalFromPath(path);
    if (success) {
        d->runQuery();
    }
    endResetModel();
    return success;
}

void JournaldUniqueQueryModel::setField(JournaldHelper::Field field)
{
    setFieldString(JournaldHelper::mapField(field));
}

void JournaldUniqueQueryModel::setFieldString(const QString &fieldString)
{
    beginResetModel();
    d->mFieldString = fieldString;
    d->runQuery();
    endResetModel();
}

QString JournaldUniqueQueryModel::fieldString() const
{
    return d->mFieldString;
}

QHash<int, QByteArray> JournaldUniqueQueryModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[JournaldUniqueQueryModel::FIELD] = "field";
    roles[JournaldUniqueQueryModel::SELECTED] = "selected";
    return roles;
}

void JournaldUniqueQueryModel::setSystemJournal()
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
    // model represents a list and has has no children
    if (!parent.isValid()) {
        return d->mEntries.size();
    } else {
        return 0;
    }
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
        return d->mEntries.at(index.row()).first;
    case JournaldUniqueQueryModel::Roles::SELECTED:
        return QVariant::fromValue<bool>(d->mEntries.at(index.row()).second);
    }
    return QVariant();
}

bool JournaldUniqueQueryModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (d->mEntries.count() <= index.row()) {
        return false;
    }
    if (role == JournaldUniqueQueryModel::Roles::SELECTED) {
        if (d->mEntries.at(index.row()).second == value.toBool()) {
            return false;
        } else {
            d->mEntries[index.row()].second = value.toBool();
            Q_EMIT dataChanged(index, index);
            return true;
        }
    }
    return QAbstractItemModel::setData(index, value, role);
}
