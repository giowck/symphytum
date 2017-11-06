/**
  * \class SyncProcessDialog
  * \brief This dialog is used to sync with a cloud service.
  *        This is where the sync happens with all its
  *        exceptions and handlers.
  * \author Giorgio Wicklein - GIOWISYS Software
  * \date 29/08/2012
  */

#ifndef SYNCPROCESSDIALOG_H
#define SYNCPROCESSDIALOG_H


//-----------------------------------------------------------------------------
// Headers
//-----------------------------------------------------------------------------

#include "../components/sync_framework/syncengine.h"

#include <QtWidgets/QDialog>


//-----------------------------------------------------------------------------
// Forward declarations
//-----------------------------------------------------------------------------

namespace Ui {
class SyncProcessDialog;
}

class AbstractSyncDriver;
class SettingsManager;
class DatabaseManager;
class FileManager;

#ifdef Q_OS_WIN
class QWinTaskbarProgress;
#endif


//-----------------------------------------------------------------------------
// SyncProcessDialog
//-----------------------------------------------------------------------------

class SyncProcessDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit SyncProcessDialog(QWidget *parent = 0);
    ~SyncProcessDialog();

    /** Set that on sync completion this dialog will close itself */
    void enableAutoCloseAfterSync();

public slots:
    void reject();

private slots:
    void connectionFailedSlot();
    void tokenInvalidOrExpiredSlot();
    void syncError(const QString &message);
    void metadataNotFoundSlot();
    void nothingToSyncSlot();
    void clientAlreadyLoggedInSlot();
    void cloudInitConfigCompletedSlot();
    void firstTimeSyncSlot();
    void localDataToSyncSlot();
    void sessionKeyChangedSlot();
    void newSyncRevisionAvailableSlot();
    void syncRevisionConflictSlot();
    void metadataInitConflictSlot();
    void initConfOkButtonClicked();
    void initOkButtonClicked();
    void syncNowButtonClicked();
    void revConfInfoOkButtonClicked();
    void revConfOkButtonClicked();
    void chunkUploadedSlot(int uploadedChunk, int totalChunks);
    void downloadFileNotFoundSlot(const QString &file);
    void fileDownloadedSlot(const QString &src, const QString &dest);
    void fileUploadedSlot(const QString &src, const QString &dest);
    void fileRemovedSlot(const QString &file);
    void syncCancelButtonClicked();
    
private:
    void init();
    void initSync();
    void createConnections();
    void createSyncConnections();

    /** Start checking the status of the cloud by emitting apropriate signals */
    void startSyncCheck();

    /** Start sync process */
    void startSyncProcess();

    /** Continue sync process until all lists of op on files are done */
    void syncFiles();

    /** Called when sync() is done */
    void completeSync();

    /** Enum of operations for database file */
    enum DatabaseSyncOp {
        DbInvalidOp, /**< The database op is invalid */
        UploadDbOp, /**< The database file should be uploaded */
        DownloadDbOp /**< The database file should be downloaded */
    };

    /** Enum for download file instruction */
    enum DownloadFileOp {
        DownloadNoOp, /**< No files to download */
        DownloadNeededOp, /**< Download all files that are not already present */
        DownloadAllOp /**< Download all files */
    };

    /** Enum for upload file instruction */
    enum UploadFileOp {
        UploadNoOp, /**< No files to upload */
        UploadNeededOp, /**< Upload all files that are not already present */
        UploadAllOp /**< Upload all files */
    };

    Ui::SyncProcessDialog *ui;
#ifdef Q_OS_WIN
    QWinTaskbarProgress *m_taskbarProgress;
#endif
    SyncEngine *m_syncEngine;
    DatabaseManager *m_databaseManager;
    FileManager *m_fileManager;
    AbstractSyncDriver *m_syncDriver;
    SettingsManager *m_settingsManager;
    QStringList m_filesToDownload;
    QStringList m_filesToUpload;
    QStringList m_filesToCloudRemove;
    QStringList m_filesToLocalRemove;
    DatabaseSyncOp m_databaseSyncOp;
    DownloadFileOp m_downloadFileOp;
    UploadFileOp m_uploadFileOp;
    bool m_cleanFilesOp;
    bool m_fileSyncInitialized;
    QString m_dbPath;
    QString m_dbName;
    QString m_filesDir;
    QString m_metadataFileName;
    QString m_metadataFilePath;
    bool m_syncCompleted;
    bool m_autoCloseAfterSync;
    SyncEngine::MetadataFile *m_metadataBeforeSync;
};

#endif // SYNCPROCESSDIALOG_H
