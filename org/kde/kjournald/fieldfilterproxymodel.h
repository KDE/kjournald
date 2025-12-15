
/*
    SPDX-License-Identifier: LGPL-2.1-or-later OR MIT
    SPDX-FileCopyrightText: 2021 Andreas Cord-Landwehr <cordlandwehr@kde.org>
*/

#ifndef FIELDFILTERPROXYMODEL_H
#define FIELDFILTERPROXYMODEL_H

#include "journaldviewmodel.h"
#include "kjournald_export.h"
#include <QJSValue>
#include <QQmlEngine>
#include <QQmlParserStatus>
#include <QSortFilterProxyModel>

class KJOURNALD_EXPORT FieldFilterProxyModel : public QSortFilterProxyModel, public QQmlParserStatus
{
    Q_OBJECT
    Q_INTERFACES(QQmlParserStatus)

    Q_PROPERTY(int count READ rowCount NOTIFY countChanged)
    Q_PROPERTY(QString field WRITE setField)

    QML_ELEMENT

public:
    explicit FieldFilterProxyModel(QObject *parent = nullptr);

    void setField(const QString &field);

    QString filterString() const;
    void setFilterString(const QString &filter);

    Q_INVOKABLE QJSValue get(int index) const;

    void classBegin() override;
    void componentComplete() override;

Q_SIGNALS:
    void countChanged();

protected:
    int roleKey(const QByteArray &role) const;
    QHash<int, QByteArray> roleNames() const override;

private:
    bool mComplete;
    JournaldViewModel::Roles mFilterRole;
    QString mFilter;
};

#endif // FIELDFILTERPROXYMODEL_H
