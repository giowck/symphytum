/**
  * \class FileManager
  * \brief This class handles all operation related to storage of files
  *        that belong to database content data. Files which are added
  *        as content data are not stored directly in the sqlite database
  *        instead there are just copied to a directory and linked to
  *        a database table. This class handles all operations such
  *        as file add/remove etc. All operations are sync aware, this means
  *        that this component is integrated with sync services to keep track
  *        of files that need to be uploaded/downloaded or deleted.
  * \author Giorgio Wicklein - GIOWISYS Software
  * \date 28/08/2012
  */

#ifndef FILEMANAGER_H
#define FILEMANAGER_H


//-----------------------------------------------------------------------------
// Headers
//-----------------------------------------------------------------------------

#include <QtCore/QObject>


//-----------------------------------------------------------------------------
// Forward declarations
//-----------------------------------------------------------------------------

class FileTask : public QObject
{
    Q_OBJECT
public:
    enum FileOp {
        CopyOp,
        RemoveOp
    };
    FileTask(const QString &filesDir, QObject *parent = nullptr);
    ~FileTask();
    void configureTask(const QString &srcfileName,
                       const QString &destFileName = QString(),
                       FileOp operation = RemoveOp);
public slots:
    void startFileOp();
signals:
    void finishedSignal(const QString &srcFileName,
                        const QString &destFileName,
                        int operation);
    void errorSignal(const QString &message);
private:
    QString m_srcFileName;
    QString m_destFileName;
    FileOp m_currentOp;
    QString m_filesDir;
};

class MetadataEngine;
class SettingsManager;


//-----------------------------------------------------------------------------
// FileManager
//-----------------------------------------------------------------------------

class FileManager : public QObject
{
    Q_OBJECT

public:
    explicit FileManager(QObject *parent = nullptr);
    ~FileManager();
    
    /** Whether local files were added/removed and need to be synced */
    bool localFileChangesToSync();

    /** Return directory where db content files are saved */
    QString getFilesDirectory();

    /** Return a list of files that should be downloaded (sync) */
    QStringList fileListToDownload();

    /** Return a list with all files (from files table of database) */
    QStringList fullFileList();

    /** Return a list of files that should be uploaded (sync) */
    QStringList fileListToUpload();

    /** Return a list of files that should be removed from cloud (sync) */
    QStringList fileListToRemove();

    /** Return a list of files that are not in db but in local files dir */
    QStringList unneededLocalFileList();

    /** Return a list of files that are in database's file table but not used in records */
    QStringList orphanDatabaseFileList();

    /** Remove the specified file from to upload list */
    void removeFileFromUploadList(const QString &file);

    /** Remove the specified file from to remove list */
    void removeFileFromRemoveList(const QString &file);

    /** Clear all file lists (to upload/to delete/to watch) */
    void clearAllLists();

    /**
     * Start an async file add process.
     * This method is used to add external user files to the database
     * (saved in the files dir). When adding a file, the specified file
     * is copied (asnyc) and added to the database files table, where all
     * files are referenced. This method is async because big files may take
     * a while to be copied. Once completed, the addFileCompleted() signal is
     * emitted.
     */
    void startAddFile(const QString &file);

    /**
     * Start an async file remove process.
     * This method is used to remove user files from the database.
     * When removing a file, the specified file
     * is deleted (asnyc) and removed from the database files table, where all
     * files are referenced. This method is async because big files may take
     * a while to be deleted. Once completed, the removeFileCompleted() signal is
     * emitted.
     */
    void startRemoveFile(const QString &file);

    /**
     * Open a content file and if sync is enabled the file is added
     * to a file watch list to detect changes to that files. On next
     * sync, if necessary, those files are handled.
     * @param file - content file to open and watch for edits
     */
    void openContentFile(const QString &file);

    /** Delete all content files saved in the files directory */
    void removeAllFiles();

    /** Get all local files that are in the files directory (not from db) */
    QStringList getAllLocalFiles();

    /**
     * Remove the specified file id from database's files table and if sync
     * is enabled, add file to cloud delete list.
     * This method is designed to delete directly all metadata
     * related to the specified file.
     * NOTE: the specified file is not deleted from file system only its metadata
     * and cloud data.
     */
    void removeFileMetadata(const int fileId);

public slots:
    /** Stop the file operation thread if running */
    void stopFileOp();

signals:
    /** This signal is emitted when a startAddFile() request completes */
    void addFileCompletedSignal(const QString &file);

    /** This signal is emitted when a startRemoveFile() request completes */
    void removeFileCompletedSignal(const QString &file);

    /** Emitted when an error occurred during file op */
    void fileOpFailed();

private slots:
    void fileOperationErrorSlot(const QString &message);
    void fileOperationFinishedSlot(const QString &srcFileName,
                                   const QString &destFileName,
                                   int op);

private:
    void createFileThreadConnections(QThread *thread, FileTask *fileTask);
    void addFileToUploadList(const QString &file);
    void addFileToDeleteList(const QString &file);
    void addFileToWatchList(const QString &file);

    QString m_fileDirPath; /**< The path where content data files are saved */
    QThread *m_fileOpThread;
    MetadataEngine *m_metadataEngine;
    SettingsManager *m_settingsManager;
};

#endif // FILEMANAGER_H
