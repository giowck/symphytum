/**
  * \class CollectionListModel
  * \brief This model holds all collection names and
  *        is used by the collection list widget
  *        in main window's dock widget. This model is editable
  *        allowing collection renaming.
  * \author Giorgio Wicklein - GIOWISYS Software
  * \date 07/06/2012
  */

#ifndef COLLECTIONLISTMODEL_H
#define COLLECTIONLISTMODEL_H


//-----------------------------------------------------------------------------
// Headers
//-----------------------------------------------------------------------------

#include <QtSql/QSqlTableModel>


//-----------------------------------------------------------------------------
// Forward declarations
//-----------------------------------------------------------------------------



//-----------------------------------------------------------------------------
// CollectionListModel
//-----------------------------------------------------------------------------

class CollectionListModel : public QSqlTableModel
{
    Q_OBJECT

public:
    explicit CollectionListModel(QObject *parent = nullptr);

    /** Reimplemented for icons (decoration role) */
    QVariant data(const QModelIndex &idx, int role) const;

    /** Reimplement to avoid empty data */
    bool setData(const QModelIndex &index, const QVariant &value, int role);

    /** Add an empty collection to the end
     * @param collectionOrder - order int for position in collection list
     */
    void addCollection(const int collectionOrder,
                       const QString &name = tr("New Collection"));
    
private:
    
};

#endif // COLLECTIONLISTMODEL_H
