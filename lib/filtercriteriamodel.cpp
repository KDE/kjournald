/*
    SPDX-License-Identifier: LGPL-2.1-or-later OR MIT
    SPDX-FileCopyrightText: 2021 Andreas Cord-Landwehr <cordlandwehr@kde.org>
*/

#include "filtercriteriamodel.h"
#include "filtercriteriamodel_p.h"
#include "kjournald_export.h"
#include "loggingcategories.h"
#include <QDebug>
#include <QDir>
#include <QString>
#include <memory>

constexpr QLatin1String mapPriorityToString(int priority)
{
    switch (priority) {
    case 0:
        return QLatin1String("emergency");
    case 1:
        return QLatin1String("alert");
    case 2:
        return QLatin1String("critical");
    case 3:
        return QLatin1String("error");
    case 4:
        return QLatin1String("warning");
    case 5:
        return QLatin1String("notice");
    case 6:
        return QLatin1String("info");
    case 7:
        return QLatin1String("debug");
    }
    return QLatin1String("");
}

SelectionEntry::SelectionEntry(const QString &text,
                               const QVariant &data,
                               FilterCriteriaModel::Category category,
                               bool selected,
                               std::shared_ptr<SelectionEntry> parent)
    : mText(text)
    , mData(data)
    , mCategory(category)
    , mSelected(selected)
    , mParentItem(parent)
{
}

void SelectionEntry::appendChild(std::shared_ptr<SelectionEntry> item)
{
    mChildItems.push_back(item);
}

std::shared_ptr<SelectionEntry> SelectionEntry::child(int row)
{
    if (row < 0 || row >= mChildItems.size()) {
        return nullptr;
    }
    return mChildItems.at(row);
}

int SelectionEntry::childCount() const
{
    return mChildItems.size();
}

int SelectionEntry::row() const
{
    auto parent = mParentItem.lock();
    if (parent) {
        for (int i = 0; i < parent->mChildItems.size(); ++i) {
            if (parent->mChildItems.at(i).get() == this) {
                return i;
            }
        }
    }
    return 0;
}

int SelectionEntry::columnCount() const
{
    return 1;
}

QVariant SelectionEntry::data(FilterCriteriaModel::Roles role) const
{
    switch (role) {
    case FilterCriteriaModel::Roles::CATEGORY:
        return QVariant::fromValue(mCategory);
    case FilterCriteriaModel::Roles::TEXT:
        return QVariant::fromValue(mText); // TODO abbreviate
    case FilterCriteriaModel::Roles::DATA:
        return QVariant::fromValue(mData);
    case FilterCriteriaModel::Roles::LONGTEXT:
        return QVariant::fromValue(mText);
    case FilterCriteriaModel::Roles::SELECTED:
        return QVariant::fromValue(mSelected);
    }
    return QVariant();
}

bool SelectionEntry::setData(const QVariant &value, FilterCriteriaModel::Roles role)
{
    if (role == FilterCriteriaModel::Roles::SELECTED) {
        mSelected = value.toBool();
        return true;
    }
    qCWarning(journald) << "no settable role";
    return false;
}

std::shared_ptr<SelectionEntry> SelectionEntry::parentItem()
{
    return mParentItem.lock();
}

FilterCriteriaModelPrivate::FilterCriteriaModelPrivate()
{
    rebuildModel();
}

FilterCriteriaModelPrivate::~FilterCriteriaModelPrivate() = default;

void FilterCriteriaModelPrivate::rebuildModel()
{
    mRootItem = std::make_unique<SelectionEntry>();
    {
        auto parent = std::make_shared<SelectionEntry>(QLatin1String("Transport"), QVariant(), FilterCriteriaModel::Category::TRANSPORT, true, mRootItem);
        mRootItem->appendChild(parent);
        mRootItem->child(FilterCriteriaModel::Category::TRANSPORT)
            ->appendChild(std::move(
                std::make_unique<SelectionEntry>(QLatin1String("Kernel"), QLatin1String("kernel"), FilterCriteriaModel::Category::TRANSPORT, false, parent)));
    }
    {
        auto parent = std::make_shared<SelectionEntry>(QLatin1String("Priority"), QVariant(), FilterCriteriaModel::Category::PRIORITY, true, mRootItem);
        mRootItem->appendChild(parent);
        for (int i = 0; i <= 7; ++i) {
            mRootItem->child(FilterCriteriaModel::Category::PRIORITY)
                ->appendChild(std::move(std::make_unique<SelectionEntry>(mapPriorityToString(i),
                                                                         QString::number(i),
                                                                         FilterCriteriaModel::Category::PRIORITY,
                                                                         i == 5 ? true : false, // TODO hardcoded default, should be made configurable
                                                                         parent)));
        }
    }
    {
        auto parent = std::make_shared<SelectionEntry>(QLatin1String("Systemd"), QVariant(), FilterCriteriaModel::Category::SYSTEMD_UNIT, true, mRootItem);
        mRootItem->appendChild(parent);
        QVector<QString> units = JournaldHelper::queryUnique(mJournal, JournaldHelper::Field::SYSTEMD_UNIT);
        std::sort(std::begin(units), std::end(units), [](const QString &a, const QString &b) {
            return QString::compare(a, b, Qt::CaseInsensitive) <= 0;
        });
        for (const auto &unit : std::as_const(units)) {
            mRootItem->child(FilterCriteriaModel::Category::SYSTEMD_UNIT)
                ->appendChild(std::move(
                    std::make_unique<SelectionEntry>(JournaldHelper::cleanupString(unit), unit, FilterCriteriaModel::Category::SYSTEMD_UNIT, true, parent)));
        }
    }
    {
        auto parent = std::make_shared<SelectionEntry>(QLatin1String("Process"), QVariant(), FilterCriteriaModel::Category::EXE, true, mRootItem);
        mRootItem->appendChild(parent);
        QVector<QString> exes = JournaldHelper::queryUnique(mJournal, JournaldHelper::Field::EXE);
        std::sort(std::begin(exes), std::end(exes), [](const QString &a, const QString &b) {
            return QString::compare(a, b, Qt::CaseInsensitive) <= 0;
        });
        for (const auto &exe : std::as_const(exes)) {
            mRootItem->child(FilterCriteriaModel::Category::EXE)
                ->appendChild(
                    std::move(std::make_unique<SelectionEntry>(JournaldHelper::cleanupString(exe), exe, FilterCriteriaModel::Category::EXE, true, parent)));
        }
    }
}

FilterCriteriaModel::FilterCriteriaModel(QObject *parent)
    : QAbstractItemModel(parent)
    , d(new FilterCriteriaModelPrivate)
{
    beginResetModel();
    d->mJournal = std::make_shared<LocalJournal>();
    d->rebuildModel();
    endResetModel();
}

FilterCriteriaModel::FilterCriteriaModel(const QString &journalPath, QObject *parent)
    : QAbstractItemModel(parent)
    , d(new FilterCriteriaModelPrivate)
{
    beginResetModel();
    d->mJournal = std::make_shared<LocalJournal>(journalPath);
    d->rebuildModel();
    endResetModel();
}

FilterCriteriaModel::~FilterCriteriaModel() = default;

bool FilterCriteriaModel::setJournaldPath(const QString &path)
{
    beginResetModel();
    d->mJournal = std::make_shared<LocalJournal>(path);
    bool success = d->mJournal->isValid();
    if (d->mJournal->isValid()) {
        d->rebuildModel();
    }
    endResetModel();
    return success;
}

QHash<int, QByteArray> FilterCriteriaModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[FilterCriteriaModel::TEXT] = "text";
    roles[FilterCriteriaModel::DATA] = "data";
    roles[FilterCriteriaModel::LONGTEXT] = "longtext";
    roles[FilterCriteriaModel::CATEGORY] = "category";
    roles[FilterCriteriaModel::SELECTED] = "selected";
    return roles;
}

void FilterCriteriaModel::setSystemJournal()
{
    beginResetModel();
    d->mJournal = std::make_shared<LocalJournal>();
    d->rebuildModel();
    endResetModel();
}

int FilterCriteriaModel::priorityFilter() const
{
    std::shared_ptr<SelectionEntry> parent = d->mRootItem->child(FilterCriteriaModel::Category::PRIORITY);
    for (int i = 0; i < parent->childCount(); ++i) {
        if (parent->child(i)->data(FilterCriteriaModel::SELECTED).toBool()) {
            return parent->child(i)->data(FilterCriteriaModel::DATA).toInt();
        }
    }
    qCWarning(journald()) << "No priority selected, falling back to 0";
    return 0;
}

QStringList FilterCriteriaModel::systemdUnitFilter() const
{
    std::shared_ptr<SelectionEntry> parent = d->mRootItem->child(FilterCriteriaModel::Category::SYSTEMD_UNIT);
    QStringList entries;
    for (int i = 0; i < parent->childCount(); ++i) {
        if (parent->child(i)->data(FilterCriteriaModel::SELECTED).toBool()) {
            entries.append(parent->child(i)->data(FilterCriteriaModel::DATA).toString());
        }
    }
    return entries;
}

QStringList FilterCriteriaModel::exeFilter() const
{
    std::shared_ptr<SelectionEntry> parent = d->mRootItem->child(FilterCriteriaModel::Category::EXE);
    QStringList entries;
    for (int i = 0; i < parent->childCount(); ++i) {
        if (parent->child(i)->data(FilterCriteriaModel::SELECTED).toBool()) {
            entries.append(parent->child(i)->data(FilterCriteriaModel::DATA).toString());
        }
    }
    return entries;
}

bool FilterCriteriaModel::isKernelFilterEnabled() const
{
    std::shared_ptr<SelectionEntry> parent = d->mRootItem->child(FilterCriteriaModel::Category::TRANSPORT);
    for (int i = 0; i < parent->childCount(); ++i) {
        if (parent->child(i)->data(FilterCriteriaModel::DATA) == QLatin1String("kernel") && parent->child(i)->data(FilterCriteriaModel::SELECTED).toBool()) {
            return true;
        }
    }
    return false;
}

QModelIndex FilterCriteriaModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    SelectionEntry *parentItem;

    if (!parent.isValid()) {
        parentItem = d->mRootItem.get();
    } else {
        parentItem = static_cast<SelectionEntry *>(parent.internalPointer());
    }

    std::shared_ptr<SelectionEntry> childItem = parentItem->child(row);
    if (childItem) {
        return createIndex(row, column, childItem.get());
    }
    return QModelIndex();
}

QModelIndex FilterCriteriaModel::parent(const QModelIndex &index) const
{
    if (!index.isValid()) {
        return QModelIndex();
    }

    SelectionEntry *childItem = static_cast<SelectionEntry *>(index.internalPointer());
    std::shared_ptr<SelectionEntry> parentItem = childItem->parentItem();

    if (parentItem.get() == d->mRootItem.get()) {
        return QModelIndex();
    }

    return createIndex(parentItem->row(), 0, parentItem.get());
}

int FilterCriteriaModel::rowCount(const QModelIndex &parent) const
{
    if (!parent.isValid()) {
        return d->mRootItem->childCount();
    } else {
        auto parentItem = static_cast<SelectionEntry *>(parent.internalPointer());
        return parentItem->childCount();
    }
    return 0;
}

int FilterCriteriaModel::columnCount(const QModelIndex &parent) const
{
    return 1;
}

QVariant FilterCriteriaModel::data(const QModelIndex &index, int role) const
{
    if (!index.parent().isValid()) {
        if (index.row() < 0 || index.row() >= d->mRootItem->childCount()) {
            qCCritical(journald()) << "Index out of range" << index;
            return QVariant();
        }
        return d->mRootItem->child(index.row())->data(static_cast<FilterCriteriaModel::Roles>(role));
    } else {
        auto entry = static_cast<SelectionEntry *>(index.internalPointer());
        if (nullptr != entry) {
            return entry->data(static_cast<FilterCriteriaModel::Roles>(role));
        }
    }
    return QVariant();
}

bool FilterCriteriaModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    auto entry = static_cast<SelectionEntry *>(index.internalPointer());
    if (nullptr != entry) {
        if (value == entry->data(static_cast<FilterCriteriaModel::Roles>(role))) {
            return true; // nothing to do
        }
        const bool result = entry->setData(value, static_cast<FilterCriteriaModel::Roles>(role));
        const auto category = entry->data(FilterCriteriaModel::Roles::CATEGORY).value<FilterCriteriaModel::Category>();

        // emit change only one entry is activated
        if (result && category == FilterCriteriaModel::Category::PRIORITY && static_cast<FilterCriteriaModel::Roles>(role) == SELECTED && result) {
            Q_EMIT priorityFilterChanged(index.row());
        } else if (result && category == FilterCriteriaModel::Category::SYSTEMD_UNIT) {
            Q_EMIT systemdUnitFilterChanged();
        } else if (result && category == FilterCriteriaModel::Category::EXE) {
            Q_EMIT exeFilterChanged();
        } else if (result && category == FilterCriteriaModel::Category::TRANSPORT) {
            Q_EMIT kernelFilterChanged();
        }
        Q_EMIT dataChanged(index, index, {role});
        return result;
    } else {
        return QAbstractItemModel::setData(index, value, role);
    }
}

QVector<std::pair<QString, bool>> FilterCriteriaModel::entries(FilterCriteriaModel::Category category) const
{
    QVector<std::pair<QString, bool>> values;

    for (int i = 0; i < d->mRootItem->child(static_cast<int>(category))->childCount(); ++i) {
        values.append(
            std::make_pair<QString, bool>(d->mRootItem->child(static_cast<int>(category))->child(i)->data(FilterCriteriaModel::TEXT).toString(), false));
    }
    return values;
}
