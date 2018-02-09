/**
  * \class AbstractSyncDriver
  * \brief This is the abstract base class to create sync service drivers.
  *        A sync driver allows to upload/download files from cloud service
  *        providers in addition to other operations. A sync driver works
  *        asynchronously and emits signals to notify the state of an operation
  *        such as an file upload. Other methods with a short duration in time
  *        are synchronous, check docs for each methods.
  * \author Giorgio Wicklein - GIOWISYS Software
  * \date 10/08/2012
  */

#ifndef ABSTRACTSYNCDRIVER_H
#define ABSTRACTSYNCDRIVER_H


//-----------------------------------------------------------------------------
// Headers
//-----------------------------------------------------------------------------

#include <QtCore/QObject>


//-----------------------------------------------------------------------------
// Forward declarations
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// AbstractSyncDriver
//-----------------------------------------------------------------------------

class AbstractSyncDriver : public QObject
{
    Q_OBJECT

public:
    explicit AbstractSyncDriver(QObject *parent = 0);
    virtual ~AbstractSyncDriver();

    /**
     * This method starts the authentication process (OAuth)
     * and emits the authenticationUrlReady() signal with
     * the URL to be visited by the user
     * @param authArgs - optional arguments list if needed by the implementation
     */
    virtual void startAuthenticationRequest(const QStringList &authArgs) = 0;

    /** @overload */
    void startAuthenticationRequest();

    /**
     * This method validates the authentication request done
     * by startAuthenticationRequest. Usually an OAuth
     * access token needs to be created and stored. A subclass
     * should create that access token in this method and store
     * it in an encoded/encrypted way in the settings.
     * Emit authenticationValidated() when access token has been
     * created and stored.
     * @param authToken - the OAuth auth token required to create the access token
     */
    virtual void startAuthenticationValidationRequest(const QString &authToken) = 0;

    /**
     * Start a request to get the user name of the cloud service account
     */
    virtual void startUserNameRequest() = 0;

    /**
     * Start a download request for the given file.
     * Emit downloadReady() on download completion.
     * @param srcFilePath - the file to download from cloud
     * @param destFilePath - where to save the file (overwrite)
     */
    virtual void startDownloadRequest(const QString &srcFilePath,
                                      const QString &destFilePath) = 0;

    /**
     * Start an upload request for the given file.
     * Emit uploadReady() on upload completion.
     * @param srcFilePath - the file to upload to the cloud
     * @param destFilePath - where to save the file (overwrite) in the cloud
     */
    virtual void startUploadRequest(const QString &srcFilePath,
                                    const QString &destFilePath) = 0;

    /**
     * Start a remove request for the given file.
     * Emit removeReady() on delete completion.
     * @param cloudFilePath - the file to delete from the cloud service
     */
    virtual void startRemoveRequest(const QString &cloudFilePath) = 0;

    /** Stop all running requests */
    virtual void stopAllRequests() = 0;

    /** Return the home page of the cloud service as html formatted link */
    virtual QString getServiceUrl() = 0;

signals:
    /** Emitted when the athentication/token has expired */
    void authTokenExpired();

    /** Emitted if the connection to the sync service failed */
    void connectionFailed();

    /** Emitted when the result of an authentication request is ready */
    void authenticationUrlReady(const QString &url);

    /** Emitted when the authentication validatation request completed */
    void authenticationValidated();

    /** Emitted on user name request completion */
    void userNameResultReady(const QString &userName);

    /** Emitted when an error occurs during a request */
    void errorSignal(const QString &message);

    /** Emitted when a file has been downloaded */
    void downloadReady(const QString &srcFilePath,
                       const QString &destFilePath);

    /** Emitted if a download request failed due a not found error */
    void downloadFileNotFound(const QString &filePath);

    /** Emitted when a file has been uploaded */
    void uploadReady(const QString &srcFilePath,
                     const QString &destFilePath);

    /** Emitted when a remove request completed */
    void removeReady(const QString &cloudFilePath);

    /** Emitted when no more free space on the cloud service is available */
    void storageQuotaExceeded();

    /**
     * For big files some cloud services use
     * a chunked uploader, where fixed sized chunks are uploaded
     * until completion. This signal is emitted when a chunk
     * has been uploaded, to inform about upload progress.
     */
    void uploadedChunkReady(int uploadedChunk, int totalChunks);
};

#endif // ABSTRACTSYNCDRIVER_H
