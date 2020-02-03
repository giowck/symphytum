/**
  * \class SyncEngine
  * \brief Engine for sync with cloud services.
  *        This class handles all operations to sync with a service,
  *        methods for metadata handling and more.
  *        This class also exposes convenience methods to make the usage of
  *        AbstractSyncDriver more confortable.
  * \author Giorgio Wicklein - GIOWISYS Software
  * \date 28/08/2012
  */

#ifndef SYNCENGINE_H
#define SYNCENGINE_H


//-----------------------------------------------------------------------------
// Headers
//-----------------------------------------------------------------------------

#include <QtCore/QObject>

#include "syncsession.h"


//-----------------------------------------------------------------------------
// Forward declarations
//-----------------------------------------------------------------------------

class AbstractSyncDriver;
class SettingsManager;


//-----------------------------------------------------------------------------
// SyncEngine
//-----------------------------------------------------------------------------

class SyncEngine : public QObject
{
    Q_OBJECT

public:
    /** Enum of supported sync services */
    enum SyncService {
        DropboxSync = 0,
        MegaSync = 1,
        FolderSync = 2
    };

    /** Structure of the metadata file */
    struct MetadataFile {
        QString sessionKey; /**< The sync session key */
        quint64 revision; /**< The revision of the database */
        bool isOpen; /**< Whether a client has an open cloud sync session */
        bool filesChanged; /**< Whether files were changed (added/deleted) */
        int databaseFormat; /**< The database format version */
    };

    static SyncEngine& getInstance();
    static void destroy();

    /** Create the appropriate sync driver for the specified sync service */
    static AbstractSyncDriver* createSyncDriver(SyncService service,
                                                QObject *parent = nullptr);

    /**
     * Create and return a sync driver instance for
     * the current configured sync service
     */
    AbstractSyncDriver* getCurrentSyncDriver(QObject *parent = nullptr);

    /**
     * This method starts a sync request to check if there are changes
     * available to sync (new revision) by downloading meta data from
     * the cloud service. Emits respective signals.
     */
    void startSyncCheck();

    /**
     * This method downloads the metadata file from the cloud service
     * to allow up-to-date metadata queries. Emits metadataReady()
     * on completion or other respective signals (errors).
     * The signal metadataReady() is automatically connected
     * to checkMetadataFile().
     */
    void startMetadataUpdate();

    /**
     * Resets all existing cloud configuration and metadata
     * in order to initialize a new sync setup. All existing
     * configs and metadata on cloud and local are deleted.
     * A new metadata file and config is created to allow
     * syncing as first time.
     */
    void startCloudInitConfig();

    /**
     * Open sync session to the cloud service,
     * syncSessionOpened() is emitted on completion
     */
    void startOpenCloudSession();

    /**
     * Close sync session with the cloud service,
     * syncSessionClosed() is emitted on completion
     */
    void startCloseCloudSession();

    /**
     * Get metadata file's content
     * @return MetadataFile - structure with metadata file contents
     */
    MetadataFile readMetadataFile();

    /**
     * Set metadata file's content
     * @param metadata - MetadataFile structure with metadata file contents
     * @return bool - whether metdata file was successfuly written
     */
    bool writeMetadataFile(const MetadataFile &metadata);

    /** Get the file name of the metadata file */
    QString getMetadataFileName();

    /** Get the file path of the metadata file */
    QString getMetadataFilePath();

    /** Return whether local data has been modified or not */
    bool localDataChanged();

    /** Set local data modified property */
    void setLocalDataChanged(bool b);

signals:
    /** Emitted when SyncEngine changes properties of SyncSession */
    void sessionChanged();

    /**
     * Emitted when a new revision (update) of the database
     * was found on the cloud service. Usally emited by startSyncCheck().
     */
    void newSyncRevisionAvailable();

    /** Emitted if there is a sync revision conflict */
    void syncRevisionConflict();

    /** Emitted when metadata has been download from cloud service */
    void metadataReady();

    /** Emitted if the configured cloud service has not a metadata file */
    void metadataNotFound();

    /** Emitted if metadata already present on cloud but first sync never done */
    void metadataInitConflict();

    /** Emitted when local metadta file has been uploaded to cloud */
    void metadataUploaded();

    /** Emitted if there are no changes that need to be synced */
    void nothingToSync();

    /** Emitted if connection to cloud service failed */
    void connectionFailed();

    /** Emitted on auth token expiration */
    void authTokenExpired();

    /** Emitted on sync error */
    void syncError(const QString &message);

    /** Emitted when startCloudInitConfig completed successfully */
    void cloudInitConfigCompleted();

    /** Emitted if another client is already connected to the cloud service */
    void clientAlreadyLoggedIn();

    /** Emitted if the sync session key has changed */
    void sessionKeyChanged();

    /** Emitted if the first sync is yet to be done */
    void firstSyncSignal();

    /** Emitted if there are local changes to sync */
    void localDataToSyncSignal();

    /** Emitted on sync session open */
    void syncSessionOpened();

    /** Emitted on sync session close */
    void syncSessionClosed();

public slots:
    /** Call this slot if the configuration of the cloud service changed */
    void reconfigureSyncDriver();

private slots:
    /** Called when a file has been downloaded */
    void downloadCompleted(const QString &srcFilePath,
                           const QString &destFilePath);

    /** Called when file to download was not found */
    void downloadFileNotFound(const QString &file);

    /** Called when a file has been uploaded */
    void uploadCompleted(const QString &srcFilePath,
                         const QString &destFilePath);

    /**
     * This slots checks the metadata file and reports needed
     * actions using corresponding signals
     */
    void checkMetadataFile();

    /** Handle sync check response for missing metadata file on cloud service */
    void metadataNotFoundSlot();

    /** Called when metadata file was uploaded */
    void metadataUploadedSlot();

    /** Handle offline state */
    void connectionFailedSlot();

    /** Handle invalid or expired token */
    void authTokenExpiredSlot();

    /** Handle user over quota */
    void storageQuotaExceededSlot();

private:
    SyncEngine(QObject *parent = nullptr);
    SyncEngine(const SyncEngine&) : QObject(nullptr) {}
    ~SyncEngine();

    /** Enum of special engine operations */
    enum SyncEngineOperation {
        NoSpecialOp, /**< No special operation going on that is worth to mark */
        OpeningCloudSessionOp, /**< A session is being opened to cloud service */
        ClosingCloudSessionOp, /**< A session with cloud service is being closed  */
        CloudInitOp /**< Cloud service is being initialized */
    };

    void createSyncConnections();

    /** Set session state and emit session changed signal */
    void setSessionState(SyncSession::SessionState state);

    /** Init the given metadata with new values for a new sync session */
    void initMetadata(MetadataFile &metadata);

    QString m_metadataFileName; /**< The name of the local metadata file */
    QString m_metadataFilePath; /**< The path to the local metadata file */
    AbstractSyncDriver *m_syncDriver;
    SettingsManager *m_settingsManager;
    SyncEngineOperation m_currentEngineOperation; /**< If a special engine op is going on */
    bool m_cloudSessionOpened; /**< Whether the session has been opened to the cloud */
    static SyncEngine *m_instance;
};

#endif // SYNCENGINE_H
