/*
 *  Copyright (c) 2012 Giorgio Wicklein <giowckln@gmail.com>
 */

//-----------------------------------------------------------------------------
// Hearders
//-----------------------------------------------------------------------------

#include "mainwindow.h"
#include "../views/formview/formview.h"
#include "../views/tableview/tableview.h"
#include "viewtoolbarwidget.h"
#include "dockwidget.h"
#include "../utils/definitionholder.h"
#include "../components/settingsmanager.h"
#include "../components/metadataengine.h"
#include "../components/databasemanager.h"
#include "../components/filemanager.h"
#include "../components/undocommands.h"
#include "../models/standardmodel.h"
#include "../views/collectionlistview/collectionlistview.h"
#include "field_widgets/addfielddialog.h"
#include "syncconfigdialog.h"
#include "../components/sync_framework/syncengine.h"
#include "syncprocessdialog.h"
#include "preferencesdialog.h"
#include "backupdialog.h"
#include "printdialog.h"
#include "exportdialog.h"
#include "importdialog.h"
#include "alarmlistdialog.h"
#include "aboutdialog.h"
#include "upgradesuccessdialog.h"
#include "../components/updatemanager.h"
#include "../components/alarmmanager.h"
#include "../utils/collectionfieldcleaner.h"

#include <QApplication>
#include <QtWidgets/QToolBar>
#include <QtWidgets/QDockWidget>
#include <QtWidgets/QMenu>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QStackedWidget>
#include <QtGui/QCloseEvent>
#include <QVBoxLayout>
#include <QtCore/QString>
#include <QtCore/QAbstractItemModel>
#include <QtCore/QTimer>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QProgressDialog>
#include <QtWidgets/QUndoStack>
#include <QtGui/QDesktopServices>
#include <QtCore/QUrl>


//-----------------------------------------------------------------------------
// Static init
//-----------------------------------------------------------------------------

MainWindow::ViewMode MainWindow::m_currentViewMode = MainWindow::FormViewMode;
QAbstractItemModel* MainWindow::m_currentModel = nullptr;
QStatusBar* MainWindow::m_statusBar = nullptr;
QUndoStack* MainWindow::m_undoStack = nullptr;


//-----------------------------------------------------------------------------
// Public
//-----------------------------------------------------------------------------

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      m_addFieldDialog(nullptr),
      m_updateManager(nullptr),
      m_alarmListDialog(nullptr),
      m_lastUsedCollectionId(0)
{
    //init GUI elements
    createComponents();
    createActions();
    createToolBar();
    createDockWidget();
    createMenu();
    createStatusBar();
    createCentralWidget();
    createConnections();

    //set up views with an appropriate model for the last used collection
    attachModelToViews(m_metadataEngine->getCurrentCollectionId());

    restoreSettings();
    init();
    checkAlarmTriggers();
    initSync();
    checkForUpdatesSlot();

    setWindowTitle(tr("%1").arg(DefinitionHolder::NAME));
    setMinimumHeight(500);

    checkDonationSuggestion();
}

MainWindow::~MainWindow()
{
    detachModelFromViews();
    detachCollectionModelView();
    delete m_settingsManager;

    m_syncEngine->destroy();
    m_metadataEngine->destroy();
    DatabaseManager::destroy();
}

MainWindow::ViewMode MainWindow::getCurrentViewMode()
{
    return m_currentViewMode;
}

QAbstractItemModel* MainWindow::getCurrentModel()
{
    return m_currentModel;
}

QStatusBar* MainWindow::getStatusBar()
{
    return m_statusBar;
}

QUndoStack* MainWindow::getUndoStack()
{
    return m_undoStack;
}


//-----------------------------------------------------------------------------
// Protected
//-----------------------------------------------------------------------------

void MainWindow::closeEvent(QCloseEvent *event)
{
    //set focus, to save form view
    setFocus();

    //actions before shutting down
    saveSettings();

    if (m_settingsManager->isCloudSyncActive() &&
            SyncSession::IS_ONLINE &&
            (!SyncSession::IS_READ_ONLY)) {

        //sync if data changed
        if (SyncSession::LOCAL_DATA_CHANGED)
            showSyncDialog(true);

        //close sync session
        connect(m_syncEngine, SIGNAL(syncSessionClosed()),
                qApp, SLOT(quit()));
        connect(m_syncEngine, SIGNAL(connectionFailed()),
                qApp, SLOT(quit()));
        m_syncEngine->startCloseCloudSession();

        event->ignore();
    } else {
        event->accept();
    }
}


//-----------------------------------------------------------------------------
// Private slots
//-----------------------------------------------------------------------------

void MainWindow::aboutActionTriggered()
{
    AboutDialog d(this);
    d.exec();
}

void MainWindow::aboutQtActionTriggered()
{
    qApp->aboutQt();
}

void MainWindow::onlineDocActionTriggered()
{
    QUrl helpUrl(DefinitionHolder::HELP_URL);
    QDesktopServices::openUrl(helpUrl);
}

void MainWindow::donateActionTriggered()
{
    QUrl url(DefinitionHolder::DONATE_URL);
    QDesktopServices::openUrl(url);
}

void MainWindow::preferenceActionTriggered()
{
    PreferencesDialog dialog(this);
    dialog.exec();

    //handle changes that requires triggers
    if (dialog.appearanceChanged()) {
        if (m_formView) {
            m_formView->reloadAppearanceSettings();
            //reload because some settings like pruneUnusedSpace
            //require components like FormView (and FormLayoutMatrix) to be reloaded
            //to read new settings
            this->currentCollectionChanged();
        }
        if (m_tableView) {
            m_tableView->reloadRowSize();
        }
    }
    if (dialog.cloudSyncChanged()) {
        if (m_settingsManager->isCloudSyncActive()) {
            if(m_settingsManager->isCloudSyncInitialized()) {
                m_syncEngine->reconfigureSyncDriver();
                initSync();
            } else {
                m_settingsManager->setCloudSyncActive(false);
                syncActionTriggered();
            }
        } else {
            removeSyncConnections();
        }
        m_dockWidget->updateSyncStatusWidgetVisibility();
        m_dockWidget->updateSyncStatusWidget();
    }
    if (dialog.softwareResetActivated()) {
        //ask for confirmation
        QMessageBox box(QMessageBox::Warning, tr("Software Reset"),
                        tr("Are you sure you want to delete all data "
                           "from the database including all files and settings?"
                           "<br><br><b>Warning:</b> This cannot be undone!"),
                        QMessageBox::Yes | QMessageBox::No,
                        this);
        box.setDefaultButton(QMessageBox::Yes);
        box.setWindowModality(Qt::WindowModal);
        int r = box.exec();
        if (r == QMessageBox::No) return;

        //close sync session if open
        if (SyncSession::IS_ENABLED) {
            if (SyncSession::IS_ONLINE && (!SyncSession::IS_READ_ONLY)) {
                QProgressDialog pd(this);
                pd.setWindowModality(Qt::WindowModal);
                pd.setWindowTitle(tr("Closing Session"));
                pd.setLabelText(tr("Closing sync session... Please wait!"));
                pd.setRange(0, 0);
                pd.setValue(-1);

                connect(m_syncEngine, SIGNAL(syncSessionClosed()),
                        &pd, SLOT(close()));
                m_syncEngine->startCloseCloudSession();

                //wait until session is closed
                pd.exec();
            }
        }

        QProgressDialog *pd = new QProgressDialog(this);
        pd->setWindowModality(Qt::WindowModal);
        pd->setWindowTitle(tr("Progress"));
        pd->setLabelText(tr("Deleting files... Please wait!"));
        pd->setRange(0, 6);
        pd->setValue(1);
        pd->show();
        qApp->processEvents();

        QString fullDbPath = DatabaseManager::getInstance().getDatabasePath();

        //detach views
        detachModelFromViews();
        detachCollectionModelView();
        pd->setValue(2);
        qApp->processEvents();

        //delete all files
        FileManager fm(this);
        fm.removeAllFiles();
        pd->setValue(3);
        qApp->processEvents();

        //close database
        DatabaseManager::getInstance().destroy();
        pd->setValue(4);
        qApp->processEvents();

        //delete db
        QFile::remove(fullDbPath);
        pd->setValue(5);
        qApp->processEvents();

        //delete all settings
        m_settingsManager->removeAllSettings();
        pd->setValue(6);
        qApp->processEvents();

        //close app
        QMessageBox box2(QMessageBox::Information, tr("Software Reset"),
                        tr("Software successfully resetted. "
                           "Terminating now."),
                        QMessageBox::NoButton,
                        this);
        box2.setWindowModality(Qt::WindowModal);
        box2.exec();
        qApp->quit();
    }
    if (dialog.databasePathChanged()) {
        QMessageBox::information(this, tr("Database Directory Change"),
                                 tr("The database directory change will not take effect"
                                    " until software restart."
                                    "<br />Use the backup function to export and import your "
                                    "data to the new location."
                                    "<br />If the selected directory is empty,"
                                    " a new database file will be created."));

        //if sync enabled close session
        if (SyncSession::IS_ENABLED) {
            if (SyncSession::IS_ONLINE && (!SyncSession::IS_READ_ONLY)) {
                QProgressDialog pd(this);
                pd.setWindowModality(Qt::WindowModal);
                pd.setWindowTitle(tr("Closing Session"));
                pd.setLabelText(tr("Closing sync session... Please wait!"));
                pd.setRange(0, 0);
                pd.setValue(-1);

                connect(m_syncEngine, SIGNAL(syncSessionClosed()),
                        &pd, SLOT(close()));
                m_syncEngine->startCloseCloudSession();

                //wait until session is closed
                pd.exec();

                //disable sync
                m_settingsManager->setCloudSyncActive(false);

                //cause conflict
                m_settingsManager->saveCloudLocalDataChanged(true);
                m_settingsManager->saveCloudSessionKey("invalid");
            }
        }
    }
}

void MainWindow::formViewModeTriggered()
{
    //set focus to form view and not on its
    //form widgets, to avoid random input
    m_formView->setFocus();

    m_viewStackedWidget->setCurrentIndex(0);
    m_formViewModeAction->setChecked(true);

    //update view toolbar buttons
    m_viewToolBar->setViewModeState(ViewToolBarWidget::FormViewMode);

    m_currentViewMode = FormViewMode;
}

void MainWindow::tableViewModeTriggered()
{
    m_viewStackedWidget->setCurrentIndex(1);
    m_tableViewModeAction->setChecked(true);

    //update view toolbar buttons
    m_viewToolBar->setViewModeState(ViewToolBarWidget::TableViewMode);

    m_currentViewMode = TableViewMode;

    //set current row (record) from FormView
    //this is needed because when editing in form
    //view mode the selection (current) is lost
    int row = m_formView->getCurrentRow();
    if (row != -1) {
        m_tableView->clearSelection();
        m_tableView->setCurrentIndex(m_currentModel->index(row, 1));
    }
}

void MainWindow::fullscreenActionTriggered()
{
/*#ifdef Q_OS_OSX
    if (QSysInfo::MacintoshVersion >= QSysInfo::MV_10_7) {
        MacLionFullscreenProvider::toggleFullscreen(this); //mac lion fullscreen API
    } else {
        if (isFullScreen()) {
            showNormal();
            setUnifiedTitleAndToolBarOnMac(true); //workaround for QTBUG-16274
        } else {
            setUnifiedTitleAndToolBarOnMac(false); //workaround for QTBUG-16274
            showFullScreen();
        }
    }
#else*/
    if (isFullScreen()) {
        showNormal();
    } else {
        showFullScreen();
    }
//#endif // Q_OS_OSX
}

void MainWindow::toggleDockActionTriggered()
{
    m_dockContainerWidget->setHidden(m_toggleDockAction->isChecked());
}

void MainWindow::currentCollectionIdChanged(int collectionId)
{
    //save last active record id (row) to restore it on changeback
    if (m_formView && m_currentModel) {
        QModelIndex index = m_formView->currentIndex();
        if (index.isValid()) {
            int recordId = index.row();
            m_collectionSessionIndexMap[m_lastUsedCollectionId] = recordId;
        }
    }

    reattachModelToViews(collectionId);

    //restore last active record id (row) if previously saved
    if (m_formView && m_currentModel) {
        int recordId = m_collectionSessionIndexMap[collectionId];
        if (recordId != 0) {
            QModelIndex index = m_formView->model()->index(recordId, 1);
            if (index.isValid())
                m_tableView->setCurrentIndex(index);
        }
    }
}

void MainWindow::currentCollectionChanged()
{
    int currentRow = m_formView->getCurrentRow();

    //for now only StandardCollection as CollectionType
    //is supprted, if new types are added this method needs
    //some adjustments

    //save search filter, in order to restore correct record and index
    StandardModel *sModel = qobject_cast<StandardModel*>(m_currentModel);
    QString currentFilter;
    if (sModel) {
        currentFilter = sModel->filter();
    }

    reattachModelToViews(m_metadataEngine->getCurrentCollectionId());

    //restore search filter, in order to restore correct record and index
    if (sModel && (!currentFilter.isEmpty())) {
        sModel->setFilter(currentFilter);
    }

    //restore current row
    QModelIndex index = m_formView->model()->index(currentRow, 1);
    if (index.isValid())
        m_tableView->setCurrentIndex(index);
}

void MainWindow::newRecordActionTriggered()
{
    if (!m_currentModel || SyncSession::IS_READ_ONLY) return;

    if (m_metadataEngine->getFieldCount() <= 1) { //1 cause of _id
        QMessageBox box(QMessageBox::Critical, tr("No Fields"),
                        tr("Failed to create new record!<br>"
                           "Add some fields first."),
                        QMessageBox::NoButton,
                        this);
        box.setWindowModality(Qt::WindowModal);
        box.exec();
        return;
    }

    //for now only StandardCollection as CollectionType
    //is supprted, if new types are added this method needs
    //some adjustments

    StandardModel *sModel = qobject_cast<StandardModel*>(m_currentModel);
    if (sModel) {
        m_formView->setFocus(); //clear focus from form widgets to avoid edit events

        //clear search filter, if any
        if (!sModel->filter().isEmpty())
            sModel->setFilter(""); //clear filter

        //set form view enabled if it was not
        //form view is disabled by searchSlot()
        //if no search result
        if (!m_formView->isEnabled())
            m_formView->setEnabled(true);

        sModel->addRecord(); //add e new empty record
        int realRowCount = sModel->realRowCount();

        //set local data changed
        SyncSession::LOCAL_DATA_CHANGED = true;

        //create undo action
        QUndoCommand *cmd = new NewRecordCommand;
        m_undoStack->push(cmd);

        statusBar()->showMessage(tr("New record created"));

        //FIXME: temporary workaround for Qt5
        //views should atomatically update but don't
        //needs investigation
        //update views (hard way)
        reattachModelToViews(m_metadataEngine->getCurrentCollectionId());

        //select newly created record
        m_formView->selectionModel()->setCurrentIndex(
                    m_currentModel->index(realRowCount - 1, 1),
                    QItemSelectionModel::SelectCurrent);
    }
}

void MainWindow::duplicateRecordActionTriggered()
{
    if (!m_currentModel || SyncSession::IS_READ_ONLY) return;
    if (!m_currentModel->rowCount()) {
        QMessageBox box(QMessageBox::Critical, tr("Duplication Failed"),
                        tr("Failed to duplicate record!<br>"
                           "Add some records first."),
                        QMessageBox::NoButton,
                        this);
        box.setWindowModality(Qt::WindowModal);
        box.exec();
        return;
    }

    //for now only StandardCollection as CollectionType
    //is supprted, if new types are added this method needs
    //some adjustments

    StandardModel *sModel = qobject_cast<StandardModel*>(m_currentModel);
    if (!sModel) return;

    if (m_currentViewMode == FormViewMode) {
        int row = m_formView->getCurrentRow();
        QModelIndex index = m_currentModel->index(row, 0);
        if (index.isValid()) {
            m_formView->setFocus(); //clear focus from form widgets to avoid edit events

            //create undo action
            QUndoCommand *cmd = new DuplicateRecordCommand(row);
            m_undoStack->push(cmd);

            sModel->duplicateRecord(row); //add duplicated record of row
            int realRowCount = sModel->realRowCount();
            statusBar()->showMessage(tr("Record %1 duplicated").arg(row+1));

            //FIXME: temporary workaround for Qt5
            //views should atomatically update but don't
            //needs investigation
            //update views (hard way)
            reattachModelToViews(m_metadataEngine->getCurrentCollectionId());

            m_formView->selectionModel()->setCurrentIndex(
                        m_currentModel->index(realRowCount - 1, 1),
                        QItemSelectionModel::SelectCurrent);
        }
    } else { //table view
        QModelIndexList indexes = m_tableView->selectionModel()->selectedIndexes();
        QSet<int> rows;
        int indexesSize = indexes.size();
        for (int i = 0; i < indexesSize; i++) {
            rows.insert(indexes.at(i).row());
        }
        int rowsSize = rows.size();

        bool canUndo = rowsSize <= 100;
        if (!canUndo) {
            //ask for confirmation
            QMessageBox box(QMessageBox::Question, tr("Duplicate Record"),
                            tr("Are you sure you want to duplicate all selected records?"
                               "<br><br><b>Warning:</b> This cannot be undone!"),
                            QMessageBox::Yes | QMessageBox::No,
                            this);
            box.setDefaultButton(QMessageBox::Yes);
            box.setWindowModality(Qt::WindowModal);
            int r = box.exec();
            if (r == QMessageBox::No) return;
        }

        //create main undo action
        QUndoCommand *mainUndoCommand = new QUndoCommand(tr("record duplication"));

        //set local data changed
        SyncSession::LOCAL_DATA_CHANGED = true;

        //init progress dialog
        QProgressDialog progressDialog(tr("Duplicating record 0 of %1")
                                       .arg(rows.size()),
                                       tr("Cancel"), 1,
                                       rowsSize, this);
        progressDialog.setWindowModality(Qt::WindowModal);
        progressDialog.setWindowTitle(tr("Progress"));
        //workaroung bug where dialog remains hidden sometimes
        progressDialog.setMinimumDuration(0);
        progressDialog.show();
        int progress = 0;

        //duplicate selected rows
        m_currentModel->blockSignals(true); //speed up
        DatabaseManager::getInstance().beginTransaction(); //speed up writes

        foreach (const int &row, rows) {
            qApp->processEvents();
            if (progressDialog.wasCanceled())
                break;
            progressDialog.setValue(++progress);
            progressDialog.setLabelText(tr("Duplicating record %1 of %2")
                                        .arg(progress)
                                        .arg(rowsSize));

            if (canUndo) {
                //create child undo command
                new DuplicateRecordCommand(row, mainUndoCommand);
            }

            //duplicate
            sModel->duplicateRecord(row);
        }
        DatabaseManager::getInstance().endTransaction();
        m_currentModel->blockSignals(false);

        if (canUndo) {
            //setup main undo command
            m_undoStack->push(mainUndoCommand);
        } else {
            m_undoStack->clear();
        }

        int realRowCount = sModel->realRowCount(); //call before reattach because Smodel gets deleted
        //update views (hard way)
        reattachModelToViews(m_metadataEngine->getCurrentCollectionId());

        statusBar()->showMessage(tr("%1 record(s) duplicated").arg(progress));

        //select first duplicate
        m_formView->selectionModel()->setCurrentIndex(
                    m_currentModel->index(realRowCount - rows.size(), 1),
                    QItemSelectionModel::SelectCurrent);
    }

    //set focus back (workaround)
    this->activateWindow();
}

void MainWindow::deleteRecordActionTriggered()
{
    if (!m_currentModel || SyncSession::IS_READ_ONLY) return;
    if (!m_currentModel->rowCount()) {
        QMessageBox box(QMessageBox::Critical, tr("Deletion Failed"),
                        tr("Failed to delete record!<br>"
                           "The collection is empty."),
                        QMessageBox::NoButton,
                        this);
        box.setWindowModality(Qt::WindowModal);
        box.exec();
        return;
    }

    if (m_currentViewMode == FormViewMode) {
        //ask for confirmation
        QMessageBox box(QMessageBox::Question, tr("Delete Record"),
                        tr("Delete current record?"),
                        QMessageBox::Yes | QMessageBox::No,
                        this);
        box.setDefaultButton(QMessageBox::Yes);
        box.setWindowModality(Qt::WindowModal);
        int r = box.exec();
        if (r == QMessageBox::No) return;

        int currentRow = m_formView->getCurrentRow();
        QModelIndex index = m_currentModel->index(currentRow, 0);
        if (index.isValid()) {
            m_formView->setFocus(); //clear focus from form widgets to avoid edit events

            //create undo action
            QUndoCommand *cmd = new DeleteRecordCommand(index.row());
            m_undoStack->push(cmd);

            //remove
            m_currentModel->removeRow(index.row());

            //FIXME: temporary workaround for Qt5
            //views should atomatically update but don't
            //needs investigation
            //update views (hard way)
            //reattachModelToViews(); //no longer needed because StandardModel::rowsDeleted
                                      //gets connected in attachModelToViews

            statusBar()->showMessage(tr("Record %1 deleted").arg(index.row()+1));
        }
    } else { //table view
        QModelIndexList indexes = m_tableView->selectionModel()->selectedIndexes();
        QSet<int> rows;
        int indexesSize = indexes.size();
        for (int i = 0; i < indexesSize; i++) {
            rows.insert(indexes.at(i).row());
        }

        //select entire records
        m_tableView->selectionModel()->select(m_tableView->selectionModel()->selection(),
                                              QItemSelectionModel::ClearAndSelect |
                                              QItemSelectionModel::Rows);

        bool canUndo = rows.size() <= 100;
        if (canUndo) {
            //ask for confirmation
            QMessageBox box(QMessageBox::Question, tr("Delete Record"),
                            tr("Delete selected records?"),
                            QMessageBox::Yes | QMessageBox::No,
                            this);
            box.setDefaultButton(QMessageBox::Yes);
            box.setWindowModality(Qt::WindowModal);
            int r = box.exec();
            if (r == QMessageBox::No) return;
        } else {
            //ask for confirmation
            QMessageBox box(QMessageBox::Question, tr("Delete Record"),
                            tr("Are you sure you want to delete all selected records?"
                               "<br><br><b>Warning:</b> This cannot be undone!"),
                            QMessageBox::Yes | QMessageBox::No,
                            this);
            box.setDefaultButton(QMessageBox::Yes);
            box.setWindowModality(Qt::WindowModal);
            int r = box.exec();
            if (r == QMessageBox::No) return;
        }

        //create main undo action
        QUndoCommand *mainUndoCommand = new QUndoCommand(tr("record deletion"));

        //init progress dialog
        QProgressDialog progressDialog(tr("Deleting record 0 of %1")
                                       .arg(rows.size()),
                                       tr("Cancel"), 1,
                                       rows.size(), this);
        progressDialog.setWindowModality(Qt::WindowModal);
        progressDialog.setWindowTitle(tr("Progress"));
#ifdef Q_OS_WIN
        //workaroung for windows bug where dialog remains hidden sometimes
        progressDialog.setMinimumDuration(0);
        progressDialog.show();
#endif // Q_OS_WIN
        int progress = 0;

        //remove selected rows
        m_currentModel->blockSignals(true); //speed up
        DatabaseManager::getInstance().beginTransaction(); //speed up writes

        //sort rows in a list so that removing from bottom to top works
        QList<int> rowList = rows.toList();
        qSort(rowList.begin(), rowList.end());
        int rowListSize = rowList.size();
        for (int i = rowListSize - 1; i >= 0; i--) {
#ifdef Q_OS_WIN
            //workaroung for windows bug where dialog remains hidden sometimes
            qApp->processEvents();
#endif // Q_OS_WIN
            if (progressDialog.wasCanceled())
                break;
            progressDialog.setValue(++progress);
            progressDialog.setLabelText(tr("Deleting record %1 of %2")
                                        .arg(progress)
                                        .arg(rowListSize));
            int r = rowList.at(i);

            if (canUndo) {
                //create child undo command
                new DeleteRecordCommand(r, mainUndoCommand);
            } else {
                m_undoStack->clear();
            }

            //remove
            m_currentModel->removeRow(r);
        }
        DatabaseManager::getInstance().endTransaction();
        m_currentModel->blockSignals(false);

        if (canUndo) {
            //setup main undo command
            m_undoStack->push(mainUndoCommand);
        }

        //update views (hard way)
        reattachModelToViews(m_metadataEngine->getCurrentCollectionId());

        //status message
        statusBar()->showMessage(tr("%1 record(s) deleted").arg(progress));

        //select record before deleted items
        int previousRecord = 0;
        if (indexes.count() > 0 )
            previousRecord = indexes.at(0).row() - 1;

        m_formView->selectionModel()->setCurrentIndex(
                    m_currentModel->index(previousRecord, 1),
                    QItemSelectionModel::SelectCurrent);
    }

    //set local data changed
    SyncSession::LOCAL_DATA_CHANGED = true;

    //set focus back (workaround)
    this->activateWindow();
}

void MainWindow::newCollectionActionTriggered()
{
    m_dockWidget->createNewCollection();
}

void MainWindow::syncReadOnlyActionTriggered()
{
    if (!SyncSession::IS_ONLINE) {
        m_statusBar->showMessage(tr("Cloud not connected."));
        return;
    }

    if (SyncSession::IS_READ_ONLY) {
        SyncSession::IS_READ_ONLY = false;
        SyncSession::IS_ONLINE = false;

        //restore modified state
        SyncSession::LOCAL_DATA_CHANGED =
                m_settingsManager->restoreCloudLocalDataChanged();

        //show force access dialog
        m_syncEngine->startSyncCheck();
    } else {
        //save modified state
        m_settingsManager->saveCloudLocalDataChanged(SyncSession::LOCAL_DATA_CHANGED);

        //close session
        SyncSession::IS_READ_ONLY = true;
        m_syncEngine->startCloseCloudSession();
    }

    //clear undo stack
    m_undoStack->clear();
}

void MainWindow::duplicateCollectionActionTriggered()
{
    m_dockWidget->duplicateCollection();
}

void MainWindow::deleteCollectionActionTriggered()
{
    m_dockWidget->deleteCollection();
}

void MainWindow::deleteAllRecordsActionTriggered()
{
    if (SyncSession::IS_READ_ONLY) return;

    //ask for confirmation
    QMessageBox box(QMessageBox::Question, tr("Delete All Records"),
                    tr("Are you sure you want to delete all records from the "
                       "current collection?"
                       "<br><br><b>Warning:</b> This cannot be undone!"),
                    QMessageBox::Yes | QMessageBox::No,
                    this);
    box.setDefaultButton(QMessageBox::Yes);
    box.setWindowModality(Qt::WindowModal);
    int r = box.exec();
    if (r == QMessageBox::No) return;

    int id = m_metadataEngine->getCurrentCollectionId();
    m_metadataEngine->deleteAllRecords(id);

    //update views (hard way)
    reattachModelToViews(m_metadataEngine->getCurrentCollectionId());

    //set local data changed
    SyncSession::LOCAL_DATA_CHANGED = true;

    statusBar()->showMessage(tr("All records successfully deleted"));

    //set focus back (workaround)
    this->activateWindow();

    //clear undo stack since this action is not undoable
    m_undoStack->clear();
}

void MainWindow::optimizeDbSizeActionTriggered()
{
    if (SyncSession::IS_READ_ONLY) return;

    double bytesBefore = DatabaseManager::getInstance().getDatabaseFileSize();

    FileManager fm(this);

    //remove orphan file ids from database
    //(orphans are files that are in database but not used in any records)
    QStringList orphanFileIds = fm.orphanDatabaseFileList();
    foreach (QString orphanId, orphanFileIds) {
        fm.removeFileMetadata(orphanId.toInt());
    }

    //get unneeded local files (local files that are not in database)
    //this includes orphan files because they just got removed from database
    QStringList files = fm.unneededLocalFileList();


    QString filesDir = fm.getFilesDirectory();
    int steps = 1 + files.size();
    qint64 removedFileBytes = 0;

    //init progress dialog
    QProgressDialog progressDialog(tr("Removing obsolete data..."),
                                   tr("Cancel"), 0,
                                   steps, this);
    progressDialog.setWindowModality(Qt::WindowModal);
    progressDialog.setWindowTitle(tr("Progress"));
#ifdef Q_OS_WIN
    //workaroung for windows bug where dialog remains hidden sometimes
    progressDialog.setMinimumDuration(0);
    progressDialog.show();
#endif // Q_OS_WIN
    int progress = 0;

    //clean db
    DatabaseManager::getInstance().optimizeDatabaseSize();
    progress++;

    foreach (QString s, files) {
#ifdef Q_OS_WIN
        //workaroung for windows bug where dialog remains hidden sometimes
        qApp->processEvents();
#endif // Q_OS_WIN
        if (progressDialog.wasCanceled())
            break;

        //remove file
        removedFileBytes += QFile(filesDir + s).size();
        QFile::remove(filesDir + s);

        progressDialog.setValue(++progress);
    }

    //finished progress dialog should auto close
    //but steps don't match maximum() somehow
    //so force close
    progressDialog.close();

    double bytesAfter = DatabaseManager::getInstance().getDatabaseFileSize();
    double kib = (bytesBefore - bytesAfter) / (1024.0); //B -> KiB
    qint64 mib = removedFileBytes / (1024 * 1024); //b -> MiB

    QMessageBox box(QMessageBox::Information, tr("Database Size"),
                    tr("Database size reduced by %1 KiB\n"
                       "Files archive reduced by %2 MiB").arg(kib).arg(mib),
                    QMessageBox::NoButton,
                    this);
    box.setWindowModality(Qt::WindowModal);
    box.exec();

    //set local data changed
    SyncSession::LOCAL_DATA_CHANGED = true;

    //clear undo stack (deleted files/images are not undoable)
    m_undoStack->clear();
}

void MainWindow::newFieldActionTriggered()
{
    if ((!m_metadataEngine->getCurrentCollectionId()) ||
            SyncSession::IS_READ_ONLY) return;

    if (!m_addFieldDialog) {
        m_addFieldDialog = new AddFieldDialog(this);
        m_addFieldDialog->setWindowModality(Qt::WindowModal);
    }

    m_addFieldDialog->setCreationMode(AbstractFieldWizard::NewEditMode);
    m_addFieldDialog->show();
}

void MainWindow::duplicateFieldActionTriggered()
{
    if ((!m_metadataEngine->getCurrentCollectionId()) ||
            SyncSession::IS_READ_ONLY) return;

    int fieldId;
    if (m_currentViewMode == FormViewMode) {
        fieldId = m_formView->getSelectedField();
    } else {
        if (m_tableView->selectionModel()->hasSelection())
            fieldId = m_tableView->currentIndex().column();
        else
            fieldId = -1;
    }

    if (fieldId == -1) { //if no selection
        QMessageBox box(QMessageBox::Information, tr("Missing Field Selection"),
                        tr("Select a field to duplicate first!"),
                        QMessageBox::NoButton,
                        this);
        box.setWindowModality(Qt::WindowModal);
        box.exec();
        return;
    }

    if (!m_addFieldDialog) {
        m_addFieldDialog = new AddFieldDialog(this);
        m_addFieldDialog->setWindowModality(Qt::WindowModal);
    }

    m_addFieldDialog->setCreationMode(AbstractFieldWizard::DuplicateEditMode,
                                      fieldId,
                                      m_metadataEngine->getCurrentCollectionId());
    m_addFieldDialog->show();
}

void MainWindow::deleteFieldActionTriggered()
{
    if ((!m_metadataEngine->getCurrentCollectionId()) ||
            SyncSession::IS_READ_ONLY) return;

    int fieldId;
    if (m_currentViewMode == FormViewMode) {
        m_formView->setFocus(); //clear focus from form widgets to avoid edit events
        fieldId = m_formView->getSelectedField();
    } else {
        if (m_tableView->selectionModel()->hasSelection()) {
            fieldId = m_tableView->currentIndex().column();

            //select entire column
            QModelIndex currentIndex = m_tableView->currentIndex();
            m_tableView->selectionModel()->select(currentIndex,
                                                  QItemSelectionModel::ClearAndSelect |
                                                  QItemSelectionModel::Columns);
        } else {
            fieldId = -1;
        }
    }

    if (fieldId == -1) { //if no selection
        QMessageBox box(QMessageBox::Information, tr("Missing Field Selection"),
                        tr("Select a field to delete first!"),
                        QMessageBox::NoButton,
                        this);
        box.setWindowModality(Qt::WindowModal);
        box.exec();
        return;
    } else {
        //ask for confirmation
        QMessageBox box(QMessageBox::Question, tr("Field Deletion"),
                        tr("Are you sure you want to delete the selected "
                           "collection field with all data related?"
                           "<br><br><b>Warning:</b> This cannot be undone!"),
                        QMessageBox::Yes | QMessageBox::No,
                        this);
        box.setDefaultButton(QMessageBox::Yes);
        box.setWindowModality(Qt::WindowModal);
        int r = box.exec();
        if (r == QMessageBox::No) return;
    }

    //check field deletion trigger actions
    CollectionFieldCleaner cleaner(this);
    cleaner.cleanField(m_metadataEngine->getCurrentCollectionId(),
                       fieldId);

    //remove field
    m_metadataEngine->deleteField(fieldId);

    //set local data changed
    SyncSession::LOCAL_DATA_CHANGED = true;

    //clear undo stack since this action is not undoable
    m_undoStack->clear();
}

void MainWindow::modifyFieldActionTriggered()
{
    if (SyncSession::IS_READ_ONLY) return;

    int fieldId;
    if (m_currentViewMode == FormViewMode) {
        fieldId = m_formView->getSelectedField();
    } else {
        if (m_tableView->selectionModel()->hasSelection())
            fieldId = m_tableView->currentIndex().column();
        else
            fieldId = -1;
    }

    if (fieldId == -1) { //if no selection
        return;
    }

    if (!m_addFieldDialog) {
        m_addFieldDialog = new AddFieldDialog(this);
        m_addFieldDialog->setWindowModality(Qt::WindowModal);
    }

    m_addFieldDialog->setCreationMode(AbstractFieldWizard::ModifyEditMode,
                                      fieldId,
                                      m_metadataEngine->getCurrentCollectionId());
    m_addFieldDialog->show();
}

void MainWindow::searchSlot(const QString &s)
{
    if (!m_metadataEngine->getCurrentCollectionId()) return;

    QString key(s);

    //set to table view mode
    tableViewModeTriggered();

    //if searching for double (numeric) values
    //replace ',' with '.', since in db
    //decimal point is always '.'
    //and not ',' as in some languages
    QLocale locale;
    if (locale.decimalPoint() == ',')
        key.replace(',', '.');

    //remove the "'" char because model gets messed up
    //even if removed from search, model is not able
    //to populate again (SQL injection risk?)
    key.remove(QRegExp("'"));

    //adapt if new collection types are added,
    //for now assuming only standard collection
    StandardModel *sModel = qobject_cast<StandardModel*>(m_currentModel);
    if (sModel) {
        int count = m_metadataEngine->getFieldCount();

        //generate filter (where clause)
        QString filter;
        if (count > 0) {
            filter.append(QString("\"1\" LIKE '%%2%'").arg(key));
        }
        for (int i = 2; i < count; i++) { //start with 2 cause 0 is _id and 1 done
            switch(m_metadataEngine->getFieldType(i)) {
            case MetadataEngine::CheckboxType:
            case MetadataEngine::ComboboxType:
            case MetadataEngine::ProgressType:
            case MetadataEngine::ImageType:
            case MetadataEngine::FilesType:
            case MetadataEngine::DateType:
            case MetadataEngine::CreationDateType:
            case MetadataEngine::ModDateType:
                //exclude field type from search results
                break;
            default:
                filter.append(QString(" OR \"%1\" LIKE '%%2%'")
                              .arg(i).arg(key));
                break;
            }
        }
        if (!s.isEmpty())
            sModel->setFilter(filter);
        else
            sModel->setFilter(""); //clear filter
    }

    //select first result, if no result disable form view
    QModelIndex index = m_currentModel->index(0, 1);
    m_formView->selectionModel()->setCurrentIndex(
                index,
                QItemSelectionModel::SelectCurrent);
    m_formView->setEnabled(index.isValid());

    //clear undo stack cause row ids are not longer valid
    m_undoStack->clear();
}

void MainWindow::selectAllActionTriggered()
{
    if (!m_currentModel) return;

    //set form view
    tableViewModeTriggered();

    //select all
    QModelIndex p;
    QModelIndex topLeft = m_currentModel->index(0, 1, p);
    QModelIndex bottomRight = m_currentModel->index(
                m_currentModel->rowCount(p)-1,
                m_currentModel->columnCount(p)-1,
                p);
    QItemSelection selection(topLeft, bottomRight);
    m_formView->selectionModel()->select(selection, QItemSelectionModel::Select);
}

void MainWindow::backupActionTriggered()
{
    //detach views
    detachModelFromViews();
    detachCollectionModelView();
    int id = m_metadataEngine->getCurrentCollectionId();

    BackupDialog dialog(this);
    dialog.exec();

    //if backup restored, restart
    if (dialog.backupRestored()) {
        //if sync enabled close session
        if (SyncSession::IS_ENABLED) {
            if (SyncSession::IS_ONLINE && (!SyncSession::IS_READ_ONLY)) {
                QProgressDialog pd(this);
                pd.setWindowModality(Qt::WindowModal);
                pd.setWindowTitle(tr("Closing Session"));
                pd.setLabelText(tr("Closing sync session... Please wait!"));
                pd.setRange(0, 0);
                pd.setValue(-1);

                connect(m_syncEngine, SIGNAL(syncSessionClosed()),
                        &pd, SLOT(close()));
                m_syncEngine->startCloseCloudSession();

                //wait until session is closed
                pd.exec();

                //disable sync
                m_settingsManager->setCloudSyncActive(false);

                //cause conflict
                m_settingsManager->saveCloudLocalDataChanged(true);
                m_settingsManager->saveCloudSessionKey("invalid");
            }
        }
        QMessageBox box(QMessageBox::Information, tr("Software Restart"),
                        tr("Software restart required! "
                           "Please restart %1 manually.").arg(DefinitionHolder::NAME),
                        QMessageBox::NoButton,
                        this);
        box.setWindowModality(Qt::WindowModal);
        box.exec();
        qApp->quit();
    } else {
        //attach views
        attachCollectionModelView();
        m_metadataEngine->setCurrentCollectionId(id);
    }
}

void MainWindow::printActionTriggered()
{
    if ((!m_currentModel) || (!m_currentModel->rowCount())) {
        QMessageBox box(QMessageBox::Information, tr("Printing aborted"),
                        tr("There are no records to print!"),
                        QMessageBox::NoButton,
                        this);
        box.setWindowModality(Qt::WindowModal);
        box.exec();
        return;
    }

    QList<int> recordIdList; //ids of records to print

    if (m_currentViewMode == FormViewMode) {
        //get current record id
        int currentRow = m_formView->getCurrentRow();
        QModelIndex index = m_currentModel->index(currentRow, 0);
        if (index.isValid()) {
            bool ok;
            int recordId = index.data().toInt(&ok);
            if (ok)
                recordIdList.append(recordId);
        }

    } else { //table view
        QModelIndexList indexes = m_tableView->selectionModel()->selectedIndexes();
        QSet<int> rows;
        int indexesSize = indexes.size();
        for (int i = 0; i < indexesSize; i++) {
            rows.insert(indexes.at(i).row());
        }

        QList<int> rowList = rows.toList();
        int size = rowList.size();
        QModelIndex index;
        //extract record ids
        for (int i = 0; i < size; i++) {
            index = m_currentModel->index(rowList.at(i), 0);
            if (index.isValid()) {
                bool ok;
                int recordId = index.data().toInt(&ok);
                if (ok)
                    recordIdList.append(recordId);
            }
        }
    }

    //print dialog
    PrintDialog d(m_metadataEngine->getCurrentCollectionId(),
                  recordIdList,
                  this);
    d.exec();

    //set focus back (workaround)
    this->activateWindow();
}

void MainWindow::exportActionTriggered()
{
    if ((!m_currentModel) || (!m_currentModel->rowCount())) {
        QMessageBox box(QMessageBox::Information, tr("Export aborted"),
                        tr("There are no records to export!"),
                        QMessageBox::NoButton,
                        this);
        box.setWindowModality(Qt::WindowModal);
        box.exec();
        return;
    }

    QList<int> recordIdList; //ids of records to export

    if (m_currentViewMode == FormViewMode) {
        //get current record id
        int currentRow = m_formView->getCurrentRow();
        QModelIndex index = m_currentModel->index(currentRow, 0);
        if (index.isValid()) {
            bool ok;
            int recordId = index.data().toInt(&ok);
            if (ok)
                recordIdList.append(recordId);
        }

    } else { //table view
        QModelIndexList indexes = m_tableView->selectionModel()->selectedIndexes();
        QSet<int> rows;
        int indexesSize = indexes.size();
        for (int i = 0; i < indexesSize; i++) {
            rows.insert(indexes.at(i).row());
        }

        QList<int> rowList = rows.toList();
        int size = rowList.size();
        QModelIndex index;
        //extract record ids
        for (int i = 0; i < size; i++) {
            index = m_currentModel->index(rowList.at(i), 0);
            if (index.isValid()) {
                bool ok;
                int recordId = index.data().toInt(&ok);
                if (ok)
                    recordIdList.append(recordId);
            }
        }
    }

    //export dialog
    ExportDialog d(m_metadataEngine->getCurrentCollectionId(),
                  recordIdList,
                  this);
    d.exec();

    //set focus back (workaround)
    this->activateWindow();
}

void MainWindow::importActionTriggered()
{
    if (SyncSession::IS_READ_ONLY) {
        QMessageBox box(QMessageBox::Information, tr("Import not available"),
                        tr("Import is disabled due read-only session!"),
                        QMessageBox::NoButton,
                        this);
        box.setWindowModality(Qt::WindowModal);
        box.exec();
        return;
    }

    //export dialog
    ImportDialog d(this);
    d.exec();

    //refresh collection list view
    m_dockWidget->getCollectionListView()->detachModel();
    m_dockWidget->getCollectionListView()->attachModel();

    //set focus back (workaround)
    this->activateWindow();
}

void MainWindow::lockFormViewActionToggled(const bool locked)
{
    m_formView->setLockFormLayout(locked);
}

void MainWindow::showAlarmListDialog()
{
    if (!m_alarmListDialog) {
        m_alarmListDialog = new AlarmListDialog(this);
        connect(m_alarmListDialog, SIGNAL(showRecord(int,int,int)),
                this, SLOT(showRecordfromAlarm(int,int,int)));
    } else {
        //check again
        m_alarmListDialog->reloadAlarmList();
    }
    m_alarmListDialog->show();
    m_alarmListDialog->activateWindow();
}

void MainWindow::showRecordfromAlarm(int collectionId,
                                     int fieldId,
                                     int recordId)
{
    if (!m_currentModel) return;

    //set collection
    if (collectionId != m_metadataEngine->getCurrentCollectionId()) {
        m_metadataEngine->setCurrentCollectionId(collectionId);
    }

    //get row
    int row = -1;
    Qt::MatchFlags flags = Qt::MatchFlags( Qt::MatchExactly | Qt::MatchWrap );
    QModelIndexList indexList = m_currentModel->match(m_currentModel->index(0,0), Qt::DisplayRole,
                                                      recordId, flags);
    if (!indexList.isEmpty()) {
        row = indexList.at(0).row();
    }

    //fetch all data from model
    while(m_currentModel->canFetchMore(QModelIndex()))
        m_currentModel->fetchMore(QModelIndex());

    if (row != -1) {
        m_formView->selectionModel()->setCurrentIndex(
                    m_currentModel->index(row, fieldId),
                    QItemSelectionModel::SelectCurrent);
    }

    this->activateWindow();
}

void MainWindow::syncActionTriggered()
{
    bool syncConfigured = m_settingsManager->isCloudSyncActive();

    if (!syncConfigured) {
        //configure sync
        SyncConfigDialog dialog(this);
        dialog.exec();

        //if dialog cancelled
        if (!dialog.result())
            return;

        //enable sync status widget
        m_dockWidget->updateSyncStatusWidgetVisibility();
    }

    showSyncDialog();
}

void MainWindow::syncErrorSlot(const QString &message)
{
    QMessageBox box(QMessageBox::Critical, tr("Sync Error"),
                    tr("Cloud sync error: ").append(message),
                    QMessageBox::NoButton,
                    this);

    //if no other dialog is open, show modal
    if (isActiveWindow())
        box.setWindowModality(Qt::WindowModal);

    box.exec();
}

void MainWindow::syncClientAlreadyLoggedInSlot()
{
    QMessageBox box(QMessageBox::Warning, tr("Sync Session"),
                    tr("Sync session is already open. "
                       "This happens when another client is running in online "
                       "mode. Please continue in read-only mode until the "
                       "first client exits. It is also possible to force "
                       " write access by taking ownership of the session. "
                       "This is useful for cases where "
                       "the connection was accidentaly interrupted, "
                       "leaving the session open."
                       "<br><br><b>Warning:</b> Forcing access could "
                       "lead to data loss!"),
                    QMessageBox::Cancel,
                    this);

    //if no other dialog is open, show modal
    if (isActiveWindow())
        box.setWindowModality(Qt::WindowModal);

    box.addButton(tr("force access"), QMessageBox::NoRole);
    QPushButton *readOnly = box.addButton(tr("read-only access"),
                                          QMessageBox::YesRole);
    box.setDefaultButton(readOnly);
    int r = box.exec();
    bool syncCheckAgain = false;

    if (r == QMessageBox::Cancel) {
        //offline mode
        SyncSession::IS_ONLINE = false;
        SyncSession::IS_READ_ONLY = false;
    } else if (r == 0) {
        //force access
        SyncSession::IS_ONLINE = true;
        SyncSession::IS_READ_ONLY = false;
        syncCheckAgain = true;
    } else {
        //read-only is already set
    }

    SyncSession::CURRENT_STATE = SyncSession::NoOperation;
    m_dockWidget->updateSyncStatusWidget();

    if (syncCheckAgain)
        m_syncEngine->startSyncCheck();
}

void MainWindow::syncSessionKeyChangedSlot()
{
    showSyncDialog();
}

void MainWindow::syncNewRevisionAvailableSlot()
{
    showSyncDialog();
}

void MainWindow::syncRevisionConflictSlot()
{
    showSyncDialog();
}

void MainWindow::syncAuthTokenExpiredSlot()
{
    SyncConfigDialog dialog(this);
    dialog.reauthenticateSyncService();

    //if another dialog is open, show non modal
    if (!isActiveWindow())
        dialog.setWindowModality(Qt::NonModal);

    dialog.exec();

    //if dialog cancelled
    if (!dialog.result())
        return;

    m_syncEngine->startSyncCheck();
}

void MainWindow::syncStatusChanged()
{
    m_syncReadOnlyAction->setChecked(SyncSession::IS_READ_ONLY);
}

void MainWindow::checkForUpdatesSlot()
{
    //check if current version has been just updated and inform user if so
    if (m_settingsManager->restoreSoftwareBuild() < DefinitionHolder::SOFTWARE_BUILD) {
        UpgradeSuccessDialog *ud = new UpgradeSuccessDialog(this);
        ud->exec();
    }

    //ask user on auto update checking on 2nd start
    if ((!m_settingsManager->isFirstTimeStart()) && (!DefinitionHolder::APP_STORE)) {
        if (!m_settingsManager->restoreUserConfirmedAutoUpdateChecks()) {
            int r = QMessageBox::question(this, tr("Software Updates"),
                                          tr("Should %1 <b>check automatically</b> for software updates?<br />"
                                             "This option can be changes later at any time in the software preferences.<br /><br />"
                                             "The information about the latest release is downloaded from GitHub, "
                                             "their <a href=\"https://help.github.com/en/articles/github-privacy-statement\">"
                                             "privacy policy</a> may apply.").arg(DefinitionHolder::NAME),
                                          QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
            if ((r == QMessageBox::Yes) || (r == QMessageBox::No)) {
                m_settingsManager->saveUserConfirmedAutoUpdateChecks(true);
                m_settingsManager->saveCheckUpdates(r == QMessageBox::Yes);
            }
        }
    }

    if (!(DefinitionHolder::APP_STORE || (!m_settingsManager->restoreCheckUpdates()))) {
        if (!m_updateManager) {
            m_updateManager = new UpdateManager(this);
            connect(m_updateManager, SIGNAL(noUpdateSignal()),
                    this, SLOT(noUpdateFoundSlot()));
            connect(m_updateManager, SIGNAL(updateErrorSignal()),
                    this, SLOT(updateErrorSlot()));
            connect(m_updateManager, SIGNAL(updatesAccepted()),
                    this, SLOT(close()));
        }

        //start async update check
        statusBar()->showMessage(tr("Checking for updates..."));
        m_updateManager->checkForUpdates();
    }
}

void MainWindow::noUpdateFoundSlot()
{
    statusBar()->showMessage(tr("Your software version is up to date"));
}

void MainWindow::updateErrorSlot()
{
    statusBar()->showMessage(tr("Error while checking for software updates"));
}

void MainWindow::reattachModelToViewsSlot()
{
    //reattach model with current collection id
    reattachModelToViews(m_metadataEngine->getCurrentCollectionId());
}


//-----------------------------------------------------------------------------
// Private
//-----------------------------------------------------------------------------

void MainWindow::createActions()
{
    m_quitAction = new QAction(tr("&Quit"), this);
    m_quitAction->setStatusTip(tr("Exit from this application"));
    m_quitAction->setShortcut(QKeySequence::Quit);
    m_quitAction->setMenuRole(QAction::QuitRole);

    m_aboutAction = new QAction(tr("About %1").arg(DefinitionHolder::NAME), this);
    m_aboutAction->setMenuRole(QAction::AboutRole);

    m_aboutQtAction = new QAction(tr("About Qt"), this);
    m_aboutQtAction->setMenuRole(QAction::AboutQtRole);

    m_onlineDocAction = new QAction(tr("Online documentation"), this);
    m_onlineDocAction->setStatusTip(tr("View the project wiki on GitHub"));

    m_donateAction = new QAction(tr("Donate!"), this);
    m_donateAction->setStatusTip(tr("Say thanks by donating any amount"));

    m_newCollectionAction = new QAction(tr("New Collection..."), this);
    m_newCollectionAction->setIcon(QIcon(":/images/icons/newcollection.png"));
    m_newCollectionAction->setStatusTip(tr("Create a new collection"));

    m_duplicateCollectionAction = new QAction(tr("Duplicate Collection"), this);
    m_duplicateCollectionAction->setIcon(QIcon(":/images/icons/duplicatecollection.png"));
    m_duplicateCollectionAction->setStatusTip(tr("Duplicate current collection"));

    m_deleteCollectionAction = new QAction(tr("Delete Collection"), this);
    m_deleteCollectionAction->setIcon(QIcon(":/images/icons/deletecollection.png"));
    m_deleteCollectionAction->setStatusTip(tr("Delete current collection"));

    m_newRecordAction = new QAction(tr("New Record"), this);
    m_newRecordAction->setShortcut(QKeySequence::New);
    m_newRecordAction->setIcon(QIcon(":/images/icons/newrecord.png"));

    m_newFieldAction = new QAction(tr("New Field..."), this);
    m_newFieldAction->setIcon(QIcon(":/images/icons/newfield.png"));

    m_backupAction = new QAction(tr("Backup..."), this);
    m_backupAction->setStatusTip(tr("Backup or restore a database file"));

    m_settingsAction = new QAction(tr("Preferences"), this);
    m_settingsAction->setMenuRole(QAction::PreferencesRole);
    m_settingsAction->setShortcut(QKeySequence::Preferences);
    m_settingsAction->setStatusTip(tr("Change application settings"));

    m_undoAction = m_undoStack->createUndoAction(this, tr("Undo"));
    m_undoAction->setShortcut(QKeySequence::Undo);

    m_redoAction = m_undoStack->createRedoAction(this, tr("Redo"));
    m_redoAction->setShortcut(QKeySequence::Redo);

    m_selectAllAction = new QAction(tr("Select all records"), this);

    m_findAction = new QAction(tr("Find"), this);
    m_findAction->setShortcut(QKeySequence::Find);

    m_formViewModeAction = new QAction(tr("Form View"), this);
    m_formViewModeAction->setShortcut(QString("CTRL+G"));
    m_formViewModeAction->setStatusTip(tr("Change current view mode to a form-like view"));
    m_formViewModeAction->setCheckable(true);
    m_formViewModeAction->setChecked(true);

    m_tableViewModeAction = new QAction(tr("Table View"), this);
    m_tableViewModeAction->setShortcut(QString("CTRL+T"));
    m_tableViewModeAction->setStatusTip(tr("Change current view mode to a table-like view"));
    m_tableViewModeAction->setCheckable(true);

    m_viewModeActionSeparator = new QAction(tr("View Mode"), this);
    m_viewModeActionSeparator->setSeparator(true);

    m_viewModeActionGroup = new QActionGroup(this);
    m_viewModeActionGroup->addAction(m_formViewModeAction);
    m_viewModeActionGroup->addAction(m_tableViewModeAction);

#ifdef Q_OS_OSX
    m_minimizeAction = new QAction(tr("Minimize"), this);
    m_minimizeAction->setShortcut(QString("CTRL+M"));

    m_closeWindowAction = new QAction(tr("Close"), this);
    m_closeWindowAction->setShortcut(QString("CTRL+W"));
#endif //Q_OS_OSX

    m_fullscreenAction = new QAction(tr("Fullscreen"), this);
#ifndef Q_OS_OSX
    m_fullscreenAction->setShortcut(QString("F11"));
#endif

    m_toggleDockAction = new QAction(tr("Hide collection sidebar"), this);
    m_toggleDockAction->setCheckable(true);
    m_toggleDockAction->setShortcut(QString("CTRL+B"));

    m_deleteAllRecordsAction = new QAction(tr("Delete all records"), this);
    m_deleteAllRecordsAction->setStatusTip(
                tr("Remove all records from current collection"));

    m_optimizeDbSizeAction = new QAction(tr("Free unused space"), this);
    m_optimizeDbSizeAction->setStatusTip(tr("Optimize size of database file "
                                            "by freeing unused resources"));

    m_syncAction = new QAction(tr("Cloud synchronization..."), this);
    m_syncAction->setStatusTip(tr("Synchronize your database with a cloud service"));
    m_syncAction->setShortcut(QString("CTRL+S"));
    m_syncAction->setIcon(QIcon(":/images/icons/sync.png"));

    m_syncReadOnlyAction = new QAction(tr("Read-only mode"), this);
    m_syncReadOnlyAction->setStatusTip(tr("Toggle read-only mode. If enabled, "
                                          "other clients can access and write to the database."));
    m_syncReadOnlyAction->setShortcut(QString("CTRL+R"));
    m_syncReadOnlyAction->setCheckable(true);

    m_checkUpdatesAction = new QAction(tr("Check for updates"), this);
    m_checkUpdatesAction->setStatusTip(tr("Check for %1 updates")
                                     .arg(DefinitionHolder::NAME));
    m_checkUpdatesAction->setMenuRole(QAction::ApplicationSpecificRole);

    m_showAlarmDialogAction = new QAction(tr("Date reminder list..."), this);

    m_printAction = new QAction(tr("Print..."), this);
    m_printAction->setStatusTip(tr("Print records or export them as PDF"));
    m_printAction->setShortcut(QKeySequence::Print);

    m_importAction = new QAction(tr("Import..."), this);
    m_importAction->setStatusTip(tr("Import existing records to the database"));

    m_exportAction = new QAction(tr("Export..."), this);
    m_exportAction->setStatusTip(tr("Export all or only selected records"));

    m_lockFormViewAction = new QAction(tr("Lock form view"), this);
    m_lockFormViewAction->setCheckable(true);
    m_lockFormViewAction->setShortcut(QString("CTRL+L"));
    m_lockFormViewAction->setIcon(QIcon(":/images/icons/locked.png"));
    m_lockFormViewAction->setStatusTip(tr("Lock the form view design to prevent "
                                          "unwanted field movements"));
}

void MainWindow::createToolBar()
{
    m_toolBar = new QToolBar(tr("Toolbar"), this);

    //set object name to allow saveState()
    m_toolBar->setObjectName("mainToolBar");

    m_toolBar->setIconSize(QSize(32, 32));
    m_toolBar->setMovable(false);

    m_toolBar->addAction(m_newCollectionAction);
    m_toolBar->addAction(m_duplicateCollectionAction);
    m_toolBar->addAction(m_deleteCollectionAction);
    m_toolBar->addSeparator();
    m_toolBar->addAction(m_lockFormViewAction);
    m_toolBar->addSeparator();
    m_toolBar->addAction(m_syncAction);

//#ifndef Q_OS_OSX
//    m_toolBar->setStyleSheet(
//                "QToolBar { background-color: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 rgb(246, 248, 249), stop:0.5 rgb(229, 235, 238),stop:0.51 rgb(215, 222, 227),stop:1 rgb(245, 247,249)); }"
//                "QToolButton {"
//                "border: none;"
//                "background-color: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 rgb(246, 248, 249), stop:0.5 rgb(229, 235, 238),stop:0.51 rgb(215, 222, 227),stop:1 rgb(245, 247,249));"
//                "}"
//                "QToolButton:pressed {"
//                "border-radius: 6px;"
//                "background-color: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #d2d4d5, stop:0.5 #a8a9a9,stop:0.51 #98999a,stop:1 #d2d4d5);"
//                "}");
//#endif //Q_OS_OSX

    this->addToolBar(m_toolBar);
}

void MainWindow::createDockWidget()
{
    m_dockWidget = new DockWidget(this);
    m_dockContainerWidget = new QDockWidget(tr("COLLECTIONS"), this);

    //set object name to allow saveState()
    m_dockContainerWidget->setObjectName("collectionDock");

    m_dockContainerWidget->setFeatures(QDockWidget::NoDockWidgetFeatures);
    m_dockContainerWidget->setWidget(m_dockWidget);

    m_dockContainerWidget->setStyleSheet("QDockWidget::title {"
                                         "padding-top: 0px;"
                                         "padding-bottom: 0px;"
                                         "margin-top: 0px;"
                                         "margin-bottom: 0px;"
                                         "text-align: left;"
                                         "padding-left: 8px;"
                                         "background-color: transparent;"
                                         "}"
                                         "QDockWidget {"
                                         "font-size: 11px;"
                                         "font-weight: bold;"
                                         "color: #6F7E8B;"
                                         "}");

    addDockWidget(Qt::LeftDockWidgetArea, m_dockContainerWidget);
}

void MainWindow::createMenu()
{
    m_newMenu = new QMenu(tr("New"), this);
    m_newMenu->addAction(m_newCollectionAction);
    m_newMenu->addAction(m_newRecordAction);
    m_newMenu->addAction(m_newFieldAction);

    m_fileMenu = menuBar()->addMenu(tr("&File"));
    m_fileMenu->addMenu(m_newMenu);
    m_fileMenu->addSeparator();
    m_fileMenu->addAction(m_backupAction);
    m_fileMenu->addSeparator();
    m_fileMenu->addAction(m_printAction);
    m_fileMenu->addSeparator();
    m_fileMenu->addAction(m_importAction);
    m_fileMenu->addAction(m_exportAction);
    m_fileMenu->addSeparator();
    m_fileMenu->addAction(m_quitAction);

    m_editMenu = menuBar()->addMenu(tr("&Edit"));
    m_editMenu->addAction(m_undoAction);
    m_editMenu->addAction(m_redoAction);
    m_editMenu->addSeparator();
    m_editMenu->addAction(m_selectAllAction);
    m_editMenu->addSeparator();
    m_editMenu->addAction(m_lockFormViewAction);
    m_editMenu->addSeparator();
    m_editMenu->addAction(m_findAction);

    m_viewMenu = menuBar()->addMenu(tr("&View"));
    m_viewMenu->addAction(m_fullscreenAction);
    m_viewMenu->addAction(m_toggleDockAction);
    m_viewMenu->addAction(m_viewModeActionSeparator);
    m_viewMenu->addAction(m_formViewModeAction);
    m_viewMenu->addAction(m_tableViewModeAction);
    m_viewMenu->addSeparator();
    m_viewMenu->addAction(m_showAlarmDialogAction);

    m_cloudMenu = new QMenu(tr("Cloud"), this);
    m_cloudMenu->addAction(m_syncReadOnlyAction);

    m_recordsMenu = new QMenu(tr("Records"), this);
    m_recordsMenu->addAction(m_deleteAllRecordsAction);

    m_databaseMenu = new QMenu(tr("Database"), this);
    m_databaseMenu->addAction(m_optimizeDbSizeAction);

    m_toolsMenu = menuBar()->addMenu(tr("&Tools"));
    m_toolsMenu->addAction(m_syncAction);
    m_toolsMenu->addSeparator();
    m_toolsMenu->addMenu(m_cloudMenu);
    m_toolsMenu->addMenu(m_recordsMenu);
    m_toolsMenu->addMenu(m_databaseMenu);
    m_toolsMenu->addSeparator();
    m_toolsMenu->addAction(m_settingsAction);

#ifdef Q_OS_OSX
    m_windowMenu = menuBar()->addMenu(tr("&Window"));
    m_windowMenu->addAction(m_minimizeAction);
    m_windowMenu->addAction(m_closeWindowAction);
#endif //Q_OS_OSX

    m_helpMenu = menuBar()->addMenu(tr("&Help"));
    m_helpMenu->addAction(m_aboutAction);
    m_helpMenu->addAction(m_aboutQtAction);
    m_helpMenu->addSeparator();
    m_helpMenu->addAction(m_onlineDocAction);
    m_helpMenu->addAction(m_donateAction);
    m_helpMenu->addSeparator();
    if (!DefinitionHolder::APP_STORE)
        m_helpMenu->addAction(m_checkUpdatesAction);
}

void MainWindow::createStatusBar()
{
    m_statusBar = statusBar();
    m_statusBar->showMessage(tr(" Ready "));
}

void MainWindow::createComponents()
{
    m_settingsManager = new SettingsManager;
    m_metadataEngine = &MetadataEngine::getInstance();
    m_syncEngine = &SyncEngine::getInstance();
    m_undoStack = new QUndoStack(this);
}

void MainWindow::createCentralWidget()
{
    //create widgets
    m_viewToolBar = new ViewToolBarWidget(this);
    m_formView = new FormView(this);
    m_tableView = new TableView(this);
    m_centralStackedWidget = new QStackedWidget(this);
    m_viewStackedWidget = new QStackedWidget(this);

    //setup ViewStackedWidget
    m_viewStackedWidget->addWidget(m_formView);
    m_viewStackedWidget->addWidget(m_tableView);

    //build ViewWidget
    QWidget *viewWidget = new QWidget(this);
    QVBoxLayout *viewMainLayout = new QVBoxLayout(viewWidget);
    viewWidget->setLayout(viewMainLayout);
    viewMainLayout->addWidget(m_viewToolBar);
    viewMainLayout->addWidget(m_viewStackedWidget);
    viewMainLayout->setSpacing(0);
    viewMainLayout->setContentsMargins(0,0,0,0);

    //setup central widget
    m_centralStackedWidget->addWidget(viewWidget);
    setCentralWidget(m_centralStackedWidget);

    m_formView->setFocus();
}

void MainWindow::createConnections()
{
    //main window
    connect(m_quitAction, SIGNAL(triggered()),
            this, SLOT(close()));
    connect(m_aboutAction, SIGNAL(triggered()),
            this, SLOT(aboutActionTriggered()));
    connect(m_aboutQtAction, SIGNAL(triggered()),
            this, SLOT(aboutQtActionTriggered()));
    connect(m_onlineDocAction, &QAction::triggered,
            this, &MainWindow::onlineDocActionTriggered);
    connect(m_donateAction, &QAction::triggered,
            this, &MainWindow::donateActionTriggered);
    connect(m_settingsAction, SIGNAL(triggered()),
            this, SLOT(preferenceActionTriggered()));
    connect(m_findAction, SIGNAL(triggered()),
            m_viewToolBar, SLOT(setSearchLineFocus()));
    connect(m_formViewModeAction, SIGNAL(triggered()),
            this, SLOT(formViewModeTriggered()));
    connect(m_tableViewModeAction, SIGNAL(triggered()),
            this, SLOT(tableViewModeTriggered()));
    connect(m_fullscreenAction, SIGNAL(triggered()),
            this, SLOT(fullscreenActionTriggered()));
    connect(m_toggleDockAction, SIGNAL(triggered()),
            this, SLOT(toggleDockActionTriggered()));
    connect(m_syncAction, SIGNAL(triggered()),
            this, SLOT(syncActionTriggered()));
    connect(m_selectAllAction, SIGNAL(triggered()),
            this, SLOT(selectAllActionTriggered()));
    connect(m_backupAction, SIGNAL(triggered()),
            this, SLOT(backupActionTriggered()));
    connect(m_checkUpdatesAction, SIGNAL(triggered()),
            this, SLOT(checkForUpdatesSlot()));
    connect(m_showAlarmDialogAction, SIGNAL(triggered()),
            this, SLOT(showAlarmListDialog()));
    connect(m_printAction, SIGNAL(triggered()),
            this, SLOT(printActionTriggered()));
    connect(m_exportAction, SIGNAL(triggered()),
            this, SLOT(exportActionTriggered()));
    connect(m_importAction, SIGNAL(triggered()),
            this, SLOT(importActionTriggered()));
    connect(m_lockFormViewAction, &QAction::toggled,
            this, &MainWindow::lockFormViewActionToggled);

    //record actions
    connect(m_newRecordAction, SIGNAL(triggered()),
            this, SLOT(newRecordActionTriggered()));
    connect(m_viewToolBar, SIGNAL(newRecordSignal()),
            this, SLOT(newRecordActionTriggered()));
    connect(m_viewToolBar, SIGNAL(duplicateRecordSignal()),
            this, SLOT(duplicateRecordActionTriggered()));
    connect(m_viewToolBar, SIGNAL(deleteRecordSignal()),
            this, SLOT(deleteRecordActionTriggered()));

    //field actions
    connect(m_newFieldAction, SIGNAL(triggered()),
            this, SLOT(newFieldActionTriggered()));
    connect(m_viewToolBar, SIGNAL(newFieldSignal()),
            this, SLOT(newFieldActionTriggered()));
    connect(m_viewToolBar, SIGNAL(duplicateFieldSignal()),
            this, SLOT(duplicateFieldActionTriggered()));
    connect(m_viewToolBar, SIGNAL(deleteFieldSignal()),
            this, SLOT(deleteFieldActionTriggered()));

    //form view context menu actions
    connect(m_formView, SIGNAL(newFieldSignal()),
           this, SLOT(newFieldActionTriggered()));
    connect(m_formView, SIGNAL(duplicateFieldSignal()),
           this, SLOT(duplicateFieldActionTriggered()));
    connect(m_formView, SIGNAL(deleteFieldSignal()),
           this, SLOT(deleteFieldActionTriggered()));
    connect(m_formView, SIGNAL(modifyFieldSignal()),
           this, SLOT(modifyFieldActionTriggered()));
    connect(m_formView, SIGNAL(newRecordSignal()),
           this, SLOT(newRecordActionTriggered()));
    connect(m_formView, SIGNAL(duplicateRecordSignal()),
           this, SLOT(duplicateRecordActionTriggered()));
    connect(m_formView, SIGNAL(deleteRecordSignal()),
           this, SLOT(deleteRecordActionTriggered()));

    //table view context menu actions
    connect(m_tableView, SIGNAL(newFieldSignal()),
           this, SLOT(newFieldActionTriggered()));
    connect(m_tableView, SIGNAL(duplicateFieldSignal()),
           this, SLOT(duplicateFieldActionTriggered()));
    connect(m_tableView, SIGNAL(deleteFieldSignal()),
           this, SLOT(deleteFieldActionTriggered()));
    connect(m_tableView, SIGNAL(modifyFieldSignal()),
           this, SLOT(modifyFieldActionTriggered()));
    connect(m_tableView, SIGNAL(newRecordSignal()),
           this, SLOT(newRecordActionTriggered()));
    connect(m_tableView, SIGNAL(duplicateRecordSignal()),
           this, SLOT(duplicateRecordActionTriggered()));
    connect(m_tableView, SIGNAL(deleteRecordSignal()),
           this, SLOT(deleteRecordActionTriggered()));

    //collection actions
    connect(m_newCollectionAction, SIGNAL(triggered()),
            this, SLOT(newCollectionActionTriggered()));
    connect(m_duplicateCollectionAction, SIGNAL(triggered()),
            this, SLOT(duplicateCollectionActionTriggered()));
    connect(m_deleteCollectionAction, SIGNAL(triggered()),
            this, SLOT(deleteCollectionActionTriggered()));

    //tools action
    connect(m_syncReadOnlyAction, SIGNAL(triggered()),
            this, SLOT(syncReadOnlyActionTriggered()));
    connect(m_deleteAllRecordsAction, SIGNAL(triggered()),
            this, SLOT(deleteAllRecordsActionTriggered()));
    connect(m_optimizeDbSizeAction, SIGNAL(triggered()),
            this, SLOT(optimizeDbSizeActionTriggered()));

#ifdef Q_OS_OSX
    //minimize action
    connect(m_minimizeAction, SIGNAL(triggered()),
            this, SLOT(showMinimized()));
    connect(m_closeWindowAction, SIGNAL(triggered()),
            this, SLOT(close()));
#endif //Q_OS_OSX

    //view toolbar
    connect(m_viewToolBar, SIGNAL(formViewModeSignal()),
            this, SLOT(formViewModeTriggered()));
    connect(m_viewToolBar, SIGNAL(tableViewModeSignal()),
            this, SLOT(tableViewModeTriggered()));
    connect(m_viewToolBar, SIGNAL(nextRecordSignal()),
            m_formView, SLOT(navigateNextRecord()));
    connect(m_viewToolBar, SIGNAL(previousRecordSignal()),
            m_formView, SLOT(navigatePreviousRecord()));
    connect(m_viewToolBar, SIGNAL(searchSignal(QString)),
            this, SLOT(searchSlot(QString)));

    //collection changing
    connect(m_metadataEngine, SIGNAL(currentCollectionIdChanged(int)),
            this, SLOT(currentCollectionIdChanged(int)));
    connect(m_metadataEngine, SIGNAL(currentCollectionChanged()),
            this, SLOT(currentCollectionChanged()));

    //table view -> form view
    connect(m_tableView, SIGNAL(recordEditFinished(int,int)),
            m_formView, SLOT(updateLastModified(int,int)));
}

void MainWindow::restoreSettings()
{
    restoreGeometry(m_settingsManager->restoreGeometry("mainWindow"));
    restoreState(m_settingsManager->restoreState("mainWindow"));

    //restore view mode
    m_currentViewMode = static_cast<ViewMode>(m_settingsManager->restoreViewMode());
    if(m_currentViewMode == TableViewMode)
        tableViewModeTriggered();

    //restore last used record
    int last = m_settingsManager->restoreLastUsedRecord();
    if ((last != -1) && m_currentModel) { //if valid
        QModelIndex index = m_formView->model()->index(last, 1);
        if (index.isValid())
            m_tableView->setCurrentIndex(index);
    }

#ifdef Q_OS_OSX
    if (QSysInfo::MacintoshVersion >= QSysInfo::MV_10_7) {
        bool f = m_settingsManager->restoreProperty("macLionFullscreen", "mainWindow")
                .toBool();
        if (f) {
            //this is a workaround to activate fullsreen on mac without crashing on start
            QTimer::singleShot(100, this, SLOT(fullscreenActionTriggered()));
        }
    }
#endif //Q_OS_OSX

#ifdef Q_OS_LINUX
    if (m_settingsManager->restoreProperty("linuxDarkAmbianceToolbar", "mainWindow")
            .toBool()) {
        m_toolBar->setStyleSheet("QToolBar {background-color: qlineargradient(spread:pad,"
                                 " x1:0.5, y1:0.01, x2:0.5, y2:0.99, stop:0 rgba(65, 64, "
                                 "59, 255), stop:0.01 rgba(56, 55, 52, 255), stop:0.99 "
                                 "rgba(84, 82, 74, 255), stop:1 rgba(66, 65, 60, 255));} "
                                 "QToolBar:!active {background-color: rgb(60, 59, 55);}");
    }
#endif //Q_OS_LINUX

    //update dock widget status (hidden/visisble) and toggleDockAction
    //dock status is saved as part of geometry so only menu sync is needed
    if (m_dockContainerWidget->isHidden()){
        m_toggleDockAction->setChecked(true);
    }

    //restore layout locked status of formView
    bool fwLock = m_settingsManager->restoreProperty("lockFormView", "mainWindow").toBool();
    m_lockFormViewAction->setChecked(fwLock);
    m_formView->setLockFormLayout(fwLock);

    //sync
    SyncSession::LOCAL_DATA_CHANGED = m_syncEngine->localDataChanged();
    SyncSession::IS_ENABLED = m_settingsManager->isCloudSyncActive();
}

void MainWindow::saveSettings()
{
    m_settingsManager->saveGeometry("mainWindow", saveGeometry());
    m_settingsManager->saveState("mainWindow", saveState());
    m_settingsManager->saveSoftwareBuild();
    m_settingsManager->saveViewMode(m_currentViewMode);
    m_settingsManager->saveLastUsedRecord(m_formView->getCurrentRow());

    //save layout locked status of formView
    m_settingsManager->saveProperty("lockFormView", "mainWindow",
                                    m_lockFormViewAction->isChecked());

    //sync
    if (SyncSession::IS_ENABLED) {
        bool changed = SyncSession::LOCAL_DATA_CHANGED;
        if (!SyncSession::IS_READ_ONLY) {
            m_syncEngine->setLocalDataChanged(changed);
        }
    }
}

void MainWindow::init()
{
#ifdef Q_OS_OSX
    setUnifiedTitleAndToolBarOnMac(true);
#endif // Q_OS_OSX
}

void MainWindow::attachModelToViews(const int collectionId)
{
    if (!collectionId) { //0 stands for invalid
        m_formView->setModel(nullptr);
        m_tableView->setModel(nullptr);
        return;
    }

    //if there are other collection type supported
    //add the code here to find the correct collection type
    //for now only standard type is supported
    MetadataEngine::CollectionType type = MetadataEngine::StandardCollection;

    //create model
    m_currentModel = m_metadataEngine->createModel(type, collectionId);
    if (!m_currentModel) return;

    //fetch all data from model (avoid lazy loading at startup)
    while(m_currentModel->canFetchMore(QModelIndex()))
        m_currentModel->fetchMore(QModelIndex());

    //create connections
    StandardModel *sModel = qobject_cast<StandardModel*>(m_currentModel);
    connect(sModel, &StandardModel::rowsDeleted, //needed to detect undo of record deletions
            this, &MainWindow::reattachModelToViewsSlot);

    //set model on views
    m_formView->setModel(m_currentModel);
    m_tableView->setModel(m_currentModel);
    //share selection model between the two views
    QItemSelectionModel *old = m_tableView->selectionModel();
    m_tableView->setSelectionModel(m_formView->selectionModel());
    delete old;

    //if collection is empty
    //set form view as default
    //to show help messages
    if (!m_currentModel->rowCount())
        formViewModeTriggered();

    m_lastUsedCollectionId = collectionId;
}

void MainWindow::detachModelFromViews()
{
    //save form view if form widget has focus
    setFocus();

    //content data views
    m_formView->setModel(nullptr);
    m_tableView->setModel(nullptr);
    if (m_currentModel) {
        //connected signals are deleted too with the object
        delete m_currentModel;
        m_currentModel = nullptr;
    }
}

void MainWindow::reattachModelToViews(const int collectionId)
{
    //detach and attach models to views to reload the model and views (hard way)
    detachModelFromViews();
    attachModelToViews(collectionId);
}

void MainWindow::attachCollectionModelView()
{
    //collection list view
    CollectionListView *cv = m_dockWidget->getCollectionListView();
    QAbstractItemModel *cm = cv->model();
    if (!cm) {
        cv->attachModel();
    }
}

void MainWindow::detachCollectionModelView()
{
    //collection list view
    CollectionListView *cv = m_dockWidget->getCollectionListView();
    QAbstractItemModel *cm = cv->model();
    if (cm) {
        cv->detachModel();
    }
}

void MainWindow::initSync()
{
    SettingsManager s;
    if (!s.isCloudSyncActive()) return;

    createSyncConnections();
    m_syncEngine->startSyncCheck();
}

void MainWindow::showSyncDialog(bool autoclose)
{
    //detach views
    detachModelFromViews();
    detachCollectionModelView();

    //disable temporary sync connections
    //to allow exclusive sync handling
    //by the sync dialog
    removeSyncConnections();
    SyncProcessDialog syncDialog(this);
    if (autoclose) syncDialog.enableAutoCloseAfterSync();

    //if another dialog is open, show non modal
    if (!isActiveWindow())
        syncDialog.setWindowModality(Qt::NonModal);

    syncDialog.exec();
    createSyncConnections(); //enable sync connections again

    int id = m_metadataEngine->getCurrentCollectionId();

    //attach views
    attachCollectionModelView();
    m_metadataEngine->setCurrentCollectionId(id);

    //update status widget
    m_dockWidget->updateSyncStatusWidget();

    //clear undo stack since id may not be valid anymore
    m_undoStack->clear();
}

void MainWindow::createSyncConnections()
{
    //sync engine
    connect(m_syncEngine, SIGNAL(sessionChanged()),
            m_dockWidget, SLOT(updateSyncStatusWidget()));
    connect(m_syncEngine, SIGNAL(sessionChanged()),
            this, SLOT(syncStatusChanged()));
    connect(m_syncEngine, SIGNAL(syncError(QString)),
            this, SLOT(syncErrorSlot(QString)));
    connect(m_syncEngine, SIGNAL(clientAlreadyLoggedIn()),
            this, SLOT(syncClientAlreadyLoggedInSlot()));
    connect(m_syncEngine, SIGNAL(sessionKeyChanged()),
            this, SLOT(syncSessionKeyChangedSlot()));
    connect(m_syncEngine, SIGNAL(newSyncRevisionAvailable()),
            this, SLOT(syncNewRevisionAvailableSlot()));
    connect(m_syncEngine, SIGNAL(syncRevisionConflict()),
            this, SLOT(syncRevisionConflictSlot()));
    connect(m_syncEngine, SIGNAL(authTokenExpired()),
            this, SLOT(syncAuthTokenExpiredSlot()));
}

void MainWindow::removeSyncConnections()
{
    //sync engine
    disconnect(m_syncEngine, nullptr, m_dockWidget, nullptr);
    disconnect(m_syncEngine, nullptr, this, nullptr);
}

void MainWindow::checkAlarmTriggers()
{
    AlarmManager alarmManager(this);
    if (alarmManager.checkAlarms()) {
        //this is a workaround to activate focus on the dialog
        QTimer::singleShot(100, this, SLOT(showAlarmListDialog()));
    }
}

void MainWindow::checkDonationSuggestion()
{
    bool skipDonate = m_settingsManager->restoreProperty("skipDonate", "mainWindow").toBool();
    if (!skipDonate) {
        QDate lastUsageDate = m_settingsManager->restoreProperty("lastUsageDate", "mainWindow").toDate();
        int daysUsed = m_settingsManager->restoreProperty("donationDaysCount", "mainWindow").toInt();
        if (daysUsed >= 30) {
            int r = QMessageBox::question(this, tr("Support %1").arg(DefinitionHolder::NAME),
                                          tr("Dear user, you have been using %1 for a while. If you enjoy using this software, "
                                             "please consider supporting our development effort by making a small donation, thanks!<br />"
                                             "Would you like to donate now?").arg(DefinitionHolder::NAME),
                                          QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
            if (r == QMessageBox::Yes) {
                donateActionTriggered();
            }
            m_settingsManager->saveProperty("skipDonate", "mainWindow", true);
        }
        else if (lastUsageDate != QDate::currentDate()) {
            daysUsed++;
            m_settingsManager->saveProperty("donationDaysCount", "mainWindow", daysUsed);
            m_settingsManager->saveProperty("lastUsageDate", "mainWindow", QDate::currentDate());
        }
    }
}
