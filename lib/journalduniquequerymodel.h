/*
    SPDX-License-Identifier: LGPL-2.1-or-later OR MIT
    SPDX-FileCopyrightText: 2021 Andreas Cord-Landwehr <cordlandwehr@kde.org>
*/

#ifndef JOURNALDUNIQUEQUERYMODEL_H
#define JOURNALDUNIQUEQUERYMODEL_H

#include <QAbstractItemModel>
#include <memory>
#include "kjournald_export.h"

class JournaldUniqueQueryModelPrivate;

class KJOURNALD_EXPORT JournaldUniqueQueryModel : public QAbstractItemModel
{
    Q_OBJECT
    Q_PROPERTY(QString journalPath WRITE setJournaldPath RESET loadSystemJournal)
    Q_PROPERTY(QString field WRITE setField)
    Q_PROPERTY(QStringList selectedEntries READ selectedEntries NOTIFY selectedEntriesChanged)

public:
    enum Roles {
        FIELD = Qt::UserRole + 1,
        SELECTED //!< supports UI integration by storing checked
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

    /**
     * Reset model by reading from a new journal DB
     *
     * @param path The path to directory that obtains the journald DB, usually ending with "journal".
     * @return true if path could be found and opened, otherwise false
     */
    bool setJournaldPath(const QString &path);

    void loadSystemJournal();

    void seekHead();

    void setField(const QString &fieldString);

    QHash<int, QByteArray> roleNames() const override;

    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &index) const override;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    Q_INVOKABLE bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;

    QStringList selectedEntries() const;

    Q_INVOKABLE void setAllSelectionStates(bool selected);

Q_SIGNALS:
    void selectedEntriesChanged();

private:
    std::unique_ptr<JournaldUniqueQueryModelPrivate> d;
};

#endif // JOURNALDUNIQUEQUERYMODEL_H
