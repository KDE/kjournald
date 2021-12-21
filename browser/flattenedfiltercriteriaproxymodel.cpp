/*
    SPDX-License-Identifier: LGPL-2.1-or-later OR MIT
    SPDX-FileCopyrightText: 2021 Andreas Cord-Landwehr <cordlandwehr@kde.org>
*/

#include "colorizer.h"
#include "flattenedfiltercriteriaproxymodel.h"
#include "filtercriteriamodel.h"
#include <QDebug>

QHash<int, QByteArray> FlattenedFilterCriteriaProxyModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[FlattenedFilterCriteriaProxyModel::TEXT] = "text";
    roles[FlattenedFilterCriteriaProxyModel::LONGTEXT] = "longtext";
    roles[FlattenedFilterCriteriaProxyModel::SELECTED] = "selected";
    roles[FlattenedFilterCriteriaProxyModel::INDENTATION] = "indentation";
    roles[FlattenedFilterCriteriaProxyModel::EXPANDED] = "expanded";
    roles[FlattenedFilterCriteriaProxyModel::TYPE] = "type";
    roles[FlattenedFilterCriteriaProxyModel::COLOR] = "color";
    return roles;
}

void FlattenedFilterCriteriaProxyModel::setSourceModel(QAbstractItemModel *model)
{
    // TODO add assert that this model only handles two level hierarchies
    beginResetModel();
    mMapToSourceIndex.clear();
    mSourceModel = model;

    // top level items
    for (int i = 0; i < mSourceModel->rowCount(); ++i) {
        mMapToSourceIndex.append({mSourceModel->index(i, 0), false});
    }
    endResetModel();
}

QAbstractItemModel *FlattenedFilterCriteriaProxyModel::sourceModel() const
{
    return mSourceModel;
}

int FlattenedFilterCriteriaProxyModel::rowCount(const QModelIndex &parent) const
{
    return mMapToSourceIndex.size();
}

int FlattenedFilterCriteriaProxyModel::columnCount(const QModelIndex &parent) const
{
    return 1;
}

QModelIndex FlattenedFilterCriteriaProxyModel::index(int row, int column, const QModelIndex &parent) const
{
    return createIndex(row, column);
}

QModelIndex FlattenedFilterCriteriaProxyModel::parent(const QModelIndex &index) const
{
    // no tree model, thus no parent
    return QModelIndex();
}

QVariant FlattenedFilterCriteriaProxyModel::data(const QModelIndex &index, int role) const
{
    if (index.row() < 0 || index.row() >= rowCount()) {
        return QVariant();
    }
    switch (role) {
    case FlattenedFilterCriteriaProxyModel::Roles::TEXT:
        return mSourceModel->data(mMapToSourceIndex.at(index.row()).mSourceIndex, FilterCriteriaModel::Roles::TEXT);
    case FlattenedFilterCriteriaProxyModel::Roles::LONGTEXT:
        return mSourceModel->data(mMapToSourceIndex.at(index.row()).mSourceIndex, FilterCriteriaModel::Roles::LONGTEXT);
    case FlattenedFilterCriteriaProxyModel::Roles::EXPANDED:
        return mMapToSourceIndex.at(index.row()).mIsExpanded;
    case FlattenedFilterCriteriaProxyModel::Roles::SELECTED:
        return mSourceModel->data(mMapToSourceIndex.at(index.row()).mSourceIndex, FilterCriteriaModel::Roles::SELECTED);
    case FlattenedFilterCriteriaProxyModel::Roles::INDENTATION:
        return mMapToSourceIndex.at(index.row()).mIndentation;
    case FlattenedFilterCriteriaProxyModel::Roles::TYPE: {
        switch (mSourceModel->data(mMapToSourceIndex.at(index.row()).mSourceIndex, FilterCriteriaModel::Roles::CATEGORY).toInt()) {
        case FilterCriteriaModel::Category::PRIORITY:
            return FlattenedFilterCriteriaProxyModel::DelgateType::RADIOBUTTON;
        case FilterCriteriaModel::Category::SYSTEMD_UNIT:
            return FlattenedFilterCriteriaProxyModel::DelgateType::CHECKBOX_COLORED;
        case FilterCriteriaModel::Category::EXE:
            return FlattenedFilterCriteriaProxyModel::DelgateType::CHECKBOX;
        case FilterCriteriaModel::Category::TRANSPORT:
            return FlattenedFilterCriteriaProxyModel::DelgateType::CHECKBOX;
        }
        return QVariant::fromValue(FlattenedFilterCriteriaProxyModel::DelgateType::SECTION);
    }
    case FlattenedFilterCriteriaProxyModel::COLOR:
        return Colorizer::color(mSourceModel->data(mMapToSourceIndex.at(index.row()).mSourceIndex, FilterCriteriaModel::Roles::DATA).toString(),
                                Colorizer::COLOR_TYPE::FOREGROUND);
    }

    return QVariant();
}

bool FlattenedFilterCriteriaProxyModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (role == FlattenedFilterCriteriaProxyModel::Roles::EXPANDED) {
        const int childrenCount = mSourceModel->rowCount(mMapToSourceIndex.at(index.row()).mSourceIndex);
        const QModelIndex parent = mMapToSourceIndex.at(index.row()).mSourceIndex;
        if (childrenCount == 0) {
            return false;
        }
        if (mMapToSourceIndex.at(index.row()).mIsExpanded == false) {
            mMapToSourceIndex[index.row()].mIsExpanded = true;
            Q_EMIT dataChanged(index, index);
            beginInsertRows(QModelIndex(), index.row() + 1, index.row() + childrenCount);
            for (int i = 0; i < childrenCount; ++i) {
                mMapToSourceIndex.insert(index.row() + 1 + i, {mSourceModel->index(i, 0, parent), false, 1});
            }
            endInsertRows();
        } else {
            mMapToSourceIndex[index.row()].mIsExpanded = false;
            Q_EMIT dataChanged(index, index);
            beginRemoveRows(QModelIndex(), index.row() + 1, index.row() + childrenCount);
            mMapToSourceIndex.remove(index.row() + 1, childrenCount);
            endRemoveRows();
        }
        return true;
    }

    if (role == FlattenedFilterCriteriaProxyModel::Roles::SELECTED) {
        mSourceModel->setData(mMapToSourceIndex.at(index.row()).mSourceIndex, value, FilterCriteriaModel::Roles::SELECTED);
        Q_EMIT dataChanged(index, index);
    }

    return false;
}
