/*
 *  Copyright (c) 2012 Giorgio Wicklein <giowckln@gmail.com>
 */

//-----------------------------------------------------------------------------
// Hearders
//-----------------------------------------------------------------------------

#include "filemanager.h"
#include "../components/metadataengine.h"
#include "../components/sync_framework/syncsession.h"
#include "../components/settingsmanager.h"
#include "../components/databasemanager.h"

#include <QtCore/QStringList>
#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QThread>
#include <QtWidgets/QMessageBox>
#include <QtCore/QDateTime>
#include <QtCore/QCryptographicHash>
#include <QtGui/QDesktopServices>
#include <QtCore/QUrl>
#include <QtSql/QSqlQuery>
#include <QtCore/QVariant>


//-----------------------------------------------------------------------------
// FileTask
//-----------------------------------------------------------------------------

FileTask::FileTask(const QString &filesDir, QObject *parent)
    : QObject(parent), m_currentOp(CopyOp)
{
    m_filesDir = filesDir;
}

FileTask::~FileTask()
{
}

void FileTask::configureTask(const QString &srcfileName,
                             const QString &destFileName,
                             FileOp operation)
{
    m_srcFileName = srcfileName;
    m_destFileName = destFileName;
    m_currentOp = operation;
}

void FileTask::startFileOp()
{
    bool error = false;
    QString errMessage;
    QFile src(m_srcFileName);

    switch (m_currentOp) {
    case CopyOp:
        if (!src.copy(m_filesDir + m_destFileName)) {
            error = true;
            errMessage = tr("Failed to copy %1 to %2: %3")
                    .arg(m_srcFileName).arg(m_filesDir + m_destFileName)
                    .arg(src.errorString());
        }
        break;
    case RemoveOp:
        if (!QFile::remove(m_filesDir + m_srcFileName)) {
            error = true;
            errMessage = tr("Failed to remove %1: %2")
                    .arg(m_filesDir + m_srcFileName).arg(src.errorString());
        }
        break;
    default:
        break;
    }

    if (error) {
        emit errorSignal(errMessage);
        return;
    }

    emit finishedSignal(m_srcFileName, m_destFileName, m_currentOp);
}


//-----------------------------------------------------------------------------
// Public
//-----------------------------------------------------------------------------

FileManager::FileManager(QObject *parent) :
    QObject(parent), m_fileOpThread(nullptr)
{
    QString dataDir = QFileInfo(DatabaseManager::getInstance()
                                .getDatabasePath()).dir().path();
#ifdef Q_OS_WIN
    dataDir.replace("\\", "/");
#endif // Q_OS_WIN
    if (!QDir(dataDir).exists()) {
        QDir::current().mkpath(dataDir);
    }
    m_fileDirPath = dataDir + "/files/";

    //create file dir if not present
    QDir fileDir(m_fileDirPath);
    if (!fileDir.exists()) {
        QDir::current().mkpath(m_fileDirPath);
    }

    m_metadataEngine = &MetadataEngine::getInstance();
    m_settingsManager = new SettingsManager;
}

FileManager::~FileManager()
{
    stopFileOp();

    if (m_fileOpThread)
        delete m_fileOpThread;

    if (m_settingsManager)
        delete m_settingsManager;
}

bool FileManager::localFileChangesToSync()
{
    bool b = !fileListToDownload().isEmpty();
    b = b || (!fileListToRemove().isEmpty());
    b = b || (!fileListToUpload().isEmpty());

    return b;
}

QString FileManager::getFilesDirectory()
{
    return m_fileDirPath;
}

QStringList FileManager::fileListToDownload()
{
    QStringList downloadFileList;
    QStringList localFileList = getAllLocalFiles();
    QStringList databaseFileList =
            m_metadataEngine->getAllContentFiles().values();

    foreach (QString s, databaseFileList) {
        if (!localFileList.contains(s))
            downloadFileList.append(s);
    }

    return downloadFileList;
}

QStringList FileManager::fullFileList()
{
    QHash<int,QString> map = m_metadataEngine->getAllContentFiles();
    return map.values();
}

QStringList FileManager::fileListToUpload()
{
    QStringList uploadList = m_settingsManager->restoreToUploadList();
    QHash<QString,QDateTime> watchList = m_settingsManager->restoreToWatchList();

    //check watched files for modifications, if yes, add to upload list
    QHash<QString, QDateTime>::const_iterator i = watchList.constBegin();
    while (i != watchList.constEnd()) {
        QFileInfo info(m_fileDirPath + i.key());
        if (info.lastModified() != i.value()) {
            if (!uploadList.contains(i.key()))
                uploadList.append(i.key());
        }
        ++i;
    }

    m_settingsManager->saveToUploadList(uploadList);
    watchList.clear();
    m_settingsManager->saveToWatchList(watchList);

    return uploadList;
}

QStringList FileManager::fileListToRemove()
{
    return m_settingsManager->restoreToDeleteList();
}

QStringList FileManager::unneededLocalFileList()
{
    QStringList unneededFileList;
    QStringList localFileList = getAllLocalFiles();
    QStringList databaseFileList =
            m_metadataEngine->getAllContentFiles().values();

    foreach (QString s, localFileList) {
        if (!databaseFileList.contains(s))
            unneededFileList.append(s);
    }

    return unneededFileList;
}

QStringList FileManager::orphanDatabaseFileList()
{
    QStringList orphans;
    QStringList filesInDatabase;

    //get all file ids in the database's file table
    QList<int> dbFileIdsInt = m_metadataEngine->getAllContentFiles().keys();
    foreach (int t, dbFileIdsInt) {
        filesInDatabase.append(QString::number(t));
    }

    //get all file ids that are actively used in records
    QStringList filesInRecords;
    QStringList collectionIds = m_metadataEngine->getAllCollections();

    foreach(QString idString, collectionIds) {
        int collectionId = idString.toInt();
        int fieldCount = m_metadataEngine->getFieldCount(collectionId);
        for (int i = 1; i < fieldCount; i++) { //1 because _id is 0
            switch (m_metadataEngine->getFieldType(i, collectionId)) {
            case MetadataEngine::ImageType:
            {
                //extract all ids
                QSqlQuery query(DatabaseManager::getInstance().getDatabase());
                QString sql(QString("SELECT \"%1\" FROM \"%2\"")
                            .arg(QString::number(i))
                            .arg(m_metadataEngine->getTableName(collectionId)));
                query.exec(sql);

                while(query.next()) {
                    QString s = query.value(0).toString();
                    if ((!s.isEmpty()) && (s != "0"))
                        filesInRecords.append(s);
                }
            }
                break;
            case MetadataEngine::FilesType:
            {
                //extract all ids
                QSqlQuery query(DatabaseManager::getInstance().getDatabase());
                QString sql(QString("SELECT \"%1\" FROM \"%2\"")
                            .arg(QString::number(i))
                            .arg(m_metadataEngine->getTableName(collectionId)));
                query.exec(sql);

                while(query.next()) {
                    QStringList ids = query.value(0).toString().split(",", QString::SkipEmptyParts);
                    foreach (QString xs, ids) {
                        filesInRecords.append(xs);
                    }
                }
            }
                break;
            default:
                //nothing
                break;
            }
        }
    }

    //spot orphans
    foreach (QString id, filesInDatabase) {
        if (!filesInRecords.contains(id))
            orphans.append(id);
    }

    return orphans;
}

void FileManager::removeFileFromUploadList(const QString &file)
{
    QStringList list = fileListToUpload();
    list.removeOne(file);
    m_settingsManager->saveToUploadList(list);
}

void FileManager::removeFileFromRemoveList(const QString &file)
{
    QStringList list = fileListToRemove();
    list.removeOne(file);
    m_settingsManager->saveToDeleteList(list);
}

void FileManager::clearAllLists()
{
    m_settingsManager->saveToDeleteList(QStringList());
    m_settingsManager->saveToUploadList(QStringList());
    m_settingsManager->saveToWatchList(QHash<QString,QDateTime>());
}

void FileManager::startAddFile(const QString &file)
{
    //create file op thread
    m_fileOpThread = new QThread;
    FileTask *fileTask = new FileTask(m_fileDirPath); //parent is null because moved later to thread

    //create dest file name
    QByteArray dateArray = QDateTime::currentDateTime().toString().toUtf8();
    QByteArray nameArray = file.toUtf8();
    QByteArray hash = QCryptographicHash::hash(dateArray.append(nameArray),
                                               QCryptographicHash::Md5);
    QFileInfo i(file);
    QString suffix = i.suffix();
    QString extension;
    if (!suffix.isEmpty())
        extension = QString(".").append(suffix);
    QString dest(hash.toHex());
    dest.append(extension);

    fileTask->configureTask(file, dest, FileTask::CopyOp);

    fileTask->moveToThread(m_fileOpThread);
    createFileThreadConnections(m_fileOpThread, fileTask);

    m_fileOpThread->start();
}

void FileManager::startRemoveFile(const QString &file)
{
    //create file op thread
    m_fileOpThread = new QThread;
    FileTask *fileTask = new FileTask(m_fileDirPath);

    //config task
    fileTask->configureTask(file);

    fileTask->moveToThread(m_fileOpThread);
    createFileThreadConnections(m_fileOpThread, fileTask);

    m_fileOpThread->start();
}

void FileManager::openContentFile(const QString &file)
{
    QFileInfo f(m_fileDirPath + file);
    QDesktopServices::openUrl(QUrl::fromLocalFile(f.absoluteFilePath()));

    //if sync is enabled, add to a list of files that should be watched for
    //changes on next sync
    if (SyncSession::IS_ENABLED) {
        addFileToWatchList(file);
    }
}

void FileManager::removeAllFiles()
{
    QStringList files = getAllLocalFiles();

    foreach (QString file, files) {
        QFile::remove(m_fileDirPath + file);
    }
}

QStringList FileManager::getAllLocalFiles()
{
    //get all files that are in the local files directory
    QDir filesDir(m_fileDirPath);
    return filesDir.entryList(QDir::AllEntries | QDir::NoDotAndDotDot);
}

void FileManager::removeFileMetadata(const int fileId)
{
    QString hashName, fileName, origDirPath;
    QDateTime dateTime;

    m_metadataEngine->getContentFile(fileId,
                                     fileName,
                                     hashName,
                                     dateTime,
                                     origDirPath);

    if (SyncSession::IS_ENABLED) {
        addFileToDeleteList(hashName);
        removeFileFromUploadList(hashName);
    }

    m_metadataEngine->removeContentFile(fileId);
}


//-----------------------------------------------------------------------------
// Public slots
//-----------------------------------------------------------------------------

void FileManager::stopFileOp()
{
    if (m_fileOpThread) {
        if (m_fileOpThread->isRunning()) {
            m_fileOpThread->terminate();
            m_fileOpThread->wait();
        }
    }
}


//-----------------------------------------------------------------------------
// Private slots
//-----------------------------------------------------------------------------

void FileManager::fileOperationErrorSlot(const QString &message)
{
    m_fileOpThread = nullptr;
    QMessageBox::critical(nullptr, tr("File Error"), message);

    emit fileOpFailed();
}

void FileManager::fileOperationFinishedSlot(const QString &srcFileName,
                                            const QString &destFileName,
                                            int op)
{
    m_fileOpThread = nullptr;

    switch (op) {
    case FileTask::CopyOp:
    {
        QString fileName;
        QFileInfo info(srcFileName);
        fileName = info.baseName();
        if (!info.suffix().isEmpty())
            fileName.append("." + info.completeSuffix());

        m_metadataEngine->addContentFile(fileName, destFileName,
                                         info.absoluteDir().absolutePath());
        if (SyncSession::IS_ENABLED) {
            addFileToUploadList(destFileName);
        }
        emit addFileCompletedSignal(destFileName);
    }
        break;
    case FileTask::RemoveOp:
        m_metadataEngine->removeContentFile(
                    m_metadataEngine->getContentFileId(srcFileName));
        if (SyncSession::IS_ENABLED) {
            addFileToDeleteList(srcFileName);
            removeFileFromUploadList(srcFileName);
        }
        emit removeFileCompletedSignal(srcFileName);
        break;
    }
}


//-----------------------------------------------------------------------------
// Private
//-----------------------------------------------------------------------------

void FileManager::createFileThreadConnections(QThread *thread,
                                              FileTask *fileTask)
{

    //thread start and stop
    connect(thread, SIGNAL(started()),
            fileTask, SLOT(startFileOp()));
    connect(fileTask, SIGNAL(finishedSignal(QString,QString,int)),
            thread, SLOT(quit()));
    connect(fileTask, SIGNAL(errorSignal(QString)),
            thread, SLOT(quit()));

    //fileTask delete
    connect(fileTask, SIGNAL(finishedSignal(QString,QString,int)),
            fileTask, SLOT(deleteLater()));
    connect(fileTask, SIGNAL(errorSignal(QString)),
            fileTask, SLOT(deleteLater()));

    //thread delete
    connect(thread, SIGNAL(finished()),
            thread, SLOT(deleteLater()));

    //fileTask signals
    connect(fileTask, SIGNAL(errorSignal(QString)),
            this, SLOT(fileOperationErrorSlot(QString)));
    connect(fileTask, SIGNAL(finishedSignal(QString,QString,int)),
            this, SLOT(fileOperationFinishedSlot(QString,QString,int)));
}

void FileManager::addFileToUploadList(const QString &file)
{
    QStringList list = fileListToUpload();
    if (!list.contains(file)) {
        list.append(file);
        m_settingsManager->saveToUploadList(list);
    }

    //set local data changed
    SyncSession::LOCAL_DATA_CHANGED = true;
}

void FileManager::addFileToDeleteList(const QString &file)
{
    QStringList list = fileListToRemove();
    if (!list.contains(file)) {
        list.append(file);
        m_settingsManager->saveToDeleteList(list);
    }

    //set local data changed
    SyncSession::LOCAL_DATA_CHANGED = true;
}

void FileManager::addFileToWatchList(const QString &file)
{
    QFileInfo info(m_fileDirPath + file);
    QHash<QString,QDateTime> map = m_settingsManager->restoreToWatchList();
    map.insert(file, info.lastModified());
    m_settingsManager->saveToWatchList(map);

    //set local data changed
    SyncSession::LOCAL_DATA_CHANGED = true;
}
