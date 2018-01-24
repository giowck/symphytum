/**
  * \class DockWidget
  * \brief This widget represents the content of the QDockWidget in main mindow.
  *        The dock contains a tree-view of the collection's hierarchy and other
  *        indicator/status widgets.
  * \author Giorgio Wicklein - GIOWISYS Software
  * \date 23/05/2012
  */

#ifndef DOCKWIDGET_H
#define DOCKWIDGET_H


//-----------------------------------------------------------------------------
// Headers
//-----------------------------------------------------------------------------

#include <QWidget>


//-----------------------------------------------------------------------------
// Forward declarations
//-----------------------------------------------------------------------------

class QVBoxLayout;
class CollectionListView;
class SyncStatusWidget;


//-----------------------------------------------------------------------------
// DockWidget
//-----------------------------------------------------------------------------

class DockWidget : public QWidget
{
    Q_OBJECT

public:
    explicit DockWidget(QWidget *parent = 0);

    /** Create a new collection */
    void createNewCollection();

    /** Duplicate selected collection */
    void duplicateCollection();

    /** Delete selected collection */
    void deleteCollection();

    /** Return the collection list item view */
    CollectionListView* getCollectionListView();

public slots:
    /**
     * Enable/disable sync status widget based on whether
     * sync is enabled or not
     */
    void updateSyncStatusWidgetVisibility();

    /** Update sync widget */
    void updateSyncStatusWidget();

private:
    QVBoxLayout *m_mainLayout;
    CollectionListView *m_collectionListView;
    SyncStatusWidget *m_syncStatusWidget;
};

#endif // DOCKWIDGET_H
