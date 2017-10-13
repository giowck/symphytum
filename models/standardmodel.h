/**
  * \class StandardModel
  * \brief This model is the standard model used for the storage
  *        of content data. It uses the default SQLite database
  *        of Symphytum as storage service.
  * \author Giorgio Wicklein - GIOWISYS Software
  * \date 07/06/2012
  */

#ifndef STANDARDMODEL_H
#define STANDARDMODEL_H


//-----------------------------------------------------------------------------
// Headers
//-----------------------------------------------------------------------------

#include <QtSql/QSqlTableModel>


//-----------------------------------------------------------------------------
// Forward declarations
//-----------------------------------------------------------------------------

class MetadataEngine;


//-----------------------------------------------------------------------------
// StandardModel
//-----------------------------------------------------------------------------

class StandardModel : public QSqlTableModel
{
    Q_OBJECT

public:
    explicit StandardModel(MetadataEngine *meta, QObject *parent = 0);

    /**
     * Reimplemented headerData because the column names are queried from metadata
     * and not directly from database column names
     */
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const;

    /**
     * Reimplemented sort to emit layoutChanged() signal so FormView
     * and any other attached view reloads its data after sort operation
     */
    void sort(int column, Qt::SortOrder order);

    /** Add a new empty record to the model */
    void addRecord();

    /** Duplicate the specified row */
    void duplicateRecord(int row);

    /** Reimplemented to notify views that rows have been deleted (after deketion) */
    bool removeRows(int row, int count, const QModelIndex &parent);

    /**
     * This method has been introduced as a workaround for the default
     * rowCount() method, which returns 256 when using SQLite because
     * SQLite driver doesn't support size() on queries. So this method
     * returns the real row count by calling fetchMore() when needed
     */
    int realRowCount();

    /** Reimplement to avoid edits on read only session */
    bool setData(const QModelIndex &index, const QVariant &value, int role);

signals:
    /** Emitted after a model sort operation */
    void modelSortedSignal(int column);

    /**
     * This signal is used to inform attached views that rows have been deleted.
     * This is emitted _after_ row deletion operations and not before like
     * other methods exposed by QSqlTableModel
     * @param startRow - the first row that has been deleted
     * @param count - the deleted rows count beginning with startRow
     */
    void rowsDeleted(int startRow, int count);

private:
    MetadataEngine *m_metadataEngine;
};

#endif // STANDARDMODEL_H
