/*
    SPDX-License-Identifier: LGPL-2.1-or-later OR MIT
    SPDX-FileCopyrightText: 2021 Andreas Cord-Landwehr <cordlandwehr@kde.org>
*/

#ifndef BOOTMODEL_H
#define BOOTMODEL_H

#include <QAbstractItemModel>
#include <memory>

class BootModelPrivate;

class BootModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    enum Roles {
        _BOOT_ID = Qt::UserRole + 1,
        SINCE,
        UNTIL,
        DISPLAY_SHORT
    };

    explicit BootModel(QObject *parent = nullptr);

    /**
     * @brief Create model from a specific journal database
     *
     * This constructor works similar to "journalctl -D" and allows to use a custom path to the
     * journald database.
     *
     * @param journalPath path to the journald database
     * @param parent the QObject parent
     */
    BootModel(const QString &journalPath, QObject *parent = nullptr);

    ~BootModel();

    void setJournaldPath(const QString &path);

    QHash<int,QByteArray> roleNames() const override;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;

    QModelIndex parent(const QModelIndex &index) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    /**
     * @brief Convenience method to support Qt 5.12's ComboxBox
     */
    Q_INVOKABLE QString bootId(int row) const;

private:
    std::unique_ptr<BootModelPrivate> d;
};

#endif // BOOTMODEL_H
