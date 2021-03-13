/*
    SPDX-License-Identifier: LGPL-2.1-or-later OR MIT
    SPDX-FileCopyrightText: 2021 Andreas Cord-Landwehr <cordlandwehr@kde.org>
*/

#include "fieldfilterproxymodel.h"
#include "journaldviewmodel.h"
#include "loggingcategories.h"
#include <QDebug>
#include <QtQml>

FieldFilterProxyModel::FieldFilterProxyModel(QObject *parent)
    : QSortFilterProxyModel(parent)
    , mComplete(false)
    , mFilterRole{JournaldViewModel::Roles::SYSTEMD_UNIT}
{
    connect(this, &QSortFilterProxyModel::rowsInserted, this, &FieldFilterProxyModel::countChanged);
    connect(this, &QSortFilterProxyModel::rowsRemoved, this, &FieldFilterProxyModel::countChanged);
}

int FieldFilterProxyModel::count() const
{
    return rowCount();
}

QObject *FieldFilterProxyModel::source() const
{
    return sourceModel();
}

void FieldFilterProxyModel::setSource(QObject *source)
{
    setSourceModel(qobject_cast<QAbstractItemModel *>(source));
}

void FieldFilterProxyModel::setField(const QString &field)
{
    JournaldViewModel::Roles role = mFilterRole;
    if (field == "_SYSTEMD_UNIT") {
        role = JournaldViewModel::Roles::SYSTEMD_UNIT;
    } else if (field == "MESSAGE") {
        role = JournaldViewModel::Roles::MESSAGE;
    } else if (field == "PRIORITY") {
        role = JournaldViewModel::Roles::PRIORITY;
    } else if (field == "_BOOT_ID") {
        role = JournaldViewModel::Roles::BOOT_ID;
    } else if (field == "DATE") {
        role = JournaldViewModel::Roles::DATE;
    }
    if (role == mFilterRole) {
        // nothing to do
        return;
    }

    mFilterRole = role;
    if (mComplete) {
        QSortFilterProxyModel::setFilterRole(mFilterRole);
    }
}

QString FieldFilterProxyModel::filterString() const
{
    return mFilter;
}

void FieldFilterProxyModel::setFilterString(const QString &filter)
{
    mFilter = filter;
    setFilterFixedString(filter);
}

QJSValue FieldFilterProxyModel::get(int idx) const
{
    QJSEngine *engine = qmlEngine(this);
    QJSValue value = engine->newObject();
    if (idx >= 0 && idx < count()) {
        QHash<int, QByteArray> roles = roleNames();
        QHashIterator<int, QByteArray> it(roles);
        while (it.hasNext()) {
            it.next();
            value.setProperty(QString::fromUtf8(it.value()), data(index(idx, 0), it.key()).toString());
        }
    }
    return value;
}

void FieldFilterProxyModel::classBegin()
{
}

void FieldFilterProxyModel::componentComplete()
{
    mComplete = true;
    QSortFilterProxyModel::setFilterRole(mFilterRole);
}

int FieldFilterProxyModel::roleKey(const QByteArray &role) const
{
    QHash<int, QByteArray> roles = roleNames();
    QHashIterator<int, QByteArray> it(roles);
    while (it.hasNext()) {
        it.next();
        if (it.value() == role)
            return it.key();
    }
    return -1;
}

QHash<int, QByteArray> FieldFilterProxyModel::roleNames() const
{
    if (QAbstractItemModel *source = sourceModel())
        return source->roleNames();
    return QHash<int, QByteArray>();
}

bool FieldFilterProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    QAbstractItemModel *model = sourceModel();
    QModelIndex sourceIndex = model->index(sourceRow, 0, sourceParent);
    if (!sourceIndex.isValid()) {
        return true;
    }
    QString key = model->data(sourceIndex, filterRole()).toString();
    if (key == mFilter || mFilter.isEmpty()) {
        return true;
    } else {
        return false;
    }
}
