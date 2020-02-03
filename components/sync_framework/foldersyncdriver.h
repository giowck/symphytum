/**
  * \class FolderSyncDriver
  * \brief This class implements the sync driver for a generic folder based sync service.
  * \author Oirio Joshi (joshirio)
  * \date 2020-02-02
  */

#ifndef FOLDERSYNCDRIVER_H
#define FOLDERSYNCDRIVER_H


//-----------------------------------------------------------------------------
// Headers
//-----------------------------------------------------------------------------

#include "abstractsyncdriver.h"

#include <QtCore/QThread>


//-----------------------------------------------------------------------------
// Forward declarations
//-----------------------------------------------------------------------------

class FolderSyncTask : public QObject
{
    Q_OBJECT
public:
    enum FileOp {
        CopyOp,
        RemoveOp
    };
    FolderSyncTask(QObject *parent = nullptr);
    ~FolderSyncTask();
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
};


//-----------------------------------------------------------------------------
// FolderSyncDriver
//-----------------------------------------------------------------------------

class FolderSyncDriver : public AbstractSyncDriver
{
    Q_OBJECT

public:
    explicit FolderSyncDriver(QObject *parent = nullptr);
    ~FolderSyncDriver();

    void startAuthenticationRequest(const QStringList &args);
    void startAuthenticationValidationRequest(const QString &authToken);
    void startUserNameRequest();
    void startDownloadRequest(const QString &srcFilePath,
                              const QString &destFilePath);
    void startUploadRequest(const QString &srcFilePath,
                            const QString &destFilePath);
    void startRemoveRequest(const QString &cloudFilePath);
    void stopAllRequests();
    QString getServiceUrl();

private slots:
    /** Stop the file operation thread if running */
    void stopFileOp();
    void fileOperationErrorSlot(const QString &message);
    void fileOperationFinishedSlot(const QString &srcFileName,
                                   const QString &destFileName,
                                   int op);

private:
    /** Supported request types */
    enum SyncRequest {
        NoRequest,
        InitRequest,
        AuthValidationRequest,
        UserNameRequest,
        DownloadRequest,
        UploadRequest,
        RemoveRequest
    };

    void startRequest();
    void createFileThreadConnections(QThread *thread, FolderSyncTask *fileTask);

    SyncRequest m_currentRequest;
    QString m_folderPath;
    QStringList m_requestArgs;
    QThread *m_fileOpThread;
};

#endif // FOLDERSYNCDRIVER_H
