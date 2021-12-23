/*
    SPDX-License-Identifier: LGPL-2.1-or-later OR MIT
    SPDX-FileCopyrightText: 2021 Andreas Cord-Landwehr <cordlandwehr@kde.org>
*/

#ifndef FILTERCRITERIAMODEL_P_H
#define FILTERCRITERIAMODEL_P_H

#include "filtercriteriamodel.h"
#include "ijournal.h"
#include <QMap>
#include <QString>
#include <QVector>
#include <memory>

class SelectionEntry
{
public:
    explicit SelectionEntry() = default;
    explicit SelectionEntry(const QString &text,
                            const QVariant &data,
                            FilterCriteriaModel::Category category,
                            bool selected = false,
                            std::shared_ptr<SelectionEntry> parentItem = nullptr);

    void appendChild(std::shared_ptr<SelectionEntry> child);

    std::shared_ptr<SelectionEntry> child(int row);
    int childCount() const;
    int columnCount() const;
    QVariant data(FilterCriteriaModel::Roles role) const;
    bool setData(const QVariant &value, FilterCriteriaModel::Roles role);
    int row() const;
    std::shared_ptr<SelectionEntry> parentItem();

private:
    std::vector<std::shared_ptr<SelectionEntry>> mChildItems;
    std::weak_ptr<SelectionEntry> mParentItem;
    QString mText; //!< user formatted string
    QVariant mData; //!< verbatim string as needed for journald filtering
    bool mSelected{true};
    FilterCriteriaModel::Category mCategory;
};

class FilterCriteriaModelPrivate
{
public:
    FilterCriteriaModelPrivate();
    ~FilterCriteriaModelPrivate();
    /**
     * @brief clear all model data and read units, processes... from currently set journal
     */
    void rebuildModel();

    std::shared_ptr<IJournal> mJournal;
    std::shared_ptr<SelectionEntry> mRootItem;
};

#endif // FILTERCRITERIAMODEL_P_H
