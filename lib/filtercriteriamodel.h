/*
    SPDX-License-Identifier: LGPL-2.1-or-later OR MIT
    SPDX-FileCopyrightText: 2021 Andreas Cord-Landwehr <cordlandwehr@kde.org>
*/

#ifndef FILTERCRITERIAMODEL_H
#define FILTERCRITERIAMODEL_H

#include "journaldhelper.h"
#include "kjournald_export.h"
#include <QAbstractItemModel>
#include <QVector>
#include <memory>

class FilterCriteriaModelPrivate;

/**
 * @brief The JournaldUniqueQueryModel class provides an item model abstraction for journald queryunique API
 *
 * This class is useful when creating a list model for contents like:
 * - boot ids in specific journal database
 * - systemd units in specific journal database
 * - priorities in specific journal database
 *
 * The model can be create from an arbitrary local journald database by defining a path or from the
 * system's default journal. Values can either be set by @a setFieldString for arbitrary values or in a
 * typesafe manner via @a setField for most common fields.
 */
class KJOURNALD_EXPORT FilterCriteriaModel : public QAbstractItemModel
{
    Q_OBJECT
    Q_PROPERTY(QString journalPath WRITE setJournaldPath RESET setSystemJournal)
    /**
     * Filter for message priorities
     */
    Q_PROPERTY(int priorityFilter READ priorityFilter NOTIFY priorityFilterChanged)
    /**
     * Filter list for systemd units
     **/
    Q_PROPERTY(QStringList systemdUnitFilter READ systemdUnitFilter NOTIFY systemdUnitFilterChanged)
    /**
     * Filter list for executables (see journald '_EXE' field)
     **/
    Q_PROPERTY(QStringList exeFilter READ exeFilter NOTIFY exeFilterChanged)
    /**
     * if set to true, Kernel messages are added to the log output
     **/
    Q_PROPERTY(bool kernelFilter READ isKernelFilterEnabled NOTIFY kernelFilterChanged)

public:
    enum Category : quint8 {
        TRANSPORT = 0,
        PRIORITY = 1,
        SYSTEMD_UNIT = 2,
        EXE = 3,
    };
    Q_ENUM(Category)

    enum Roles {
        TEXT = Qt::DisplayRole,
        LONGTEXT = Qt::ToolTipRole,
        SELECTED = Qt::CheckStateRole,
        CATEGORY = Qt::UserRole + 1,
        DATA = Qt::UserRole + 2,
    };
    Q_ENUM(Roles)

    /**
     * @brief Create filter criteria model for the system journal
     *
     * This tree model provides a tree based structure for the most common filter options.
     *
     * @param parent the QObject parent
     */
    explicit FilterCriteriaModel(QObject *parent = nullptr);

    /**
     * @brief Create filter criteria model
     *
     * This tree model provides a tree based structure for the most common filter options.
     *
     * @param journalPath path to the journald database
     * @param parent the QObject parent
     */
    FilterCriteriaModel(const QString &journalPath, QObject *parent = nullptr);

    /**
     * @brief Destroys the object
     */
    ~FilterCriteriaModel() override;

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
     * @return the currently selected priority threshold for displayed log entries
     */
    int priorityFilter() const;

    /**
     * @return the list of enabled system units
     */
    QStringList systemdUnitFilter() const;

    /**
     * @return the list of enabled processes
     */
    QStringList exeFilter() const;

    /**
     * @return true if kernel log entries shall be shown, otherwise false
     */
    bool isKernelFilterEnabled() const;

    /**
     * @copydoc QAbstractItemModel::rolesNames()
     */
    QHash<int, QByteArray> roleNames() const override;

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
     * @copydoc QAbstractItemModel::setData()
     */
    Q_INVOKABLE bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;

    /**
     * @brief Convenience method for access to list of all entries' data of @p category with their selected states
     */
    QVector<std::pair<QString, bool>> entries(FilterCriteriaModel::Category category) const;

Q_SIGNALS:
    void priorityFilterChanged(int priority);
    void systemdUnitFilterChanged();
    void exeFilterChanged();
    void kernelFilterChanged();

private:
    std::unique_ptr<FilterCriteriaModelPrivate> d;
};

#endif
