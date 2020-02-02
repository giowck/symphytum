/*
 *  Copyright (c) 2012 Giorgio Wicklein <giowckln@gmail.com>
 */

//-----------------------------------------------------------------------------
// Hearders
//-----------------------------------------------------------------------------

#include "syncengine.h"
#include "abstractsyncdriver.h"
#include "dropboxsyncdriver.h"
#include "megasyncdriver.h"
#include "foldersyncdriver.h"
#include "../settingsmanager.h"
#include "../filemanager.h"
#include "../../utils/definitionholder.h"

#include <QtCore/QCryptographicHash>
#include <QtCore/QFile>
#include <QtCore/QDateTime>
#include <QtGui/QDesktopServices>
#include <QtCore/QDir>


//-----------------------------------------------------------------------------
// Static init
//-----------------------------------------------------------------------------

SyncEngine* SyncEngine::m_instance = nullptr;


//-----------------------------------------------------------------------------
// Public
//-----------------------------------------------------------------------------

SyncEngine& SyncEngine::getInstance()
{
    if (!m_instance)
        m_instance = new SyncEngine();
    return *m_instance;
}

void SyncEngine::destroy()
{
    if (m_instance)
        delete m_instance;
    m_instance = nullptr;
}

AbstractSyncDriver* SyncEngine::createSyncDriver(SyncService service,
                                                 QObject *parent)
{
    AbstractSyncDriver *driver = nullptr;

    switch (service) {
    case SyncEngine::DropboxSync:
        driver = new DropboxSyncDriver(parent);
        break;
    case SyncEngine::MegaSync:
        driver = new MegaSyncDriver(parent);
        break;
    case SyncEngine::FolderSync:
        driver = new FolderSyncDriver(parent);
        break;
    }

    return driver;
}

AbstractSyncDriver* SyncEngine::getCurrentSyncDriver(QObject *parent)
{
    SyncService service = (SyncService)
            m_settingsManager->restoreCurrentCloudSyncService();

    return createSyncDriver(service, parent);
}

void SyncEngine::startSyncCheck()
{
    //update session state
    setSessionState(SyncSession::Accessing);

    //download metadata file
    startMetadataUpdate();
}

void SyncEngine::startMetadataUpdate()
{
    m_currentEngineOperation = NoSpecialOp;
    m_syncDriver->startDownloadRequest(m_metadataFileName, m_metadataFilePath);
}

void SyncEngine::startCloudInitConfig()
{
    //set engine op
    m_currentEngineOperation = CloudInitOp;

    //set that first sync has yet to be done
    m_settingsManager->setCloudSyncFirstTime(true);

    //set local revision
    m_settingsManager->saveCloudSyncRevision(1);

    //reset local metadata file
    MetadataFile metadata;
    initMetadata(metadata);
    if (!writeMetadataFile(metadata))
        return;

    //reset file manager
    FileManager fm(this);
    fm.clearAllLists();

    //save new session key
    m_settingsManager->saveCloudSessionKey(metadata.sessionKey);

    //upload new metadata file (overwrite any existing one)
    m_syncDriver->startUploadRequest(m_metadataFilePath, m_metadataFileName);
}

void SyncEngine::startOpenCloudSession()
{
    m_currentEngineOperation = OpeningCloudSessionOp;
    MetadataFile metadata = readMetadataFile();
    metadata.isOpen = true;
    writeMetadataFile(metadata);

    m_syncDriver->startUploadRequest(m_metadataFilePath, m_metadataFileName);
}

void SyncEngine::startCloseCloudSession()
{
    //update state
    setSessionState(SyncSession::ClosingSession);

    m_currentEngineOperation = ClosingCloudSessionOp;
    MetadataFile metadata = readMetadataFile();

    //if metadata invalid, avoid uploading junk
    if (((int) metadata.revision) == -1) {
        m_currentEngineOperation = NoSpecialOp;
        m_cloudSessionOpened = false;
        //update state
        setSessionState(SyncSession::NoOperation);
        emit syncSessionClosed();
        return;
    }

    metadata.isOpen = false;
    writeMetadataFile(metadata);

    m_syncDriver->startUploadRequest(m_metadataFilePath, m_metadataFileName);
}

SyncEngine::MetadataFile SyncEngine::readMetadataFile()
{
    MetadataFile m;
    m.revision = -1; //invalid

    QFile file(m_metadataFilePath);

    if (!file.open(QIODevice::ReadOnly)) {
        emit syncError(tr("Failed to access metadata file: %1")
                       .arg(file.errorString()));
        return m;
    }

    QString metadataString = QString::fromUtf8(file.readAll().data());
    QStringList metadataList = metadataString.split(';', QString::SkipEmptyParts);
    file.close();

    if (metadataList.size() < 5) {
        emit syncError(tr("Failed to parse metadata file: missing content"));
        return m;
    }

    //metadata file format:
    //sessionKey;revision;isOpen;filesChanged;databaseFormat;

    bool err = false;
    bool ok;
    m.sessionKey = metadataList.at(0);
    m.revision = metadataList.at(1).toULongLong(&ok);
    err = err || (!ok);
    m.isOpen = metadataList.at(2) == "1";
    m.filesChanged = metadataList.at(3) == "1";
    m.databaseFormat = metadataList.at(4).toInt(&ok);
    err = err || (!ok);

    if (err) {
        m.revision = -1;
        emit syncError(tr("Failed to parse metadata file: invalid content"));
        return m;
    }

    return m;
}

bool SyncEngine::writeMetadataFile(const MetadataFile &metadata)
{
    QFile file(m_metadataFilePath);

    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        emit syncError(tr("Failed to access metadata file: %1")
                       .arg(file.errorString()));
        return false;
    }

    //metadata file format:
    //sessionKey;revision;isOpen;filesChanged;databaseFormat;

    QString metadataString;
    metadataString.append(metadata.sessionKey).append(";");
    metadataString.append(QString::number(metadata.revision)).append(";");

    QString isOpen;
    if (metadata.isOpen)
        isOpen = "1";
    else
        isOpen = "0";
    metadataString.append(isOpen).append(";");

    QString filesChanged;
    if (metadata.filesChanged)
        filesChanged = "1";
    else
        filesChanged = "0";
    metadataString.append(filesChanged).append(";");

    metadataString.append(QString::number(metadata.databaseFormat)).append(";");

    int r = file.write(metadataString.toUtf8());
    file.close();

    if (r == -1) {
        emit syncError(tr("Failed to write metadata file: %1")
                       .arg(file.errorString()));
        return false;
    } else {
        return true;
    }
}

QString SyncEngine::getMetadataFileName()
{
    return m_metadataFileName;
}

QString SyncEngine::getMetadataFilePath()
{
    return m_metadataFilePath;
}

bool SyncEngine::localDataChanged()
{
    FileManager fm(this);
    bool a = m_settingsManager->restoreCloudLocalDataChanged();
    bool b = fm.localFileChangesToSync();

    return a || b;
}

void SyncEngine::setLocalDataChanged(bool b)
{
    m_settingsManager->saveCloudLocalDataChanged(b);
}


//-----------------------------------------------------------------------------
// Public slots
//-----------------------------------------------------------------------------

void SyncEngine::reconfigureSyncDriver()
{
    if (m_syncDriver) {
        //remove all connections
        disconnect(m_syncDriver, 0, this, 0);
        disconnect(this, 0, this, 0);

        delete m_syncDriver;
    }

    m_syncDriver = getCurrentSyncDriver(this);
    if (m_syncDriver)
        createSyncConnections();
}


//-----------------------------------------------------------------------------
// Private slots
//-----------------------------------------------------------------------------

void SyncEngine::downloadCompleted(const QString &srcFilePath,
                                   const QString &destFilePath)
{
    Q_UNUSED(srcFilePath);

    if (destFilePath == m_metadataFilePath)
        emit metadataReady();
}

void SyncEngine::downloadFileNotFound(const QString &file)
{
    if (file == m_metadataFileName) {
        emit metadataNotFound();
    } else {
        //nothing for now
    }
}

void SyncEngine::uploadCompleted(const QString &srcFilePath,
                                   const QString &destFilePath)
{
    Q_UNUSED(destFilePath);

    if (srcFilePath == m_metadataFilePath)
        emit metadataUploaded();
}

void SyncEngine::checkMetadataFile()
{
    MetadataFile metadata = readMetadataFile();

    //check if user already logged from elsewhere
    if ((metadata.isOpen && (!SyncSession::IS_ONLINE)) ||
            SyncSession::IS_READ_ONLY) {
        SyncSession::IS_ONLINE = true;
        SyncSession::IS_READ_ONLY = true;
        setSessionState(SyncSession::NoOperation);
        emit clientAlreadyLoggedIn();
        return;
    } else {
        //if session not opened open it
        //once open is complete
        //checkMetadataFile() is auto-called again
        if (!m_cloudSessionOpened) {
            SyncSession::IS_ONLINE = true;
            SyncSession::IS_READ_ONLY = false;
            setSessionState(SyncSession::OpeningSession);
            startOpenCloudSession();
            return;
        }
    }

    //if cloud not init
    if (!m_settingsManager->isCloudSyncInitialized()) {
        SyncSession::IS_ONLINE = true;
        SyncSession::IS_READ_ONLY = false;
        setSessionState(SyncSession::NoOperation);
        emit metadataInitConflict();
        return;
    }

    //check database format version
    if (metadata.databaseFormat > DefinitionHolder::DATABASE_VERSION) {
        SyncSession::IS_ONLINE = true;
        SyncSession::IS_READ_ONLY = false;
        setSessionState(SyncSession::NoOperation);
        emit syncError(tr("Incompatible database format version. "
                          "The database on the cloud service is not compatible "
                          "with this software version. "
                          "Please upgrade %1.").arg(DefinitionHolder::NAME));
        return;
    }

    //check if session key changed
    if (metadata.sessionKey != m_settingsManager->restoreCloudSessionKey()) {
        SyncSession::IS_ONLINE = true;
        SyncSession::IS_READ_ONLY = false;
        setSessionState(SyncSession::NoOperation);
        emit sessionKeyChanged();
        return;
    }

    //check if there's a new revision
    if (metadata.revision > m_settingsManager->restoreCloudSyncRevision()) {
        SyncSession::IS_ONLINE = true;
        SyncSession::IS_READ_ONLY = false;
        setSessionState(SyncSession::NoOperation);

        if (SyncSession::LOCAL_DATA_CHANGED) {
            emit syncRevisionConflict();
        } else {
            emit newSyncRevisionAvailable();
        }
        return;
    }

    //if first sync is yet to be done, sync signal
    if (m_settingsManager->isCloudSyncFirstTime()) {
        SyncSession::IS_ONLINE = true;
        SyncSession::IS_READ_ONLY = false;
        setSessionState(SyncSession::NoOperation);
        emit firstSyncSignal();
        return;
    }

    //if revision is the same but local data where modified
    if (metadata.revision == m_settingsManager->restoreCloudSyncRevision()) {
        if (SyncSession::LOCAL_DATA_CHANGED) {
            SyncSession::IS_ONLINE = true;
            SyncSession::IS_READ_ONLY = false;
            setSessionState(SyncSession::NoOperation);
            emit localDataToSyncSignal();
            return;
        }
    }

    //at this point, nothing to sync
    SyncSession::IS_ONLINE = true;
    SyncSession::IS_READ_ONLY = false;
    setSessionState(SyncSession::NoOperation);
    emit nothingToSync();
}

void SyncEngine::metadataNotFoundSlot()
{
    SyncSession::IS_ONLINE = true;
    SyncSession::IS_READ_ONLY = false;
    setSessionState(SyncSession::NoOperation);
}

void SyncEngine::metadataUploadedSlot()
{
    if (m_currentEngineOperation == CloudInitOp) {
        m_currentEngineOperation = NoSpecialOp;
        emit cloudInitConfigCompleted();
    } else if (m_currentEngineOperation == OpeningCloudSessionOp) {
        m_currentEngineOperation = NoSpecialOp;
        m_cloudSessionOpened = true;

        //update state
        setSessionState(SyncSession::NoOperation);

        emit syncSessionOpened();
    } else if (m_currentEngineOperation == ClosingCloudSessionOp) {
        m_currentEngineOperation = NoSpecialOp;
        m_cloudSessionOpened = false;

        //update state
        setSessionState(SyncSession::NoOperation);

        emit syncSessionClosed();
    } else {
        //nothing
    }
}

void SyncEngine::connectionFailedSlot()
{
    SyncSession::IS_ONLINE = false;
    SyncSession::IS_READ_ONLY = false;
    setSessionState(SyncSession::NoOperation);

    emit connectionFailed();
}

void SyncEngine::authTokenExpiredSlot()
{
    SyncSession::IS_ONLINE = false;
    SyncSession::IS_READ_ONLY = false;
    setSessionState(SyncSession::NoOperation);

    emit authTokenExpired();
}

void SyncEngine::storageQuotaExceededSlot()
{
    SyncSession::IS_ONLINE = false;
    SyncSession::IS_READ_ONLY = false;
    setSessionState(SyncSession::NoOperation);

    emit syncError(tr("Storage quota on cloud service exceeded. "
                      "Please visit %1 to get more space.")
                   .arg(m_syncDriver->getServiceUrl()));
}


//-----------------------------------------------------------------------------
// Private
//-----------------------------------------------------------------------------

SyncEngine::SyncEngine(QObject *parent) :
    QObject(parent), m_settingsManager(0),
    m_currentEngineOperation(NoSpecialOp),
    m_cloudSessionOpened(false)
{
    QString dataDir;
    if (DefinitionHolder::WIN_PORTABLE) {
        dataDir = "portable_data";
    } else {
        dataDir = QStandardPaths::standardLocations(
                    QStandardPaths::DataLocation).at(0);
    }

    m_metadataFileName = "sync.meta";
    m_metadataFilePath = dataDir.append("/");
    m_metadataFilePath.append(m_metadataFileName);
    if (!QDir(dataDir).exists()) {
        QDir::current().mkpath(dataDir);
    }

    //settings manager
    m_settingsManager = new SettingsManager;

    //init sync driver
    m_syncDriver = getCurrentSyncDriver(this);
    if (m_syncDriver)
        createSyncConnections();
}

SyncEngine::~SyncEngine()
{
    if (m_settingsManager)
        delete m_settingsManager;
}

void SyncEngine::createSyncConnections()
{
    connect(m_syncDriver, SIGNAL(downloadReady(QString,QString)),
            this, SLOT(downloadCompleted(QString,QString)));
    connect(m_syncDriver, SIGNAL(downloadFileNotFound(QString)),
            this, SLOT(downloadFileNotFound(QString)));
    connect(m_syncDriver, SIGNAL(uploadReady(QString,QString)),
            this, SLOT(uploadCompleted(QString,QString)));
    connect(m_syncDriver, SIGNAL(connectionFailed()),
            this, SLOT(connectionFailedSlot()));
    connect(m_syncDriver, SIGNAL(authTokenExpired()),
            this, SLOT(authTokenExpiredSlot()));
    connect(m_syncDriver, SIGNAL(storageQuotaExceeded()),
            this, SLOT(storageQuotaExceededSlot()));
    connect(m_syncDriver, SIGNAL(errorSignal(QString)),
            this, SIGNAL(syncError(QString)));

    connect(this, SIGNAL(metadataReady()),
            this, SLOT(checkMetadataFile()));
    connect(this, SIGNAL(metadataNotFound()),
            this, SLOT(metadataNotFoundSlot()));
    connect(this, SIGNAL(metadataUploaded()),
            this, SLOT(metadataUploadedSlot()));
    connect(this, SIGNAL(syncSessionOpened()),
            this, SLOT(checkMetadataFile()));
}

void SyncEngine::setSessionState(SyncSession::SessionState state)
{
    SyncSession::CURRENT_STATE = state;
    emit sessionChanged();
}

void SyncEngine::initMetadata(MetadataFile &metadata)
{
    //create session key
    QByteArray dateArray = QDateTime::currentDateTime().toString().toUtf8();
    QByteArray hash = QCryptographicHash::hash(dateArray,
                                               QCryptographicHash::Md5);
    QString sessionKey(hash.toHex());

    metadata.sessionKey = sessionKey;
    metadata.revision = 1;
    metadata.isOpen = 1;
    metadata.filesChanged = 1;
    metadata.databaseFormat = DefinitionHolder::DATABASE_VERSION;
}
