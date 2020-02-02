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

#include <QtCore/QProcess>


//-----------------------------------------------------------------------------
// Forward declarations
//-----------------------------------------------------------------------------


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
    void processError(QProcess::ProcessError error);
    void processFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void processReadyReadOutput();

private:
    /** Supported request types */
    enum SyncRequest {
        NoRequest,
        AuthRequest,
        AuthValidationRequest,
        UserNameRequest,
        DownloadRequest,
        UploadRequest,
        RemoveRequest
    };

    void initSecrets();
    void startRequest();

    QProcess *m_process;
    SyncRequest m_currentRequest;
    QString m_accessTokenEncoded;
    QString m_appSecretEncoded;
    QStringList m_requestArgs;
    QString m_processOutput;
    int m_totUploadChunks;
    int m_chunksUploaded;
};

#endif // FOLDERSYNCDRIVER_H
