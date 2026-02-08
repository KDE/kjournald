/*
    SPDX-License-Identifier: LGPL-2.1-or-later OR MIT
    SPDX-FileCopyrightText: 2021 Andreas Cord-Landwehr <cordlandwehr@kde.org>
*/

#ifndef FILTERCRITERIAMODEL_H
#define FILTERCRITERIAMODEL_H

#include "ijournalprovider.h"
#include "kjournald_export.h"
#include <QAbstractItemModel>
#include <QQmlEngine>
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
    Q_PROPERTY(IJournalProvider *journalProvider READ journalProvider WRITE setJournalProvider NOTIFY journalProviderChanged FINAL)
    /**
     * Filter for message priorities
     */
    Q_PROPERTY(int priorityFilter READ priorityFilter NOTIFY priorityFilterChanged FINAL)
    /**
     * Filter list for systemd user units
     **/
    Q_PROPERTY(QStringList systemdUserUnitFilter READ systemdUserUnitFilter NOTIFY systemdUserUnitFilterChanged FINAL)
    /**
     * Filter list for systemd system units
     **/
    Q_PROPERTY(QStringList systemdSystemUnitFilter READ systemdSystemUnitFilter NOTIFY systemdSystemUnitFilterChanged FINAL)
    /**
     * Filter list for executables (see journald '_EXE' field)
     **/
    Q_PROPERTY(QStringList exeFilter READ exeFilter NOTIFY exeFilterChanged FINAL)
    /**
     * if set to true, Kernel messages are added to the log output
     **/
    Q_PROPERTY(bool kernelFilter READ isKernelFilterEnabled NOTIFY kernelFilterChanged FINAL)
    /**
     * if set to true, templated systemd services are grouped together
     **/
    Q_PROPERTY(bool enableSystemdUnitTemplateGrouping READ groupTemplatedSystemdUnits WRITE setGroupTemplatedSystemdUnits NOTIFY
                   groupTemplatedSystemdUnitsChanged FINAL)

    QML_ELEMENT

public:
    enum Category : quint8 {
        TRANSPORT = 0,
        PRIORITY = 1,
        SYSTEMD_USER_UNIT = 2,
        SYSTEMD_SYSTEM_UNIT = 3,
        EXE = 4,
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
     * @brief Create filter criteria model
     *
     * This tree model provides a tree based structure for the most common filter options.
     *
     * @param parent the QObject parent
     */
    explicit FilterCriteriaModel(QObject *parent = nullptr);

    /**
     * @brief Destroys the object
     */
    ~FilterCriteriaModel() override;

    /**
     * Reset model by reading from a new journald database
     *
     * @param path The path to directory that obtains the journald DB, usually ending with "journal".
     */
    void setJournalProvider(IJournalProvider *provider);

    IJournalProvider *journalProvider() const;

    /**
     * @return the currently selected priority threshold for displayed log entries
     */
    int priorityFilter() const;

    /**
     * @return the list of enabled user units
     */
    QStringList systemdUserUnitFilter() const;

    /**
     * @return the list of enabled user units
     */
    QStringList systemdSystemUnitFilter() const;

    /**
     * @return true of templated systemd units are grouped by template name
     */
    bool groupTemplatedSystemdUnits() const;

    /**
     * Enable or disable grouping of systemd units by their template name by value @p enabled
     * Whenever the value is changed, the groupTemplatedSystemdUnitsChanged signal is fired.
     */
    void setGroupTemplatedSystemdUnits(bool enabled);

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
    void systemdUserUnitFilterChanged();
    void systemdSystemUnitFilterChanged();
    void exeFilterChanged();
    void kernelFilterChanged();
    void journalProviderChanged();
    void groupTemplatedSystemdUnitsChanged();

private:
    std::unique_ptr<FilterCriteriaModelPrivate> d;
};

#endif
