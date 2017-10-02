/*
 *  Copyright (c) 2012 Giorgio Wicklein <giowckln@gmail.com>
 */

//-----------------------------------------------------------------------------
// Hearders
//-----------------------------------------------------------------------------

#include "syncprocessdialog.h"
#include "ui_syncprocessdialog.h"
#include "../components/sync_framework/abstractsyncdriver.h"
#include "../components/settingsmanager.h"
#include "../components/databasemanager.h"
#include "../components/filemanager.h"
#include "../components/metadataengine.h"
#include "../utils/definitionholder.h"

#include <QtCore/QFile>


//-----------------------------------------------------------------------------
// Public
//-----------------------------------------------------------------------------

SyncProcessDialog::SyncProcessDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SyncProcessDialog),
    m_settingsManager(0),
    m_databaseSyncOp(DbInvalidOp),
    m_downloadFileOp(DownloadNoOp),
    m_uploadFileOp(UploadNoOp),
    m_fileSyncInitialized(false),
    m_syncCompleted(false),
    m_autoCloseAfterSync(false),
    m_metadataBeforeSync(0)
{
    ui->setupUi(this);

    init();
    initSync();
    createConnections();
    createSyncConnections();
    startSyncCheck();
}

SyncProcessDialog::~SyncProcessDialog()
{
    delete ui;

    if (m_settingsManager)
        delete m_settingsManager;

    if (m_metadataBeforeSync)
        delete m_metadataBeforeSync;
}

void SyncProcessDialog::enableAutoCloseAfterSync()
{
    m_autoCloseAfterSync = true;
}


//-----------------------------------------------------------------------------
// Public slots
//-----------------------------------------------------------------------------

void SyncProcessDialog::reject()
{
    //if sync is interrupted
    if (ui->stackedWidget->currentIndex() == 3) {
        syncCancelButtonClicked();
    }

    QDialog::reject();
}


//-----------------------------------------------------------------------------
// Private slots
//-----------------------------------------------------------------------------

void SyncProcessDialog::connectionFailedSlot()
{
    ui->stackedWidget->setCurrentIndex(3);
    ui->syncResultLabel->setText(tr("Connection to cloud service failed,"
                                    " check your connection."));
    ui->syncOperationLabel->hide();
    ui->syncProgressBar->hide();
    ui->syncCurrentTaskLabel->hide();
    ui->syncCurrentTaskProgressBar->hide();
}

void SyncProcessDialog::tokenInvalidOrExpiredSlot()
{
    ui->stackedWidget->setCurrentIndex(3);
    ui->syncResultLabel->setText(tr("The authentication token for your "
                                    "cloud service is invalid or expired."));
    ui->syncOperationLabel->hide();
    ui->syncProgressBar->hide();
    ui->syncCurrentTaskLabel->hide();
    ui->syncCurrentTaskProgressBar->hide();
}

void SyncProcessDialog::syncError(const QString &message)
{
    ui->stackedWidget->setCurrentIndex(3);
    ui->syncResultLabel->setTextInteractionFlags(Qt::LinksAccessibleByMouse);
    ui->syncResultLabel->setOpenExternalLinks(true);
    ui->syncResultLabel->setText(message);
    ui->syncOperationLabel->hide();
    ui->syncProgressBar->hide();
    ui->syncCurrentTaskLabel->hide();
    ui->syncCurrentTaskProgressBar->hide();
}

void SyncProcessDialog::metadataNotFoundSlot()
{
    ui->stackedWidget->setCurrentIndex(1);
    ui->initOperationLabel->setText(tr("Starting..."));

    //setup new sync config
    m_syncEngine->startCloudInitConfig();

    ui->initOperationLabel->setText(tr("Uploading..."));
}

void SyncProcessDialog::nothingToSyncSlot()
{
    ui->stackedWidget->setCurrentIndex(3);
    ui->syncResultLabel->setText(tr("Your database is already up to date. "
                                    "There are no changes to synchronize."));
    ui->syncOperationLabel->hide();
    ui->syncProgressBar->hide();
    ui->syncCurrentTaskLabel->hide();
    ui->syncCurrentTaskProgressBar->hide();
    ui->syncOkButton->setEnabled(true);
}

void SyncProcessDialog::clientAlreadyLoggedInSlot()
{
    syncError(tr("Another client already opened a sync session to the "
                 "cloud service. Please retry later "
                 "or restart %1.").arg(DefinitionHolder::NAME));
}

void SyncProcessDialog::cloudInitConfigCompletedSlot()
{
    //init conflict
    if (ui->stackedWidget->currentIndex() == 1) {
        m_settingsManager->setCloudSyncInitialized(true);

        ui->initOkButton->setEnabled(true);
        ui->initOkButton->setFocus();
        ui->initResultLabel->setText(tr("Cloud service successfully configured!"));
        ui->initProgressBar->hide();
        ui->initOperationLabel->hide();
    } else if (ui->stackedWidget->currentIndex() == 5) {
        //rev conflict
        ui->revConfOkButton->setEnabled(true);
        ui->revConfOkButton->setFocus();
        ui->revConfResultLabel->setText(tr("Revision conflict resolved!"));
        ui->revConfProgressBar->hide();
        ui->revConfOperationLabel->hide();
    }
}

void SyncProcessDialog::firstTimeSyncSlot()
{
    startSyncProcess();
}

void SyncProcessDialog::localDataToSyncSlot()
{
    startSyncProcess();
}

void SyncProcessDialog::sessionKeyChangedSlot()
{
    if (SyncSession::LOCAL_DATA_CHANGED) {
        //revision conflict
        syncRevisionConflictSlot();
    } else {
        ui->stackedWidget->setCurrentIndex(2);
        ui->syncNowButton->setFocus();
    }
}

void SyncProcessDialog::newSyncRevisionAvailableSlot()
{
    ui->stackedWidget->setCurrentIndex(2);
    ui->syncNowButton->setFocus();
}

void SyncProcessDialog::syncRevisionConflictSlot()
{
    ui->stackedWidget->setCurrentIndex(4);
}

void SyncProcessDialog::metadataInitConflictSlot()
{
    ui->stackedWidget->setCurrentIndex(0);
}

void SyncProcessDialog::initConfOkButtonClicked()
{
    if (ui->dropCloudInitConfRadioButton->isChecked()) {
        ui->stackedWidget->setCurrentIndex(1);

        //create new metadata and upload it
        metadataNotFoundSlot();
    } else {
        m_settingsManager->setCloudSyncInitialized(true);
        ui->stackedWidget->setCurrentIndex(3);

        //clear file manager's lists
        m_fileManager->clearAllLists();

        //drop local by starting sync
        startSyncProcess();
    }
}

void SyncProcessDialog::initOkButtonClicked()
{
    ui->stackedWidget->setCurrentIndex(3);
    startSyncCheck();
}

void SyncProcessDialog::syncNowButtonClicked()
{
    ui->stackedWidget->setCurrentIndex(3);
    startSyncProcess();
}

void SyncProcessDialog::revConfInfoOkButtonClicked()
{
    if (ui->dropCloudRevConfRadioButton->isChecked()) {
        ui->stackedWidget->setCurrentIndex(5);
        ui->revConfOperationLabel->setText(tr("Uploading..."));

        //create new metadata and upload it
        m_syncEngine->startCloudInitConfig();
    } else {
        ui->stackedWidget->setCurrentIndex(3);

        //clear file manager's lists
        m_fileManager->clearAllLists();

        //drop local by starting sync
        startSyncProcess();
    }
}

void SyncProcessDialog::revConfOkButtonClicked()
{
    ui->stackedWidget->setCurrentIndex(3);
    startSyncProcess();
}

void SyncProcessDialog::chunkUploadedSlot(int uploadedChunk, int totalChunks)
{
    ui->syncCurrentTaskProgressBar->setMaximum(totalChunks);
    ui->syncCurrentTaskProgressBar->setValue(uploadedChunk);
}

void SyncProcessDialog::downloadFileNotFoundSlot(const QString &file)
{
    syncError(tr("Download error. The following file was not found on "
                 "the cloud service: %1").arg(file));
}

void SyncProcessDialog::fileDownloadedSlot(const QString &src,
                                           const QString &dest)
{
    //reset current task progress
    ui->syncCurrentTaskProgressBar->setValue(-1);
    ui->syncCurrentTaskProgressBar->setRange(0, 0);

    if (src == m_metadataFileName) {
        return;
    } else if (src == m_dbName) {
        //delete db
        ui->syncCurrentTaskLabel->setText(tr("Replacing..."));
        qApp->processEvents();
        if (!QFile::remove(m_dbPath)) {
            syncError(tr("Error: Failed to delete old database file."));
            return;
        }
        //rename
        QFile::rename(dest, m_dbPath);
        //open db
        m_databaseManager->getInstance();

        //continue with file sync
        syncFiles();
    } else {
        //remove file from file list
        m_filesToDownload.removeOne(src);

        //continue file sync
        syncFiles();
    }
}

void SyncProcessDialog::fileUploadedSlot(const QString &src,
                                         const QString &dest)
{
    Q_UNUSED(src);

    //reset current task progress
    ui->syncCurrentTaskProgressBar->setValue(-1);
    ui->syncCurrentTaskProgressBar->setRange(0, 0);

    if (dest == m_metadataFileName) {
        if (m_syncCompleted)
            completeSync();
    } else {
        //remove file from file lists
        m_fileManager->removeFileFromUploadList(dest);
        m_filesToUpload.removeOne(dest);

        //continue file sync
        syncFiles();
    }
}

void SyncProcessDialog::fileRemovedSlot(const QString &file)
{
    Q_UNUSED(file);

    //remove file from file lists
    m_fileManager->removeFileFromRemoveList(file);
    m_filesToCloudRemove.removeOne(file);

    //continue file sync
    syncFiles();
}

void SyncProcessDialog::syncCancelButtonClicked()
{
    //restore old metadata because sync was cancelled
    if (m_metadataBeforeSync)
        m_syncEngine->writeMetadataFile(*m_metadataBeforeSync);
}


//-----------------------------------------------------------------------------
// Private
//-----------------------------------------------------------------------------

void SyncProcessDialog::init()
{
    ui->syncCurrentTaskLabel->hide();
    ui->syncCurrentTaskProgressBar->hide();
    m_settingsManager = new SettingsManager;
    m_databaseManager = &DatabaseManager::getInstance();
    m_fileManager = new FileManager(this);
    m_dbPath = m_databaseManager->getDatabasePath();
    m_dbName = m_databaseManager->getDatabaseName();
    m_filesDir = m_fileManager->getFilesDirectory();
}

void SyncProcessDialog::initSync()
{
    m_syncEngine = &SyncEngine::getInstance();
    m_syncDriver = m_syncEngine->getCurrentSyncDriver(this);
    m_metadataFileName = m_syncEngine->getMetadataFileName();
    m_metadataFilePath = m_syncEngine->getMetadataFilePath();
}

void SyncProcessDialog::createConnections()
{
    connect(ui->syncCancelButton, SIGNAL(clicked()),
            this, SLOT(syncCancelButtonClicked()));
    connect(ui->syncOkButton, SIGNAL(clicked()),
            this, SLOT(accept()));
    connect(ui->initCancelButton, SIGNAL(clicked()),
            this, SLOT(reject()));
    connect(ui->initConfCancelButton, SIGNAL(clicked()),
            this, SLOT(reject()));
    connect(ui->revConfInfoCancelButton, SIGNAL(clicked()),
            this, SLOT(reject()));
    connect(ui->revConfCancelButton, SIGNAL(clicked()),
            this, SLOT(reject()));
    connect(ui->syncCancelButton, SIGNAL(clicked()),
            this, SLOT(reject()));
    connect(ui->cancelNewRevButton, SIGNAL(clicked()),
            this, SLOT(reject()));
    connect(ui->initConfOkButton, SIGNAL(clicked()),
            this, SLOT(initConfOkButtonClicked()));
    connect(ui->initOkButton, SIGNAL(clicked()),
            this, SLOT(initOkButtonClicked()));
    connect(ui->syncNowButton, SIGNAL(clicked()),
            this, SLOT(syncNowButtonClicked()));
    connect(ui->revConfInfoOkButton, SIGNAL(clicked()),
            this, SLOT(revConfInfoOkButtonClicked()));
    connect(ui->revConfOkButton, SIGNAL(clicked()),
            this, SLOT(revConfOkButtonClicked()));
}

void SyncProcessDialog::createSyncConnections()
{
    //sync engine
    connect(m_syncEngine, SIGNAL(connectionFailed()),
            this, SLOT(connectionFailedSlot()));
    connect(m_syncEngine, SIGNAL(authTokenExpired()),
            this, SLOT(tokenInvalidOrExpiredSlot()));
    connect(m_syncEngine, SIGNAL(syncError(QString)),
            this, SLOT(syncError(QString)));
    connect(m_syncEngine, SIGNAL(metadataNotFound()),
            this, SLOT(metadataNotFoundSlot()));
    connect(m_syncEngine, SIGNAL(nothingToSync()),
            this, SLOT(nothingToSyncSlot()));
    connect(m_syncEngine, SIGNAL(cloudInitConfigCompleted()),
            this, SLOT(cloudInitConfigCompletedSlot()));
    connect(m_syncEngine, SIGNAL(firstSyncSignal()),
            this, SLOT(firstTimeSyncSlot()));
    connect(m_syncEngine, SIGNAL(localDataToSyncSignal()),
            this, SLOT(localDataToSyncSlot()));
    connect(m_syncEngine, SIGNAL(newSyncRevisionAvailable()),
            this, SLOT(newSyncRevisionAvailableSlot()));
    connect(m_syncEngine, SIGNAL(syncRevisionConflict()),
            this, SLOT(syncRevisionConflictSlot()));
    connect(m_syncEngine, SIGNAL(metadataInitConflict()),
            this, SLOT(metadataInitConflictSlot()));
    connect(m_syncEngine, SIGNAL(clientAlreadyLoggedIn()),
            this, SLOT(clientAlreadyLoggedInSlot()));
    connect(m_syncEngine, SIGNAL(sessionKeyChanged()),
            this, SLOT(sessionKeyChangedSlot()));

    //sync driver
    connect(m_syncDriver, SIGNAL(connectionFailed()),
            this, SLOT(connectionFailedSlot()));
    connect(m_syncDriver, SIGNAL(authTokenExpired()),
            this, SLOT(tokenInvalidOrExpiredSlot()));
    connect(m_syncDriver, SIGNAL(errorSignal(QString)),
            this, SLOT(syncError(QString)));
    connect(m_syncDriver, SIGNAL(uploadedChunkReady(int,int)),
            this, SLOT(chunkUploadedSlot(int,int)));
    connect(m_syncDriver, SIGNAL(downloadReady(QString,QString)),
            this, SLOT(fileDownloadedSlot(QString,QString)));
    connect(m_syncDriver, SIGNAL(uploadReady(QString,QString)),
            this, SLOT(fileUploadedSlot(QString,QString)));
    connect(m_syncDriver, SIGNAL(removeReady(QString)),
            this, SLOT(fileRemovedSlot(QString)));
    connect(m_syncDriver, SIGNAL(downloadFileNotFound(QString)),
            this, SLOT(downloadFileNotFoundSlot(QString)));
}

void SyncProcessDialog::startSyncCheck()
{
    //update metadata file
    m_syncEngine->startMetadataUpdate();
}

void SyncProcessDialog::startSyncProcess()
{
    SyncEngine::MetadataFile metadata = m_syncEngine->readMetadataFile();
    quint64 settingsRevision = m_settingsManager->restoreCloudSyncRevision();
    bool firstSync = m_settingsManager->isCloudSyncFirstTime();
    bool localDataChanged = SyncSession::LOCAL_DATA_CHANGED;
    m_syncCompleted = false;

    //backup old metadata in case sync is cancelled
    if (!m_metadataBeforeSync)
        m_metadataBeforeSync = new SyncEngine::MetadataFile;
    *m_metadataBeforeSync = metadata;

    m_filesToDownload.clear();
    m_filesToUpload.clear();
    m_filesToCloudRemove.clear();
    m_filesToLocalRemove.clear();
    m_databaseSyncOp = DbInvalidOp;
    m_downloadFileOp = DownloadNoOp;
    m_uploadFileOp = UploadNoOp;
    m_cleanFilesOp = false;
    m_fileSyncInitialized = false;

    if ((metadata.revision > settingsRevision) ||
            (metadata.sessionKey != m_settingsManager->restoreCloudSessionKey())) {

        //download db
        m_databaseSyncOp = DownloadDbOp;

        //file operation
        m_downloadFileOp = DownloadNeededOp;

    } else if (metadata.revision <= settingsRevision) {

        if (firstSync) {

            if ((metadata.revision == 1) && (settingsRevision == 1)) {
                //upload db
                m_databaseSyncOp = UploadDbOp;

                //file operation
                m_uploadFileOp = UploadAllOp;

                metadata.revision = metadata.revision + 1;
                metadata.filesChanged = true;

            } else {
                //download db
                m_databaseSyncOp = DownloadDbOp;

                //file operation
                m_downloadFileOp = DownloadAllOp;
            }

        } else if (localDataChanged) {
            //upload db
            m_databaseSyncOp = UploadDbOp;

            metadata.revision = metadata.revision + 1;

            if (m_fileManager->localFileChangesToSync()) {
                metadata.filesChanged = true;

                //file operation
                m_uploadFileOp = UploadNeededOp;
            } else {
                metadata.filesChanged = false;
            }
        }

    } else {
        syncError(tr("Sync process could not be started: unexpected case."));
        return;
    }

    //update metadata file
    metadata.isOpen = true;
    metadata.databaseFormat = DefinitionHolder::DATABASE_VERSION;
    m_syncEngine->writeMetadataFile(metadata);

    //remove old files that are not needed
    if (m_metadataBeforeSync->revision == (settingsRevision + 1))
        m_cleanFilesOp = metadata.filesChanged;
    else
        m_cleanFilesOp = true; //outdated revision so files may be changed

    //close database
    m_databaseManager->destroy();

    ui->syncCurrentTaskLabel->show();
    ui->syncCurrentTaskProgressBar->show();

    //set progress bar
    int steps = 2;
    if (m_cleanFilesOp)
        steps++;
    if (m_uploadFileOp != UploadNoOp)
        steps++;
    if(m_downloadFileOp != DownloadNoOp)
        steps++;
    ui->syncProgressBar->setRange(0, steps);
    ui->syncProgressBar->setValue(1);

    switch (m_databaseSyncOp) {
    case UploadDbOp:
        ui->syncOperationLabel->setText(tr("Uploading..."));
        ui->syncCurrentTaskLabel->setText(QString("%1").arg(m_dbName));
        m_syncDriver->startUploadRequest(m_dbPath, m_dbName);
        break;
    case DownloadDbOp:
        ui->syncOperationLabel->setText(tr("Downloading..."));
        ui->syncCurrentTaskLabel->setText(QString("%1").arg(m_dbName));
        m_syncDriver->startDownloadRequest(m_dbName, QString(m_dbPath + "_tmp"));
        break;
    case DbInvalidOp:
        break; //nothing
    }
}

void SyncProcessDialog::syncFiles()
{
    ui->syncProgressBar->setValue(ui->syncProgressBar->value() + 1);
    qApp->processEvents();

    if (!m_fileSyncInitialized) {
        switch (m_downloadFileOp) {
        case DownloadNeededOp:
            m_filesToDownload = m_fileManager->fileListToDownload();
            break;
        case DownloadAllOp:
            m_filesToDownload = m_fileManager->fullFileList();
            break;
        case DownloadNoOp:
            //nothing
            break;
        }

        switch (m_uploadFileOp) {
        case UploadNeededOp:
            m_filesToUpload = m_fileManager->fileListToUpload();
            break;
        case UploadAllOp:
            m_filesToUpload = m_fileManager->fullFileList();
            break;
        case UploadNoOp:
            //nothing
            break;
        }

        //files to remove from cloud
        m_filesToCloudRemove = m_fileManager->fileListToRemove();

        //clean unneeded files
        if (m_cleanFilesOp) {
            m_filesToLocalRemove = m_fileManager->unneededLocalFileList();
        }

        //update progress range
        int max = ui->syncProgressBar->maximum();
        max += m_filesToDownload.size();
        max += m_filesToUpload.size();
        max += m_filesToCloudRemove.size();
        max += m_filesToLocalRemove.size();
        ui->syncProgressBar->setMaximum(max);
        qApp->processEvents();

        m_fileSyncInitialized = true;
    }

    //sync files
    if (m_filesToUpload.size()) {
        QString f = m_filesToUpload.at(0);
        ui->syncOperationLabel->setText(tr("Uploading..."));
        ui->syncCurrentTaskLabel->setText(QString(f).left(6) + "..." +
                                          QString(f).right(3));
        m_syncDriver->startUploadRequest(QString(m_filesDir + f), f);
        return; //on request completion syncFiles() is auto-called again
    }
    if (m_filesToDownload.size()) {
        QString f = m_filesToDownload.at(0);
        ui->syncOperationLabel->setText(tr("Downloading..."));
        ui->syncCurrentTaskLabel->setText(QString(f).left(6) + "..." +
                                          QString(f).right(3));
        m_syncDriver->startDownloadRequest(f, QString(m_filesDir + f));
        return;
    }
    if (m_filesToLocalRemove.size()) {
        foreach (QString s, m_filesToLocalRemove) {
            ui->syncOperationLabel->setText(tr("Removing..."));
            ui->syncCurrentTaskLabel->setText(QString(s).left(6) + "..." +
                                              QString(s).right(3));
            qApp->processEvents();
            QFile::remove(m_filesDir + s);
            ui->syncProgressBar->setValue(ui->syncProgressBar->value() + 1);
        }
        m_filesToLocalRemove.clear();
    }

    if (m_filesToCloudRemove.size()) {
        QString f = m_filesToCloudRemove.at(0);
        ui->syncOperationLabel->setText(tr("Removing from cloud..."));
        ui->syncCurrentTaskLabel->setText(QString(f).left(9) + "..." +
                                          QString(f).right(3));
        m_syncDriver->startRemoveRequest(f);
        return;
    }

    //sync done
    m_syncCompleted = true;

    //upload new metadata file
    ui->syncOperationLabel->setText(tr("Uploading..."));
    ui->syncCurrentTaskLabel->setText(m_metadataFileName);
    m_syncDriver->startUploadRequest(m_metadataFilePath, m_metadataFileName);

    ui->syncProgressBar->setValue(ui->syncProgressBar->value() + 1);
}

void SyncProcessDialog::completeSync()
{
    ui->syncProgressBar->setValue(ui->syncProgressBar->maximum());
    qApp->processEvents();

    ui->stackedWidget->setCurrentIndex(3);
    ui->syncResultLabel->setText(tr("Your database has been successfully "
                                    "synchronized."));
    ui->syncOperationLabel->hide();
    ui->syncProgressBar->hide();
    ui->syncCurrentTaskLabel->hide();
    ui->syncCurrentTaskProgressBar->hide();
    ui->syncOkButton->setEnabled(true);
    ui->syncOkButton->setFocus();

    //update sync settings from metadata
    SyncEngine::MetadataFile metadata = m_syncEngine->readMetadataFile();
    m_settingsManager->setCloudSyncFirstTime(false);
    m_settingsManager->saveCloudLocalDataChanged(false);
    SyncSession::LOCAL_DATA_CHANGED = false;
    m_settingsManager->saveCloudSessionKey(metadata.sessionKey);
    m_settingsManager->saveCloudSyncRevision(metadata.revision);

    //after sync current collection id might be changed
    //so set cached id dirty
    MetadataEngine::getInstance().setDirtyCurrentColleectionId();

    if (m_autoCloseAfterSync)
        accept();
}
