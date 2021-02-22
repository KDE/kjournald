/*
    SPDX-License-Identifier: LGPL-2.1-or-later OR MIT
    SPDX-FileCopyrightText: 2021 Andreas Cord-Landwehr <cordlandwehr@kde.org>
*/

#include "bootmodel.h"
#include "bootmodel_p.h"

BootModelPrivate::BootModelPrivate(std::unique_ptr<Journal> journal)
    : mJournal(std::move(journal))
{
}

BootModel::BootModel(QObject *parent)
    : QAbstractItemModel(parent)
    , d(new BootModelPrivate(std::make_unique<Journal>()))
{
    beginResetModel();
    d->mBootInfo = JournaldHelper::queryOrderedBootIds(*d->mJournal.get());
    endResetModel();
}

BootModel::BootModel(const QString &journalPath, QObject *parent)
    : QAbstractItemModel(parent)
    , d(new BootModelPrivate(std::make_unique<Journal>(journalPath)))
{
    beginResetModel();
    d->mBootInfo = JournaldHelper::queryOrderedBootIds(*d->mJournal.get());
    endResetModel();
}

BootModel::~BootModel() = default;

QHash<int, QByteArray> BootModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[BootModel::_BOOT_ID] = "_BOOT_ID";
    roles[BootModel::SINCE] = "since";
    roles[BootModel::UNTIL] = "until";
    roles[BootModel::DISPLAY_SHORT] = "displayshort";
    return roles;
}

int BootModel::rowCount(const QModelIndex &parent) const
{
    return d->mBootInfo.size();
}

int BootModel::columnCount(const QModelIndex &parent) const
{
    return 1;
}

QModelIndex BootModel::index(int row, int column, const QModelIndex &parent) const
{
    return createIndex(row, column);
}

QModelIndex BootModel::parent(const QModelIndex &index) const
{
    // no tree model, thus no parent
    return QModelIndex();
}

QVariant BootModel::data(const QModelIndex &index, int role) const
{
    if (index.row() < 0 || index.row() >= d->mBootInfo.size()) {
        return QVariant();
    }
    switch (role) {
    case Qt::DisplayRole:
        Q_FALLTHROUGH();
    case BootModel::_BOOT_ID:
        return d->mBootInfo.at(index.row()).mBootId;
    case BootModel::SINCE:
        return d->mBootInfo.at(index.row()).mSince;
    case BootModel::UNTIL:
        return d->mBootInfo.at(index.row()).mUntil;
    case BootModel::DISPLAY_SHORT:
        const QString sinceTime = d->mBootInfo.at(index.row()).mSince.toString("hh:mm");
        const QString sinceDate = d->mBootInfo.at(index.row()).mSince.toString("yyyy-MM-dd");
        const QString untilTime = d->mBootInfo.at(index.row()).mUntil.toString("hh:mm");
        const QString id = d->mBootInfo.at(index.row()).mBootId.left(10);
        return QString("%1 %2-%3 [%4...]").arg(sinceDate).arg(sinceTime).arg(untilTime).arg(id);
    }

    return QVariant();
}

QString BootModel::bootId(int row) const
{
    if (row < 0 || row >= d->mBootInfo.size()) {
        return QString();
    }
    return d->mBootInfo.at(row).mBootId;
}
