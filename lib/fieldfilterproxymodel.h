/*
    SPDX-License-Identifier: LGPL-2.1-or-later OR MIT
    SPDX-FileCopyrightText: 2021 Andreas Cord-Landwehr <cordlandwehr@kde.org>
*/

#ifndef FIELDFILTERPROXYMODEL_H
#define FIELDFILTERPROXYMODEL_H

#include "journaldviewmodel.h"
#include "kjournald_export.h"
#include <qjsvalue.h>
#include <qqmlparserstatus.h>
#include <qsortfilterproxymodel.h>

class KJOURNALD_EXPORT FieldFilterProxyModel : public QSortFilterProxyModel, public QQmlParserStatus
{
    Q_OBJECT
    Q_INTERFACES(QQmlParserStatus)

    Q_PROPERTY(int count READ count NOTIFY countChanged)
    Q_PROPERTY(QObject *source READ source WRITE setSource)
    Q_PROPERTY(QString field WRITE setField)

public:
    explicit FieldFilterProxyModel(QObject *parent = 0);

    QObject *source() const;
    void setSource(QObject *source);

    void setField(const QString &field);

    QString filterString() const;
    void setFilterString(const QString &filter);

    int count() const;
    Q_INVOKABLE QJSValue get(int index) const;

    void classBegin() override;
    void componentComplete() override;

Q_SIGNALS:
    void countChanged();

protected:
    int roleKey(const QByteArray &role) const;
    QHash<int, QByteArray> roleNames() const override;
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;

private:
    bool mComplete;
    JournaldViewModel::Roles mFilterRole;
    QString mFilter;
};

#endif // FIELDFILTERPROXYMODEL_H
