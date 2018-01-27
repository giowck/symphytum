/**
  * \class CollectionListView
  * \brief This view shows all collections in a list in the dock widget of the
  *        main window. The selected item represents the currently active
  *        collection.
  * \author Giorgio Wicklein - GIOWISYS Software
  * \date 17/06/2012
  */

#ifndef COLLECTIONLISTVIEW_H
#define COLLECTIONLISTVIEW_H


//-----------------------------------------------------------------------------
// Headers
//-----------------------------------------------------------------------------

#include <QtWidgets/QListView>


//-----------------------------------------------------------------------------
// Forward declarations
//-----------------------------------------------------------------------------

class CollectionListModel;
class QAction;


//-----------------------------------------------------------------------------
// CollectionListView
//-----------------------------------------------------------------------------

class CollectionListView : public QListView
{
    Q_OBJECT

public:
    explicit CollectionListView(QWidget *parent = 0);

    /**
     * Create a new collection and move focus on newly created item
     * to allow direct renaming
     */
    void createNewCollection();

    /** Delete the selected collection */
    void deleteCollection();

    /** Duplicate the selected collection */
    void duplicateCollection();

    /** Attach collection model to view */
    void attachModel();

    /** Detach collection model from view */
    void detachModel();

protected:
    void currentChanged(const QModelIndex &current, const QModelIndex &previous);
    void contextMenuEvent(QContextMenuEvent *event);

private slots:
    /**
     * This slow is called when the user changes the current collection
     * by clicking on the list. The clicked item will become the new
     * active collection.
     */
    void collectionSelected(const QModelIndex &index);

    /** Called when editing of a collection item is finished */
    void editingFinished();

    /** Called when new collection action triggered on context menu */
    void newCollectionActionTriggered();

    /** Called on delete collection click from context menu */
    void deleteCollectionActionTriggered();

    /** Called on duplicate collection context menu click */
    void duplicateCollectionActionTriggered();

    /** Called on collection change */
    void currentCollectionIdChanged(int collectionId);

private:
    /** Select the collection from the list which is currently active */
    void selectCurrentCollection();

    CollectionListModel *m_model;
    QAction *m_deleteCollectionAction;
    QAction *m_newCollectionAction;
    QAction *m_duplicateCollectionAction;
    int m_currentCollectionId;
};

#endif // COLLECTIONLISTVIEW_H
