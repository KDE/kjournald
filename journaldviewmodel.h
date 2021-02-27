/*
    SPDX-License-Identifier: LGPL-2.1-or-later OR MIT
    SPDX-FileCopyrightText: 2021 Andreas Cord-Landwehr <cordlandwehr@kde.org>
*/

#ifndef JOURNALDVIEWMODEL_H
#define JOURNALDVIEWMODEL_H

#include <QAbstractItemModel>
#include <memory>

class JournaldViewModelPrivate;

class JournaldViewModel : public QAbstractItemModel
{
    Q_OBJECT
    Q_PROPERTY(QString journalPath WRITE setJournaldPath READ journaldPath RESET loadSystemJournal NOTIFY journaldPathChanged)
    Q_PROPERTY(QStringList systemdUnitFilter WRITE setSystemdUnitFilter)
    Q_PROPERTY(QStringList bootFilter WRITE setBootFilter)
    Q_PROPERTY(int priorityFilter WRITE setPriorityFilter)

public:
    enum Roles { MESSAGE = Qt::UserRole + 1, DATE, PRIORITY, _SYSTEMD_UNIT, _BOOT_ID, UNIT_COLOR };

    explicit JournaldViewModel(QObject *parent = nullptr);

    /**
     * @brief Create model from a specific journal database
     *
     * This constructor works similar to "journalctl -D" and allows to use a custom path to the
     * journald database.
     *
     * @param journalPath path to the journald database
     * @param parent the QObject parent
     */
    JournaldViewModel(const QString &journalPath, QObject *parent = nullptr);

    ~JournaldViewModel();

    void setJournaldPath(const QString &path);

    QString journaldPath() const;

    void loadSystemJournal();

    void seekHead();

    QHash<int, QByteArray> roleNames() const override;

    // Header:
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    // Basic functionality:
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &index) const override;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    bool canFetchMore(const QModelIndex &parent) const override;

    void fetchMore(const QModelIndex &parent) override;

    void setSystemdUnitFilter(const QStringList &systemdUnitFilter);

    void setBootFilter(const QStringList &bootFilter);

    void setPriorityFilter(int);

Q_SIGNALS:
    void journaldPathChanged();

private:
    std::unique_ptr<JournaldViewModelPrivate> d;
};

#endif // JOURNALDVIEWMODEL_H
