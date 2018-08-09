/**
  * \class BackupManager
  * \brief This class is used to create/restore backups of the main database.
  *        Export/import of full database, including all files.
  * \author Giorgio Wicklein - GIOWISYS Software
  * \date 28/08/2012
  */

#ifndef BACKUPMANAGER_H
#define BACKUPMANAGER_H


//-----------------------------------------------------------------------------
// Headers
//-----------------------------------------------------------------------------

#include <QtCore/QObject>


//-----------------------------------------------------------------------------
// Forward declarations
//-----------------------------------------------------------------------------

class BackupTask : public QObject
{
    Q_OBJECT
public:
    enum BackupOp {
        ExportOp,
        ImportOp
    };
    BackupTask(const QString &filesDir,
               const QString &databasePath,
               QObject *parent = nullptr);
    ~BackupTask();
    void configureTask(const QString &path,
                       BackupOp operation = ExportOp);
public slots:
    void startBackupTask();
signals:
    void finishedSignal(int op);
    void progressSignal(int currentStep, int totalSteps);
    void errorSignal(const QString &message);
private:
    bool fullExport(const QString &destPath, QString &errorMessage);
    bool fullImport(const QString &filePath, QString &errorMessage);
    QString m_filesDir;
    QString m_dbPath;
    int m_totalProgress;
    int m_currentProgress;
    BackupOp m_currentOp;
    QString m_path;
    QStringList m_contentFileList;
    int m_magicNumber;
    int m_fileBufSize;
};


//-----------------------------------------------------------------------------
// BackupManager
//-----------------------------------------------------------------------------

class BackupManager : public QObject
{
    Q_OBJECT

public:
    explicit BackupManager(QObject *parent = nullptr);
    ~BackupManager();

    /**
     * Start an async full backup/export process.
     * This method is used to export the database including all files.
     * Once completed, the exportCompleted() signal is emitted.
     * @param destFilePath - the path where the backup file is saved
     */
    void startExport(const QString &destFilePath);

    /**
     * Start an async full database import process.
     * This method is used to import the database including all files.
     * Once completed, the importCompleted() signal is emitted.
     * @param importFilePath - the path of the backup file to restore
     */
    void startImport(const QString &importFilePath);

public slots:
    /** Stop the backup task thread if running */
    void stopBackupTask();

signals:
    /** This signal is emitted when a startExport() request completes */
    void exportCompleted();

    /** This signal is emitted when a startImport() request completes */
    void importCompleted();

    /** Emitted when an error occurred during import/export task */
    void backupTaskFailed(const QString &message);

    /** Emitted to signal progress state of current backup task */
    void progressSignal(int currentStep, int totalSteps);

private slots:
    void backupTaskErrorSlot(const QString &message);
    void backupTaskFinishedSlot(int op);

private:
    void createBackupThreadConnections(QThread *thread,
                                       BackupTask *backupTask);

    QThread *m_backupTaskThread;
    QString m_fileDirPath; /**< The path where content data files are saved */
    QString m_databasePath; /**< The path where database file is saved */
};

#endif // BACKUPMANAGER_H
