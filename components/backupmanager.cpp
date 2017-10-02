/*
 *  Copyright (c) 2012 Giorgio Wicklein <giowckln@gmail.com>
 */

//-----------------------------------------------------------------------------
// Hearders
//-----------------------------------------------------------------------------

#include "backupmanager.h"
#include "filemanager.h"
#include "databasemanager.h"
#include "metadataengine.h"
#include "../utils/definitionholder.h"

#include <QtCore/QThread>
#include <QtCore/QDataStream>
#include <QtCore/QFile>
#include <QtCore/QMap>
#include <QtCore/QStringList>


//-----------------------------------------------------------------------------
// BackupTask
//-----------------------------------------------------------------------------

BackupTask::BackupTask(const QString &filesDir,
                       const QString &databasePath,
                       QObject *parent)
    : QObject(parent), m_currentOp(ExportOp)
{
    m_filesDir = filesDir;
    m_dbPath = databasePath;

    m_magicNumber = 0x53594D50; //SYMP
    m_fileBufSize = 5242880;
}

BackupTask::~BackupTask()
{
}

void BackupTask::configureTask(const QString &path, BackupOp operation)
{
    m_path = path;
    m_currentOp = operation;
}

void BackupTask::startBackupTask()
{
    bool error = false;
    QString errMessage;

    switch (m_currentOp) {
    case ExportOp:
        error = !fullExport(m_path, errMessage);
        break;
    case ImportOp:
        error = !fullImport(m_path, errMessage);
        break;
    default:
        break;
    }

    if (error) {
        emit errorSignal(errMessage);
        return;
    }

    emit finishedSignal(m_currentOp);
}

bool BackupTask::fullExport(const QString &destPath,
                            QString &errorMessage)
{
    int dbVersion = DefinitionHolder::DATABASE_VERSION;
    qint64 metadataOffset = 0;
    QMap<qint64, QString> fileOffset;
    QStringList contentFileList = MetadataEngine::getInstance()
            .getAllContentFiles().values();

    //calc progress
    int progress = 0;
    int totalSteps = 0;
    totalSteps = 1 + contentFileList.size();
    emit progressSignal(progress, totalSteps);

    QFile destFile(destPath);
    if (!destFile.open(QIODevice::WriteOnly)) {
        errorMessage = tr("Failed to open create file %1: %2")
                .arg(destPath).arg(destFile.errorString());
        return false;
    }

    QDataStream out(&destFile);
    out << m_magicNumber;
    out << dbVersion;

    int placeHolderOffset = destFile.pos();
    out << metadataOffset; //place holder

    //write database file
    fileOffset.insert(destFile.pos(), "database");
    QFile dbFile(m_dbPath);
    if (!dbFile.open(QIODevice::ReadOnly)) {
        errorMessage = tr("Failed to open file %1: %2")
                .arg(m_dbPath).arg(dbFile.errorString());
        return false;
    }
    while(!dbFile.atEnd()) {
        destFile.write(dbFile.read(m_fileBufSize));
    }
    dbFile.close();

    //update progress
    emit progressSignal(++progress, totalSteps);

    //write content files
    foreach (QString s, contentFileList) {
        fileOffset.insert(destFile.pos(), s);
        QFile file(m_filesDir + s);
        if (!file.open(QIODevice::ReadOnly)) {
            errorMessage = tr("Failed to open file %1: %2")
                    .arg(m_filesDir + s).arg(file.errorString());
            return false;
        }
        while(!file.atEnd()) {
            destFile.write(file.read(m_fileBufSize));
        }
        file.close();

        //update progress
        emit progressSignal(++progress, totalSteps);
    }

    //write metadata
    QString metadataString;
    QMap<qint64, QString>::const_iterator i = fileOffset.constBegin();
    while (i != fileOffset.constEnd()) {
        metadataString.append(QString::number(i.key()));
        metadataString.append(":");
        metadataString.append(i.value());
        metadataString.append(";");
        i++;
    }
    metadataOffset = destFile.pos();
    out << metadataString;

    //fix placeholder for metadata
    destFile.seek(placeHolderOffset);
    out << metadataOffset;

    destFile.close();
    return true;
}

bool BackupTask::fullImport(const QString &filePath,
                            QString &errorMessage)
{
    QMap<qint64, QString> fileOffset;
    FileManager fileManager(this);
    QStringList filesToRemove = fileManager.getAllLocalFiles();

    QFile srcFile(filePath);
    if (!srcFile.open(QIODevice::ReadOnly)) {
        errorMessage = tr("Failed to open file %1: %2")
                .arg(filePath).arg(srcFile.errorString());
        return false;
    }

    //check magic and db version
    int dbVersion;
    int magic;
    QDataStream in(&srcFile);
    in >> magic;
    in >> dbVersion;
    if (magic != m_magicNumber) {
        errorMessage = tr("The selected file is not a valid backup file!");
        return false;
    }
    if (dbVersion > DefinitionHolder::DATABASE_VERSION) {
        errorMessage = tr("The selected backup file is not compatible with "
                          "this software version. "
                          "Please upgrade to a newer version and then try again.");
        return false;
    }
    //in case the database version is old
    //on restart it will be upgraded by DatabaseManager

    //get metadata offset
    qint64 metadatOffset = 0;
    in >> metadatOffset;

    //extract metadata
    QString metadataString;
    QStringList metaparse;
    srcFile.seek(metadatOffset);
    in >> metadataString;
    metaparse = metadataString.split(";", QString::SkipEmptyParts);
    foreach (QString s, metaparse) {
        QStringList l = s.split(":", QString::SkipEmptyParts);
        if (l.size() == 2) {
            fileOffset.insert(l.at(0).toLongLong(), l.at(1));
        }
    }

    //calc progress
    int progress = 0;
    int totalSteps = 0;
    totalSteps = 1 + filesToRemove.size() + fileOffset.size();
    emit progressSignal(progress, totalSteps);

    //remove database file
    DatabaseManager::getInstance().destroy(); //close db
    if (!QFile::remove(m_dbPath)) {
        errorMessage = tr("Failed to remove current database file!");
        return false;
    }
    //update progress
    emit progressSignal(++progress, totalSteps);

    //remove content files
    foreach (QString s, filesToRemove) {
        if (!QFile::remove(m_filesDir + s)) {
            errorMessage = tr("Failed to remove file %1!").arg(m_filesDir + s);
            return false;
        }

        //update progress
        emit progressSignal(++progress, totalSteps);
    }

    //write files
    QMap<qint64, QString>::const_iterator iter = fileOffset.constBegin();
    while (iter != fileOffset.constEnd()) {
        //seek
        srcFile.seek(iter.key());

        //calc file size
        qint64 fileSize = 0;
        if ((iter+1) != fileOffset.constEnd()) {
            fileSize = (iter+1).key() - iter.key(); ;
        } else {
            fileSize = metadatOffset - iter.key();
        }

        //write
        QString s = iter.value();
        QString destFilePath;
        if (s == "database") {
            destFilePath = m_dbPath;
        } else {
            destFilePath = m_filesDir + s;
        }
        QFile file(destFilePath);
        if (!file.open(QIODevice::WriteOnly)) {
            errorMessage = tr("Failed to open file %1: %2")
                    .arg(m_filesDir + s).arg(file.errorString());
            return false;
        }

        qint64 written = 0;
        while ((written < fileSize) && (!srcFile.atEnd())) {
            qint64 bytesLeft = fileSize - written;
            if (bytesLeft > m_fileBufSize) {
                file.write(srcFile.read(m_fileBufSize));
                written += m_fileBufSize;
            } else {
                file.write(srcFile.read(bytesLeft));
                written += bytesLeft;
            }
        }

        file.close();
        iter++;

        //update progress
        emit progressSignal(++progress, totalSteps);
    }

    srcFile.close();
    return true;
}


//-----------------------------------------------------------------------------
// Public
//-----------------------------------------------------------------------------

BackupManager::BackupManager(QObject *parent) :
    QObject(parent), m_backupTaskThread(0)
{
    FileManager fm(this);
    m_fileDirPath = fm.getFilesDirectory();
    m_databasePath = DatabaseManager::getInstance().getDatabasePath();
}

BackupManager::~BackupManager()
{
    stopBackupTask();

    if (m_backupTaskThread)
        delete m_backupTaskThread;
}

void BackupManager::startExport(const QString &destFilePath)
{
    //create backup task thread
    m_backupTaskThread = new QThread;
    BackupTask *backupTask = new BackupTask(m_fileDirPath, m_databasePath);

    //config task
    backupTask->configureTask(destFilePath);

    backupTask->moveToThread(m_backupTaskThread);
    createBackupThreadConnections(m_backupTaskThread, backupTask);

    m_backupTaskThread->start();
}

void BackupManager::startImport(const QString &importFilePath)
{
    //create backup task thread
    m_backupTaskThread = new QThread;
    BackupTask *backupTask = new BackupTask(m_fileDirPath, m_databasePath);

    //config task
    backupTask->configureTask(importFilePath, BackupTask::ImportOp);

    backupTask->moveToThread(m_backupTaskThread);
    createBackupThreadConnections(m_backupTaskThread, backupTask);

    m_backupTaskThread->start();
}


//-----------------------------------------------------------------------------
// Public slots
//-----------------------------------------------------------------------------

void BackupManager::stopBackupTask()
{
    if (m_backupTaskThread) {
        if (m_backupTaskThread->isRunning()) {
            m_backupTaskThread->terminate();
            m_backupTaskThread->wait();
        }
    }
}


//-----------------------------------------------------------------------------
// Private slots
//-----------------------------------------------------------------------------

void BackupManager::backupTaskErrorSlot(const QString &message)
{
    m_backupTaskThread = 0;
    emit backupTaskFailed(message);
}

void BackupManager::backupTaskFinishedSlot(int op)
{
    m_backupTaskThread = 0;

    switch (op) {
    case BackupTask::ExportOp:
        emit exportCompleted();
        break;
    case BackupTask::ImportOp:
        emit importCompleted();
        break;
    }
}


//-----------------------------------------------------------------------------
// Private
//-----------------------------------------------------------------------------

void BackupManager::createBackupThreadConnections(QThread *thread,
                                                  BackupTask *backupTask)
{
    //thread start and stop
    connect(thread, SIGNAL(started()),
            backupTask, SLOT(startBackupTask()));
    connect(backupTask, SIGNAL(finishedSignal(int)),
            thread, SLOT(quit()));
    connect(backupTask, SIGNAL(errorSignal(QString)),
            thread, SLOT(quit()));

    //backupTask delete
    connect(backupTask, SIGNAL(finishedSignal(int)),
            backupTask, SLOT(deleteLater()));
    connect(backupTask, SIGNAL(errorSignal(QString)),
            backupTask, SLOT(deleteLater()));

    //thread delete
    connect(thread, SIGNAL(finished()),
            thread, SLOT(deleteLater()));

    //backupTask signals
    connect(backupTask, SIGNAL(errorSignal(QString)),
            this, SLOT(backupTaskErrorSlot(QString)));
    connect(backupTask, SIGNAL(finishedSignal(int)),
            this, SLOT(backupTaskFinishedSlot(int)));
    connect(backupTask, SIGNAL(progressSignal(int,int)),
            this, SIGNAL(progressSignal(int,int)));
}
