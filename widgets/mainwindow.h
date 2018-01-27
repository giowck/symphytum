/**
  * \class MainWindow
  * \brief The main window of Symphytum application
  * \author Giorgio Wicklein - GIOWISYS Software
  * \date 22/03/2012
  */

#ifndef MAINWINDOW_H
#define MAINWINDOW_H


//-----------------------------------------------------------------------------
// Headers
//-----------------------------------------------------------------------------

#include <QtWidgets/QMainWindow>
#include <QtCore/QMap>


//-----------------------------------------------------------------------------
// Forward declarations
//-----------------------------------------------------------------------------

class QToolBar;
class QDockWidget;
class QMenu;
class QAction;
class QActionGroup;
class QStackedWidget;
class FormView;
class TableView;
class ViewToolBarWidget;
class DockWidget;
class SettingsManager;
class QAbstractItemModel;
class MetadataEngine;
class AddFieldDialog;
class QUndoStack;
class SyncEngine;
class UpdateManager;
class AlarmListDialog;


//-----------------------------------------------------------------------------
// MainWindow
//-----------------------------------------------------------------------------

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

    /**
     * ViewModes lists the supported view modes. The values of this enum
     * are used in settings or in main window to point to the current view mode
     */
    enum ViewMode {
        FormViewMode = 0,
        TableViewMode = 1
    };

    /**
     * Return the current active view mode,
     * static to allow access from everywhere
     */
    static ViewMode getCurrentViewMode();

    /**
     * Return the current active data model,
     * static to allow access from everywhere
     */
    static QAbstractItemModel* getCurrentModel();

    /** Get status bar */
    static QStatusBar* getStatusBar();

    /** Get undo stack */
    static QUndoStack* getUndoStack();

protected:
    void closeEvent(QCloseEvent *event);

private slots:
    void aboutActionTriggered();
    void aboutQtActionTriggered();
    void preferenceActionTriggered();
    void formViewModeTriggered();
    void tableViewModeTriggered();
    void fullscreenActionTriggered();
    void toggleDockActionTriggered();
    void currentCollectionIdChanged(int collectionId);
    void currentCollectionChanged();
    void newRecordActionTriggered();
    void duplicateRecordActionTriggered();
    void deleteRecordActionTriggered();
    void newCollectionActionTriggered();
    void syncReadOnlyActionTriggered();
    void duplicateCollectionActionTriggered();
    void deleteCollectionActionTriggered();
    void deleteAllRecordsActionTriggered();
    void optimizeDbSizeActionTriggered();
    void newFieldActionTriggered();
    void duplicateFieldActionTriggered();
    void deleteFieldActionTriggered();
    void modifyFieldActionTriggered();
    void searchSlot(const QString &s);
    void selectAllActionTriggered();
    void backupActionTriggered();
    void printActionTriggered();
    void exportActionTriggered();
    void importActionTriggered();

    /** Create and show alarm list dialog */
    void showAlarmListDialog();

    /** Show record alarm */
    void showRecordfromAlarm(int collectionId,
                             int fieldId,
                             int recordId);

    //sync slots
    void syncActionTriggered();
    void syncErrorSlot(const QString &message);
    void syncClientAlreadyLoggedInSlot();
    void syncSessionKeyChangedSlot();
    void syncNewRevisionAvailableSlot();
    void syncRevisionConflictSlot();
    void syncAuthTokenExpiredSlot();
    void syncStatusChanged();

    //updates
    void checkForUpdatesSlot();
    void noUpdateFoundSlot();
    void updateErrorSlot();

private:
    void createActions();
    void createToolBar();
    void createDockWidget();
    void createMenu();
    void createStatusBar();
    void createComponents();
    void createCentralWidget();
    void createConnections();
    void restoreSettings();
    void saveSettings();
    void init();

    /**
     * This method creates an appropriate model for the specified collection id
     * and sets up the views
     */
    void attachModelToViews(const int collectionId);

    /**
     * This methods detaches the model for the currently active
     * collection from the views. Then the model is deleted.
     */
    void detachModelFromViews();

    /** Set model on collection list view */
    void attachCollectionModelView();

    /** Detach model from collection list view */
    void detachCollectionModelView();

    /** If sync is configured start session */
    void initSync();

    /**
     * Show SyncProgressDialog and start sync
     * @param autoclose - whether the dialog should be auto-closed after sync
     */
    void showSyncDialog(bool autoclose = false);

    /** Create sync specific connections */
    void createSyncConnections();

    /** Remove connections to sync components */
    void removeSyncConnections();

    /** Check if there are any alarm triggers to show in the AlarmListDialog */
    void checkAlarmTriggers();

    QToolBar *m_toolBar;
    QDockWidget *m_dockContainerWidget;
    DockWidget *m_dockWidget;
    QMenu *m_fileMenu;
    QMenu *m_newMenu;
    QMenu *m_editMenu;
    QMenu *m_toolsMenu;
    QMenu *m_cloudMenu;
    QMenu *m_recordsMenu;
    QMenu *m_databaseMenu;
    QMenu *m_viewMenu;
#ifdef Q_OS_OSX
    QMenu *m_windowMenu;
#endif //Q_OS_OSX
    QMenu *m_helpMenu;
    QAction *m_quitAction;
    QAction *m_aboutAction;
    QAction *m_aboutQtAction;
    QAction *m_newCollectionAction;
    QAction *m_duplicateCollectionAction;
    QAction *m_deleteCollectionAction;
    QAction *m_newRecordAction;
    QAction *m_newFieldAction;
    QAction *m_backupAction;
    QAction *m_settingsAction;
    QAction *m_undoAction;
    QAction *m_redoAction;
    QAction *m_findAction;
    QAction *m_viewModeActionSeparator;
    QAction *m_formViewModeAction;
    QAction *m_tableViewModeAction;
    QAction *m_fullscreenAction;
    QAction *m_toggleDockAction;
    QAction *m_deleteAllRecordsAction;
    QAction *m_optimizeDbSizeAction;
    QAction *m_syncAction;
    QAction *m_syncReadOnlyAction;
    QAction *m_selectAllAction;
    QAction *m_checkUpdatesAction;
    QAction *m_showAlarmDialogAction;
    QAction *m_printAction;
    QAction *m_exportAction;
    QAction *m_importAction;
#ifdef Q_OS_OSX
    QAction *m_minimizeAction;
    QAction *m_closeWindowAction;
#endif //Q_OS_OSX
    QActionGroup *m_viewModeActionGroup;
    QStackedWidget *m_centralStackedWidget; /**< The central stackable widget of MainWindow */
    QStackedWidget *m_viewStackedWidget; /**< This stacked widget is used to change
                                              between form/table view mode */
    FormView *m_formView;
    TableView *m_tableView;
    ViewToolBarWidget *m_viewToolBar;
    SettingsManager *m_settingsManager;
    MetadataEngine *m_metadataEngine;
    AddFieldDialog *m_addFieldDialog;
    SyncEngine *m_syncEngine;
    UpdateManager *m_updateManager;
    AlarmListDialog *m_alarmListDialog;

    int m_lastUsedCollectionId;
    QMap<int, int> m_collectionSessionIndexMap; /**< save collection id, row id pairs during session */

    //static
    static ViewMode m_currentViewMode; /**< This holds the current view mode */
    static QAbstractItemModel *m_currentModel; /** The current active data model */
    static QStatusBar *m_statusBar; /** The status bar */
    static QUndoStack *m_undoStack;
};

#endif // MAINWINDOW_H
