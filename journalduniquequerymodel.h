/*
    SPDX-License-Identifier: LGPL-2.1-or-later OR MIT
    SPDX-FileCopyrightText: Andreas Cord-Landwehr <cordlandwehr@kde.org>
*/

#ifndef JOURNALDUNIQUEQUERYMODEL_H
#define JOURNALDUNIQUEQUERYMODEL_H

#include <QAbstractItemModel>
#include <memory>

class JournaldUniqueQueryModelPrivate;

class JournaldUniqueQueryModel : public QAbstractItemModel
{
    Q_OBJECT
    Q_PROPERTY(QString journalPath WRITE setJournaldPath RESET loadSystemJournal)
    Q_PROPERTY(QString field WRITE setField)

public:
    enum Roles {
        FIELD = Qt::UserRole + 1
    };

    explicit JournaldUniqueQueryModel(QObject *parent = nullptr);

    /**
     * @brief Create model from a specific journal database
     *
     * This constructor works similar to "journalctl -D" and allows to use a custom path to the
     * journald database.
     *
     * @param journalPath path to the journald database
     * @param parent the QObject parent
     */
    JournaldUniqueQueryModel(const QString &journalPath, QObject *parent = nullptr);

    ~JournaldUniqueQueryModel();

    void setJournaldPath(const QString &path);

    void loadSystemJournal();

    void seekHead();

    void setField(const QString &fieldString);

    QHash<int,QByteArray> roleNames() const override;

    QModelIndex index(int row, int column,
                      const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &index) const override;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

private:
    std::unique_ptr<JournaldUniqueQueryModelPrivate> d;
};

#endif // JOURNALDUNIQUEQUERYMODEL_H
