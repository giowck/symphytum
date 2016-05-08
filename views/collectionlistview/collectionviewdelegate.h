/**
  * \class CollectionViewDelegate
  * \brief This is the main delegate for CollectionListView.
  * \author Giorgio Wicklein - GIOWISYS Software
  * \date 03/08/2012
  */

#ifndef COLLECTIONVIEWDELEGATE_H
#define COLLECTIONVIEWDELEGATE_H


//-----------------------------------------------------------------------------
// Headers
//-----------------------------------------------------------------------------

#include <QtWidgets/QStyledItemDelegate>


//-----------------------------------------------------------------------------
// Forward declarations
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// CollectionViewDelegate
//-----------------------------------------------------------------------------

class CollectionViewDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    explicit CollectionViewDelegate(QObject *parent = 0);

    /** Reimplemented for undo */
    void setModelData(QWidget *editor, QAbstractItemModel *model,
                      const QModelIndex &index) const;
    
};

#endif // COLLECTIONVIEWDELEGATE_H
