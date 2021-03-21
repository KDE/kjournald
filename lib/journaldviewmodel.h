/*
    SPDX-License-Identifier: LGPL-2.1-or-later OR MIT
    SPDX-FileCopyrightText: 2021 Andreas Cord-Landwehr <cordlandwehr@kde.org>
*/

#ifndef JOURNALDVIEWMODEL_H
#define JOURNALDVIEWMODEL_H

#include <QAbstractItemModel>
#include <memory>
#include "kjournald_export.h"

class JournaldViewModelPrivate;

/**
 * @brief Item model class that provides convienence access to journald database
 *
 * This QAbstractItemModel derived class provides a model/view abstraction for the journald API
 * with the goal to ease integration in Qt based applications.
 */
class KJOURNALD_EXPORT JournaldViewModel : public QAbstractItemModel
{
    Q_OBJECT
    Q_PROPERTY(QString journalPath WRITE setJournaldPath READ journaldPath RESET setSystemJournal NOTIFY journaldPathChanged)
    /**
     * Configure model to only provide messages for stated systemd units
     */
    Q_PROPERTY(QStringList systemdUnitFilter WRITE setSystemdUnitFilter)
    /**
     * Configure model to only provide messages for stated boot ids.
     */
    Q_PROPERTY(QStringList bootFilter WRITE setBootFilter)
    /** if set to true, Kernel messages are added to the log output **/
    Q_PROPERTY(bool kernelFilter WRITE setKernelFilter READ isKernelFilterEnabled NOTIFY kernelFilterChanged)
    /**
     * Configure model to only provide messages with stated priority or higher. Default: no filter is set.
     **/
    Q_PROPERTY(int priorityFilter WRITE setPriorityFilter RESET resetPriorityFilter)

public:
    enum Roles {
        MESSAGE = Qt::UserRole + 1, //!< journal entry's message text
        DATE, //!< date of journal entry
        PRIORITY, //!< priority of journal entry
        SYSTEMD_UNIT, //!< systemd unit name of journal entry
        BOOT_ID, //!< boot ID of journal entry
        UNIT_COLOR //!< convenience rainbow color that is hashed for systemd unit
    };
    Q_ENUM(Roles);

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

    /**
     * destroys JournaldViewModel
     */
    ~JournaldViewModel() override;

    /**
     * Reset model by reading from a new journald database
     *
     * @param path The path to directory that obtains the journald DB, usually ending with "journal".
     * @return true if path could be found and opened, otherwise false
     */
    bool setJournaldPath(const QString &path);

    /**
     * @return currently set journal path or empty string if none is set
     */
    QString journaldPath() const;

    /**
     * Switch to local system's default journald database
     *
     * For details regarding preference, see journald documentation.
     */
    void setSystemJournal();

    /**
     * @copydoc QAbstractItemModel::rolesNames()
     */
    QHash<int, QByteArray> roleNames() const override;

    /**
     * @copydoc QAbstractItemModel::headerData()
     *
     * @note NOT IMPLEMENTED YET
     */
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    /**
     * @copydoc QAbstractItemModel::index()
     */
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;

    /**
     * @copydoc QAbstractItemModel::parent()
     */
    QModelIndex parent(const QModelIndex &index) const override;

    /**
     * @copydoc QAbstractItemModel::rowCount()
     */
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    /**
     * @copydoc QAbstractItemModel::columnCount()
     */
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    /**
     * @copydoc QAbstractItemModel::data()
     */
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    /**
     * @copydoc QAbstractItemModel::canFetchMore()
     */
    bool canFetchMore(const QModelIndex &parent) const override;

    /**
     * @copydoc QAbstractItemModel::fetchMore()
     */
    void fetchMore(const QModelIndex &parent) override;

    /**
     * @brief Configure for which systemd units messages shall be shown
     *
     * If no unit is configured, this filter is deactivated. The given values are compared
     * to the _SYSTEMD_UNIT journal value.
     *
     * @param systemdUnitFilter list of system units
     */
    void setSystemdUnitFilter(const QStringList &systemdUnitFilter);

    /**
     * @brief Configure for which boots messages shall be shown
     *
     * If no boot id is configured, this filter is deactivated. The given values are compared
     * to the _BOOT_ID journal value.
     *
     * @param bootFilter list of boot ids
     */
    void setBootFilter(const QStringList &bootFilter);

    /**
     * @brief Configure if Kernel messages shall be included in model
     *
     * Per default, Kernel messages are deactivated.
     *
     * @param showKernelMessages parameter that defines if Kernel messages shall be shown
     */
    void setKernelFilter(bool showKernelMessages);

    /**
     * @return true if Kernel messages are included in model, otherwise false
     */
    bool isKernelFilterEnabled() const;

    /**
     * @brief Filter messages such that only messages with this and higher priority are provided
     *
     * @note Non-systemd services may not follow systemd's priority values
     *
     * @param priority the minimal priority for messages that shall be provided by model
     */
    void setPriorityFilter(int priority);

    /**
     * @brief Discard priority filter and display all messages
     */
    void resetPriorityFilter();

    /**
     * @return row index of searched string
     */
    Q_INVOKABLE int search(const QString &searchString, int startRow);

    /**
     * @brief Format time into string
     * @param datetime the datetime object
     * @param utc if set to true, the string will be UTC time, otherwise according to the current local
     * @return formatted string
     */
    Q_INVOKABLE QString formatTime(const QDateTime &datetime, bool utc) const;

Q_SIGNALS:
    /**
     * Signal is emitted when journald DB path is changed
     */
    void journaldPathChanged();
    /**
     * Signal is emitted when Kernel message filter is changed
     */
    void kernelFilterChanged();

private:
    std::unique_ptr<JournaldViewModelPrivate> d;
};

#endif // JOURNALDVIEWMODEL_H
