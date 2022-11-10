/*
    SPDX-License-Identifier: LGPL-2.1-or-later OR MIT
    SPDX-FileCopyrightText: 2021 Andreas Cord-Landwehr <cordlandwehr@kde.org>
*/

#include "flattenedfiltercriteriaproxymodel.h"
#include "colorizer.h"
#include "filtercriteriamodel.h"
#include "loggingcategories.h"
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
    if (mSourceModel) {
        disconnect(mSourceModel, &QAbstractItemModel::dataChanged, this, &FlattenedFilterCriteriaProxyModel::handleSourceModelDataChanged);
        disconnect(mSourceModel, &QAbstractItemModel::modelAboutToBeReset, this, &FlattenedFilterCriteriaProxyModel::handleSourceModelOnModelAboutToBeReset);
        disconnect(mSourceModel, &QAbstractItemModel::modelReset, this, &FlattenedFilterCriteriaProxyModel::handleSourceModelOnModelReset);
    }

    // TODO add assert that this model only handles two level hierarchies
    handleSourceModelOnModelAboutToBeReset();
    mSourceModel = model;
    connect(mSourceModel, &QAbstractItemModel::dataChanged, this, &FlattenedFilterCriteriaProxyModel::handleSourceModelDataChanged);
    connect(mSourceModel, &QAbstractItemModel::modelAboutToBeReset, this, &FlattenedFilterCriteriaProxyModel::handleSourceModelOnModelAboutToBeReset);
    connect(mSourceModel, &QAbstractItemModel::modelReset, this, &FlattenedFilterCriteriaProxyModel::handleSourceModelOnModelReset);

    handleSourceModelOnModelReset();
}

void FlattenedFilterCriteriaProxyModel::handleSourceModelOnModelAboutToBeReset()
{
    beginResetModel();
    mMapToSourceIndex.clear();
}

void FlattenedFilterCriteriaProxyModel::handleSourceModelOnModelReset()
{
    // generate top level items
    for (int i = 0; i < mSourceModel->rowCount(); ++i) {
        mMapToSourceIndex.append({mSourceModel->index(i, 0), false});
    }
    endResetModel();
}

QAbstractItemModel *FlattenedFilterCriteriaProxyModel::sourceModel() const
{
    return mSourceModel;
}

void FlattenedFilterCriteriaProxyModel::handleSourceModelDataChanged(const QModelIndex &sourceTopLeft,
                                                                     const QModelIndex &sourceBottomRight,
                                                                     const QVector<int> &roles)
{
    Q_ASSERT(sourceTopLeft.row() <= sourceBottomRight.row());
    if (sourceTopLeft.row() > sourceBottomRight.row()) {
        qCWarning(KJOURNALD_DEBUG) << "Data change ignored, index values not in order";
        return;
    }
    for (int i = 0; i < mMapToSourceIndex.size(); ++i) {
        if (mMapToSourceIndex.at(i).mSourceIndex.row() >= sourceTopLeft.row() && mMapToSourceIndex.at(i).mSourceIndex.row() < sourceBottomRight.row()) {
            Q_EMIT dataChanged(index(i, 0), index(i, 0));
            return;
        }
    }
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
        if (mMapToSourceIndex.at(index.row()).mIndentation == 0) {
            return FlattenedFilterCriteriaProxyModel::DelgateType::FIRST_LEVEL;
        }
        switch (mSourceModel->data(mMapToSourceIndex.at(index.row()).mSourceIndex, FilterCriteriaModel::Roles::CATEGORY).toInt()) {
        case FilterCriteriaModel::Category::PRIORITY:
            return FlattenedFilterCriteriaProxyModel::DelgateType::RADIOBUTTON;
        case FilterCriteriaModel::Category::SYSTEMD_UNIT:
            return FlattenedFilterCriteriaProxyModel::DelgateType::CHECKBOX;
        case FilterCriteriaModel::Category::EXE:
            return FlattenedFilterCriteriaProxyModel::DelgateType::CHECKBOX;
        case FilterCriteriaModel::Category::TRANSPORT:
            return FlattenedFilterCriteriaProxyModel::DelgateType::CHECKBOX;
        }
        return QVariant::fromValue(FlattenedFilterCriteriaProxyModel::DelgateType::SECTION);
    }
    case FlattenedFilterCriteriaProxyModel::COLOR: {
        if (mSourceModel->data(mMapToSourceIndex.at(index.row()).mSourceIndex, FilterCriteriaModel::Roles::CATEGORY)
            == FilterCriteriaModel::Category::TRANSPORT) {
            return QColor(Qt::black);
        }
        return Colorizer::color(mSourceModel->data(mMapToSourceIndex.at(index.row()).mSourceIndex, FilterCriteriaModel::Roles::DATA).toString(),
                                Colorizer::COLOR_TYPE::FOREGROUND);
    }
    }

    return QVariant();
}

bool FlattenedFilterCriteriaProxyModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (index.row() >= mMapToSourceIndex.count()) {
        qCWarning(KJOURNALD_DEBUG) << "access setData for line out of range with index:" << index.row() << " / total rows" << mMapToSourceIndex.count();
        return false;
    }
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
        const QModelIndex sourceIndex = mMapToSourceIndex.at(index.row()).mSourceIndex;
        const int childrenCount = mSourceModel->rowCount(sourceIndex);
        const auto category = mSourceModel->data(sourceIndex, FilterCriteriaModel::Roles::CATEGORY).value<FilterCriteriaModel::Category>();
        // detect first level elements and clear selelection if they are unselected
        if (childrenCount > 0 && value == false && (category == FilterCriteriaModel::SYSTEMD_UNIT || category == FilterCriteriaModel::EXE)) {
            for (int i = 0; i < childrenCount; ++i) {
                mSourceModel->setData(mSourceModel->index(i, 0, sourceIndex), false, FilterCriteriaModel::Roles::SELECTED);
            }
        }
        mSourceModel->setData(sourceIndex, value, FilterCriteriaModel::Roles::SELECTED);
        Q_EMIT dataChanged(index, index);
    }

    return false;
}
