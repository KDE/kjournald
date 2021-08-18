/*
    SPDX-License-Identifier: LGPL-2.1-or-later OR MIT
    SPDX-FileCopyrightText: 2021 Andreas Cord-Landwehr <cordlandwehr@kde.org>
*/

#ifndef BOOTMODEL_H
#define BOOTMODEL_H

#include "ijournal.h"
#include "kjournald_export.h"
#include <QAbstractItemModel>
#include <memory>

class BootModelPrivate;

/**
 * @brief Item model class that provides convienence access to boot information
 *
 * This QAbstractItemModel derived class provides a model/view abstraction for information of all
 * boots provided by a given journald database.
 */
class KJOURNALD_EXPORT BootModel : public QAbstractListModel
{
    Q_OBJECT
public:
    enum Roles {
        BOOT_ID = Qt::UserRole + 1, //!< the journald ID of the boot
        SINCE, //!< the time of the earliest log entry for the boot
        UNTIL, //!< the time of the latest log entry for the boot
        DISPLAY_SHORT_UTC, //!< compact representation of the boot ID with all of its information: date, since-time, until-time, abbreviated hash
        DISPLAY_SHORT_LOCALTIME, //!< compact representation of the boot ID with all of its information: date, since-time, until-time, abbreviated hash
    };

    /**
     * @brief Construct model from the default local jouurnald database
     *
     * @param parent the QObject parent
     */
    explicit BootModel(QObject *parent = nullptr);

    /**
     * @brief Construct model from a journal database object
     *
     * @param journal object that contains a journald database object
     * @param parent the QObject parent
     */
    BootModel(std::unique_ptr<IJournal> journal, QObject *parent = nullptr);

    /**
     * @brief Construct model from a journal database object
     *
     * This constructor works similar to "journalctl -D" and allows to use a custom path to the
     * journald database.
     *
     * @param journalPath path to journald database
     * @param parent the QObject parent
     */
    BootModel(const QString &journalPath, QObject *parent = nullptr);

    /**
     * Destroys the boot model
     */
    ~BootModel() override;

    /**
     * Reset model by reading from a new journald database
     *
     * @param path The path to directory that obtains the journald DB, usually ending with "journal".
     * @return true if path could be found and opened, otherwise false
     */
    bool setJournaldPath(const QString &path);

    /**
     * Switch to local system's default journald database
     *
     * For details regarding preference, see journald documentation.
     */
    void setSystemJournal();

    /**
     * @copydoc QAbstractItemModel::roleNames()
     */
    QHash<int, QByteArray> roleNames() const override;

    /**
     * @copydoc QAbstractItemModel::rowCount()
     */
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    /**
     * @copydoc QAbstractItemModel::columnCount()
     */
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    /**
     * @copydoc QAbstractItemModel::index()
     */
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;

    /**
     * @copydoc QAbstractItemModel::parent()
     */
    QModelIndex parent(const QModelIndex &index) const override;

    /**
     * @copydoc QAbstractItemModel::data()
     */
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    /**
     * @brief Convenience method to support Qt 5.12's ComboxBox
     */
    Q_INVOKABLE QString bootId(int row) const;

private:
    std::unique_ptr<BootModelPrivate> d;
};

#endif // BOOTMODEL_H
