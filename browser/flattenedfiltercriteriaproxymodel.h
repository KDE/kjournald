/*
    SPDX-License-Identifier: LGPL-2.1-or-later OR MIT
    SPDX-FileCopyrightText: 2021 Andreas Cord-Landwehr <cordlandwehr@kde.org>
*/

#ifndef FLATTENEDFILTERCRITERIAPROXYMODEL_H
#define FLATTENEDFILTERCRITERIAPROXYMODEL_H

#include <QAbstractListModel>
#include <QHash>

/**
 * @brief Linear list proxy model for QML integration of @see FilterCriteriaModel
 *
 * This proxy model takes a tree model based @see FilterCriteriaModel and provides its linearization as
 * a @a QAbstractItemModel. This easies integration with QML where a ListModel or Repeater based vew can be constructed
 * that emulates the behavior of such a tree model.
 *
 * Specifically, this model provides expansing and collapsing of first level nodes.
 *
 * @note the model is constructud to deeply integrate with the FilterCriteriaModel and thus requires guarantees
 * of that model (e.g. max-level is 2).
 */
class FlattenedFilterCriteriaProxyModel : public QAbstractListModel
{
    Q_OBJECT

    /**
     * FilterCriteriaModel, which holds data of this proxy model
     */
    Q_PROPERTY(QAbstractItemModel *sourceModel READ sourceModel WRITE setSourceModel NOTIFY sourceModelChanged);

public:
    /** model roles **/
    enum Roles {
        TEXT = Qt::DisplayRole, //!< the user visible (possibley abbreviated) text
        LONGTEXT = Qt::ToolTipRole, //!< the long text which typically is used for tooltips
        SELECTED = Qt::CheckStateRole, //!< boolean role that informs if element is selected
        INDENTATION = Qt::UserRole + 1, //!< provides indentation level based in source model tree structure
        EXPANDED = Qt::UserRole + 2, //!< for expansible nodes, provides state if node is expanded
        TYPE = Qt::UserRole + 3, //!< provides data type for nodes
        COLOR = Qt::UserRole + 4, //!< provides color state for node, if present
    };
    Q_ENUM(Roles)

    enum DelgateType {
        SECTION,
        CHECKBOX,
        RADIOBUTTON,
        UNSELECT_OPTION,
    };
    Q_ENUM(DelgateType)

    /**
     * @copydoc QAbstractItemModel::roleNames()
     */
    QHash<int, QByteArray> roleNames() const override;

    /**
     * @brief setter for source model @p model
     */
    void setSourceModel(QAbstractItemModel *model);

    /**
     * @return current sourde model of proxy
     */
    QAbstractItemModel *sourceModel() const;

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
     * @copydoc QAbstractItemModel::setData()
     */
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;

private Q_SLOTS:
    void handleSourceModelDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles);

Q_SIGNALS:
    /**
     * @brief signal is fired when the source model was changed
     */
    void sourceModelChanged();

private:
    /**
     * @brief Proxy data objects that store references to source model objects
     */
    struct SourceIndex {
        QModelIndex mSourceIndex;
        bool mIsExpanded{false};
        int mIndentation{0};
    };
    QAbstractItemModel *mSourceModel{nullptr};
    QVector<SourceIndex> mMapToSourceIndex;
};

#endif
