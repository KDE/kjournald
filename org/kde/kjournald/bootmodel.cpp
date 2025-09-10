/*
    SPDX-License-Identifier: LGPL-2.1-or-later OR MIT
    SPDX-FileCopyrightText: 2021 Andreas Cord-Landwehr <cordlandwehr@kde.org>
*/

#include "bootmodel.h"
#include "bootmodel_p.h"
#include "kjournaldlib_log_general.h"

BootModel::BootModel(QObject *parent)
    : QAbstractListModel(parent)
    , d(new BootModelPrivate)
{
}

BootModel::~BootModel() = default;

void BootModel::setJournalProvider(IJournalProvider *provider)
{
    d->mJournalProvider = provider;
    if (provider) {
        d->mJournal = provider->openJournal();
    } else {
        d->mJournal.reset();
    }
    Q_EMIT journalProviderChanged();
    beginResetModel();
    d->mBootInfo.clear();
    if (d->mJournal) {
        d->mBootInfo = JournaldHelper::queryOrderedBootIds(d->mJournal->get());
        d->sort(Qt::SortOrder::DescendingOrder);
        endResetModel();
    }
}

IJournalProvider *BootModel::journalProvider() const
{
    return d->mJournalProvider;
}

QHash<int, QByteArray> BootModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[BootModel::BOOT_ID] = "bootid";
    roles[BootModel::CURRENT] = "current";
    roles[BootModel::SINCE] = "since";
    roles[BootModel::UNTIL] = "until";
    roles[BootModel::DISPLAY_SHORT_UTC] = "displayshort_utc";
    roles[BootModel::DISPLAY_SHORT_LOCALTIME] = "displayshort_localtime";
    return roles;
}

int BootModel::rowCount(const QModelIndex &parent) const
{
    return d->mBootInfo.size();
}

QVariant BootModel::data(const QModelIndex &index, int role) const
{
    if (index.row() < 0 || index.row() >= d->mBootInfo.size()) {
        return QVariant();
    }
    switch (role) {
    case BootModel::BOOT_ID:
        return d->mBootInfo.at(index.row()).mBootId;
    case BootModel::SINCE:
        return d->mBootInfo.at(index.row()).mSince;
    case BootModel::UNTIL:
        return d->mBootInfo.at(index.row()).mUntil;
    case BootModel::DISPLAY_SHORT_UTC:
        return d->prettyPrintBoot(d->mBootInfo.at(index.row()), BootModelPrivate::TIME_FORMAT::UTC);
    case BootModel::DISPLAY_SHORT_LOCALTIME:
        return d->prettyPrintBoot(d->mBootInfo.at(index.row()), BootModelPrivate::TIME_FORMAT::LOCALTIME);
    case BootModel::CURRENT:
        return QVariant::fromValue<bool>(d->mJournalProvider->currentBootId() == d->mBootInfo.at(index.row()).mBootId);
    }

    return QVariant();
}
