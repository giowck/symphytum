/**
  * \class MegaSyncDriver
  * \brief This class implements the sync driver for the MEGA service
  *        by using the MEGAcmd command line tools.
  * \author Oirio Joshi (joshirio)
  * \date 2018-02-08
  */

#ifndef MEGASYNCDRIVER_H
#define MEGASYNCDRIVER_H


//-----------------------------------------------------------------------------
// Headers
//-----------------------------------------------------------------------------

#include "abstractsyncdriver.h"

#include <QtCore/QProcess>


//-----------------------------------------------------------------------------
// Forward declarations
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// MegaSyncDriver
//-----------------------------------------------------------------------------

class MegaSyncDriver : public AbstractSyncDriver
{
    Q_OBJECT

public:
    explicit MegaSyncDriver(QObject *parent = 0);
    ~MegaSyncDriver();

    void startAuthenticationRequest();
    void startAuthenticationValidationRequest(QString &authToken);
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

#endif // MEGASYNCDRIVER_H
