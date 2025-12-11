/*
    SPDX-License-Identifier: LGPL-2.1-or-later OR MIT
    SPDX-FileCopyrightText: 2021 Andreas Cord-Landwehr <cordlandwehr@kde.org>
*/

#include "filtercriteriamodel.h"
#include "filtercriteriamodel_p.h"
#include "journaldhelper.h"
#include "kjournaldlib_log_general.h"
#include <KLocalizedString>
#include <QDebug>
#include <QDir>
#include <QString>
#include <memory>

constexpr quint8 sDefaultPriorityLevel{5};

QString FilterCriteriaModelPrivate::mapPriorityToString(int priority)
{
    switch (priority) {
    case 0:
        return i18nc("Radio box option, log priority value", "Emergency");
    case 1:
        return i18nc("Radio box option, log priority value", "Alert");
    case 2:
        return i18nc("Radio box option, log priority value", "Critical");
    case 3:
        return i18nc("Radio box option, log priority value", "Error");
    case 4:
        return i18nc("Radio box option, log priority value", "Warning");
    case 5:
        return i18nc("Radio box option, log priority value", "Notice");
    case 6:
        return i18nc("Radio box option, log priority value", "Info");
    case 7:
        return i18nc("Radio box option, log priority value", "Debug");
    case -1:
        return i18nc("Radio box option, log priority value", "No Filter");
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
    qCWarning(KJOURNALDLIB_GENERAL) << "no settable role";
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
        auto parent = std::make_shared<SelectionEntry>(i18nc("Section title for log message source", "Transport"),
                                                       QVariant(),
                                                       FilterCriteriaModel::Category::TRANSPORT,
                                                       false,
                                                       mRootItem);
        mRootItem->appendChild(parent);
        mRootItem->child(FilterCriteriaModel::Category::TRANSPORT)
            ->appendChild(std::move(std::make_unique<SelectionEntry>(i18nc("Checkbox option for kernel log messages", "Kernel"),
                                                                     QLatin1String("kernel"),
                                                                     FilterCriteriaModel::Category::TRANSPORT,
                                                                     false,
                                                                     parent)));
    }
    {
        auto parent = std::make_shared<SelectionEntry>(i18nc("Section title for log message priority", "Priority"),
                                                       QVariant(),
                                                       FilterCriteriaModel::Category::PRIORITY,
                                                       false,
                                                       mRootItem);
        mRootItem->appendChild(parent);
        for (int i = 0; i <= 7; ++i) {
            mRootItem->child(FilterCriteriaModel::Category::PRIORITY)
                ->appendChild(std::move(std::make_unique<SelectionEntry>(mapPriorityToString(i),
                                                                         QString::number(i),
                                                                         FilterCriteriaModel::Category::PRIORITY,
                                                                         i == sDefaultPriorityLevel ? true : false,
                                                                         parent)));
        }
        // add "no filter" option at end
        mRootItem->child(FilterCriteriaModel::Category::PRIORITY)
            ->appendChild(std::move(
                std::make_unique<SelectionEntry>(mapPriorityToString(-1), QString::number(-1), FilterCriteriaModel::Category::PRIORITY, false, parent)));
        mPriorityLevel = sDefaultPriorityLevel;
    }
    {
        auto parent = std::make_shared<SelectionEntry>(i18nc("Section title for systemd unit", "Unit"),
                                                       QVariant(),
                                                       FilterCriteriaModel::Category::SYSTEMD_UNIT,
                                                       false,
                                                       mRootItem);
        mRootItem->appendChild(parent);
        if (mJournal) {
            auto criteria = mJournalProvider->isUser() ? JournaldHelper::Field::_SYSTEMD_USER_UNIT : JournaldHelper::Field::_SYSTEMD_UNIT;
            QVector<QString> units = JournaldHelper::queryUnique(mJournal->get(), criteria);
            units.erase(std::remove_if(std::begin(units),
                                       std::end(units),
                                       [](const QString &unit) {
                                           return unit.startsWith(QLatin1String("systemd-coredump@"))
                                               || unit.startsWith(QLatin1String("drkonqi-coredump-processor@"))
                                               || unit.startsWith(QLatin1String("drkonqi-coredump-launcher@"));
                                       }),
                        std::cend(units));
            std::sort(std::begin(units), std::end(units), [](const QString &a, const QString &b) {
                return QString::compare(a, b, Qt::CaseInsensitive) <= 0;
            });
            for (const auto &unit : std::as_const(units)) {
                // skip any non-service units, because we expect users only be interested in filtering those
                if (!unit.endsWith(QLatin1String(".service"))) {
                    continue;
                }
                mRootItem->child(FilterCriteriaModel::Category::SYSTEMD_UNIT)
                    ->appendChild(std::move(std::make_unique<SelectionEntry>(JournaldHelper::cleanupString(unit),
                                                                             unit,
                                                                             FilterCriteriaModel::Category::SYSTEMD_UNIT,
                                                                             false,
                                                                             parent)));
            }
        }
    }
    {
        auto parent = std::make_shared<SelectionEntry>(i18nc("Section title for process list", "Process"),
                                                       QVariant(),
                                                       FilterCriteriaModel::Category::EXE,
                                                       false,
                                                       mRootItem);
        mRootItem->appendChild(parent);
        if (mJournal) {
            QVector<QString> exes = JournaldHelper::queryUnique(mJournal->get(), JournaldHelper::Field::_EXE);
            std::sort(std::begin(exes), std::end(exes), [](const QString &a, const QString &b) {
                return QString::compare(a, b, Qt::CaseInsensitive) <= 0;
            });
            for (const auto &exe : std::as_const(exes)) {
                mRootItem->child(FilterCriteriaModel::Category::EXE)
                    ->appendChild(std::move(
                        std::make_unique<SelectionEntry>(JournaldHelper::cleanupString(exe), exe, FilterCriteriaModel::Category::EXE, false, parent)));
            }
        }
    }
}

FilterCriteriaModel::FilterCriteriaModel(QObject *parent)
    : QAbstractItemModel(parent)
    , d(new FilterCriteriaModelPrivate)
{
}

void FilterCriteriaModel::setJournalProvider(IJournalProvider *provider)
{
    d->mJournalProvider = provider;
    if (provider) {
        d->mJournal = provider->openJournal();
    } else {
        d->mJournal.reset();
    }
    Q_EMIT journalProviderChanged();
    beginResetModel();
    d->rebuildModel();
    endResetModel();
}

IJournalProvider *FilterCriteriaModel::journalProvider() const
{
    return d->mJournalProvider;
}

FilterCriteriaModel::~FilterCriteriaModel() = default;

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

int FilterCriteriaModel::priorityFilter() const
{
    return static_cast<qint8>(d->mPriorityLevel.value_or(-1));
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
            qCCritical(KJOURNALDLIB_GENERAL) << "Index out of range" << index;
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
    if (nullptr == entry) {
        return QAbstractItemModel::setData(index, value, role);
    }
    if (value == entry->data(static_cast<FilterCriteriaModel::Roles>(role))) {
        return false; // nothing to do
    }

    // clear operations for top-level categories
    const bool result = entry->setData(value, static_cast<FilterCriteriaModel::Roles>(role));
    const auto category = entry->data(FilterCriteriaModel::Roles::CATEGORY).value<FilterCriteriaModel::Category>();
    Q_EMIT dataChanged(index, index, {role});

    if (result && category == FilterCriteriaModel::Category::PRIORITY && static_cast<FilterCriteriaModel::Roles>(role) == SELECTED) {
        // only listen on changes that set entry data to true, because this is considered a selector in the list
        std::shared_ptr<SelectionEntry> parent = d->mRootItem->child(FilterCriteriaModel::Category::PRIORITY);
        for (int i = 0; i < parent->childCount(); ++i) {
            const bool selectedValue = (i == index.row());
            parent->child(i)->setData(selectedValue, FilterCriteriaModel::SELECTED);
            static_cast<SelectionEntry *>(FilterCriteriaModel::index(i, 0, index.parent()).internalPointer())
                ->setData(selectedValue, static_cast<FilterCriteriaModel::Roles>(role));
        }
        Q_EMIT dataChanged(FilterCriteriaModel::index(0, 0, index.parent()), FilterCriteriaModel::index(parent->childCount() - 1, 0, index.parent()), {role});
        Q_ASSERT(index.row() >= 0);
        if (parent->child(index.row())->data(FilterCriteriaModel::DATA).toInt() >= 0) {
            d->mPriorityLevel = parent->child(index.row())->data(FilterCriteriaModel::DATA).toInt();
        } else {
            d->mPriorityLevel = std::nullopt;
        }
        qCDebug(KJOURNALDLIB_GENERAL) << "set priority level to:" << static_cast<qint8>(d->mPriorityLevel.value_or(-1));
        Q_EMIT priorityFilterChanged(index.row());
    } else if (result && category == FilterCriteriaModel::Category::SYSTEMD_UNIT) {
        // for checkable entries update parent's selected state
        if (value.toBool() == true) {
            setData(index.parent(), true, FilterCriteriaModel::Roles::SELECTED);
        } else {
            const auto parent = static_cast<SelectionEntry *>(index.parent().internalPointer());
            if (parent) {
                bool hasSelectedSibling{false};
                for (int i = 0; i < parent->childCount(); ++i) {
                    hasSelectedSibling = hasSelectedSibling || parent->child(i)->data(SELECTED).toBool();
                }
                setData(index.parent(), hasSelectedSibling, FilterCriteriaModel::Roles::SELECTED);
            }
        }
        Q_EMIT systemdUnitFilterChanged();
    } else if (result && category == FilterCriteriaModel::Category::EXE) {
        // for checkable entries update parent's selected state
        if (value.toBool() == true) {
            setData(index.parent(), true, FilterCriteriaModel::Roles::SELECTED);
        } else {
            const auto parent = static_cast<SelectionEntry *>(index.parent().internalPointer());
            if (parent) {
                bool hasSelectedSibling{false};
                for (int i = 0; i < parent->childCount(); ++i) {
                    hasSelectedSibling = hasSelectedSibling || parent->child(i)->data(SELECTED).toBool();
                }
                setData(index.parent(), hasSelectedSibling, FilterCriteriaModel::Roles::SELECTED);
            }
        }
        Q_EMIT exeFilterChanged();
    } else if (result && category == FilterCriteriaModel::Category::TRANSPORT) {
        Q_EMIT kernelFilterChanged();
    }
    return result;
}

QVector<std::pair<QString, bool>> FilterCriteriaModel::entries(FilterCriteriaModel::Category category) const
{
    QVector<std::pair<QString, bool>> values;

    for (int i = 0; i < d->mRootItem->child(static_cast<int>(category))->childCount(); ++i) {
        values.append(
            std::make_pair<QString, bool>(d->mRootItem->child(static_cast<int>(category))->child(i)->data(FilterCriteriaModel::DATA).toString(), false));
    }
    return values;
}

#include "moc_filtercriteriamodel.cpp"
