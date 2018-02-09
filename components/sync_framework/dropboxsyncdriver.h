/**
  * \class DropboxSyncDriver
  * \brief This class implements the sync driver for the Dropbox service
  *        by using the Dropbox Python SDK package.
  * \author Giorgio Wicklein - GIOWISYS Software
  * \date 10/08/2012
  */

#ifndef DROPBOXSYNCDRIVER_H
#define DROPBOXSYNCDRIVER_H


//-----------------------------------------------------------------------------
// Headers
//-----------------------------------------------------------------------------

#include "abstractsyncdriver.h"

#include <QtCore/QProcess>


//-----------------------------------------------------------------------------
// Forward declarations
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// DropboxSyncDriver
//-----------------------------------------------------------------------------

class DropboxSyncDriver : public AbstractSyncDriver
{
    Q_OBJECT

public:
    explicit DropboxSyncDriver(QObject *parent = 0);
    ~DropboxSyncDriver();

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

#endif // DROPBOXSYNCDRIVER_H
