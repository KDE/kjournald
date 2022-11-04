/*
    SPDX-License-Identifier: LGPL-2.1-or-later OR MIT
    SPDX-FileCopyrightText: 2021 Andreas Cord-Landwehr <cordlandwehr@kde.org>
*/

#ifndef JOURNALDUNIQUEQUERYMODEL_H
#define JOURNALDUNIQUEQUERYMODEL_H

#include "journaldhelper.h"
#include "kjournald_export.h"
#include <QAbstractItemModel>
#include <memory>

class JournaldUniqueQueryModelPrivate;

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
class KJOURNALD_EXPORT JournaldUniqueQueryModel : public QAbstractItemModel
{
    Q_OBJECT
    Q_PROPERTY(QString journalPath WRITE setJournaldPath RESET setSystemJournal)
    Q_PROPERTY(QString field WRITE setFieldString)

public:
    enum Roles {
        FIELD = Qt::UserRole + 1,
        SELECTED, //!< supports UI integration by storing checked
    };
    Q_ENUM(Roles)

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

    /**
     * @brief Destroys the JournaldUniqueQueryModel object
     */
    ~JournaldUniqueQueryModel() override;

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
     * Set field for which unique query shall be run
     *
     * This method allows defining an arbitrary string as query string. The most common fields are available
     * by type-safe @a setField setter. Examples for the argument are "_SYSTEMD_UNIT", "PRIORITY", "_BOOT_ID".
     *
     * @param fieldString the string that names the field
     */
    void setFieldString(const QString &fieldString);

    /**
     * Set field for which unique query shall be run
     *
     * @param field enum for field for which query shall be run
     */
    void setField(JournaldHelper::Field field);

    /**
     * @return the currently set field string
     */
    QString fieldString() const;

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

private:
    std::unique_ptr<JournaldUniqueQueryModelPrivate> d;
};

#endif // JOURNALDUNIQUEQUERYMODEL_H
