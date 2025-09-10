/*
    SPDX-License-Identifier: LGPL-2.1-or-later OR MIT
    SPDX-FileCopyrightText: 2021 Andreas Cord-Landwehr <cordlandwehr@kde.org>
*/

#ifndef BOOTMODEL_H
#define BOOTMODEL_H

#include "ijournalprovider.h"
#include "kjournald_export.h"
#include <QAbstractItemModel>
#include <QQmlEngine>
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
    Q_PROPERTY(IJournalProvider * journalProvider WRITE setJournalProvider READ journalProvider NOTIFY journalProviderChanged)

    QML_ELEMENT

public:
    enum Roles {
        BOOT_ID = Qt::DisplayRole, //!< the journald ID of the boot
        SINCE = Qt::UserRole + 1, //!< the time of the earliest log entry for the boot
        UNTIL, //!< the time of the latest log entry for the boot
        DISPLAY_SHORT_UTC, //!< compact representation of the boot ID with all of its information: date, since-time, until-time, abbreviated hash
        DISPLAY_SHORT_LOCALTIME, //!< compact representation of the boot ID with all of its information: date, since-time, until-time, abbreviated hash
        CURRENT, //!< boolen role the tells if this boot is current for the system and thus expands
    };
    Q_ENUM(Roles)

    /**
     * @brief Construct model from the default local jouurnald database
     *
     * @param parent the QObject parent
     */
    explicit BootModel(QObject *parent = nullptr);

    /**
     * Destroys the boot model
     */
    ~BootModel() override;

    /**
     * Reset model by reading from a new journald database
     *
     * @param provider journal access factory
     */
    void setJournalProvider(IJournalProvider * provider);

    /**
     * @return currently set journal access factory
     */
    IJournalProvider * journalProvider() const;

    /**
     * @copydoc QAbstractItemModel::roleNames()
     */
    QHash<int, QByteArray> roleNames() const override;

    /**
     * @copydoc QAbstractItemModel::rowCount()
     */
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    /**
     * @copydoc QAbstractItemModel::data()
     */
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

Q_SIGNALS:
    void journalProviderChanged();

private:
    std::unique_ptr<BootModelPrivate> d;
};

#endif // BOOTMODEL_H
