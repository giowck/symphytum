/*
 *  Copyright (c) 2020 Oirio Joshi (joshirio)
 */

//-----------------------------------------------------------------------------
// Hearders
//-----------------------------------------------------------------------------

#include "foldersyncdriver.h"
#include "../settingsmanager.h"
#include "../../utils/definitionholder.h"

#include <QtCore/QFileInfo>
#include <QtCore/QDir>
#include <QtWidgets/QApplication>


//-----------------------------------------------------------------------------
// FolderSyncTask
//-----------------------------------------------------------------------------

FolderSyncTask::FolderSyncTask(QObject *parent)
    : QObject(parent), m_currentOp(CopyOp)
{
}

FolderSyncTask::~FolderSyncTask()
{
}

void FolderSyncTask::configureTask(const QString &srcfileName,
                                   const QString &destFileName,
                                   FileOp operation)
{
    m_srcFileName = srcfileName;
    m_destFileName = destFileName;
    m_currentOp = operation;
}

void FolderSyncTask::startFileOp()
{
    bool error = false;
    QString errMessage;
    QFile src(m_srcFileName);
    QFile dest(m_destFileName);

    switch (m_currentOp) {
    case CopyOp:
        if (dest.exists()) {
            error = !dest.remove(m_destFileName);
            if (error) {
                errMessage = tr("Failed to copy %1 to %2: %3 because removing failed")
                        .arg(m_srcFileName).arg(m_destFileName)
                        .arg(src.errorString());
                break;
            }
        }
        if (!src.copy(m_destFileName)) {
            error = true;
            errMessage = tr("Failed to copy %1 to %2: %3")
                    .arg(m_srcFileName).arg(m_destFileName)
                    .arg(src.errorString());
        }
        break;
    case RemoveOp:
        if (!QFile::remove(m_srcFileName)) {
            error = true;
            errMessage = tr("Failed to remove %1: %2")
                    .arg(m_srcFileName).arg(src.errorString());
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

FolderSyncDriver::FolderSyncDriver(QObject *parent) :
    AbstractSyncDriver(parent),
    m_currentRequest(NoRequest),
    m_fileOpThread(nullptr)
{
    m_folderPath = "/invalid";
}

FolderSyncDriver::~FolderSyncDriver()
{
    stopFileOp();

    if (m_fileOpThread)
        delete m_fileOpThread;
}

void FolderSyncDriver::startAuthenticationRequest(const QStringList &args)
{
    m_currentRequest = InitRequest;

    //folder path
    if (args.size()) {
        m_folderPath = args.at(0);
    }

    startRequest();
}

void FolderSyncDriver::startAuthenticationValidationRequest(const QString &authToken)
{
    m_currentRequest = AuthValidationRequest;
    m_requestArgs.append(authToken);
    startRequest();
}

void FolderSyncDriver::startUserNameRequest()
{
    m_currentRequest = UserNameRequest;
    startRequest();
}

void FolderSyncDriver::startDownloadRequest(const QString &srcFilePath,
                                             const QString &destFilePath)
{
    m_currentRequest = DownloadRequest;
    m_requestArgs.append(srcFilePath);
    m_requestArgs.append(destFilePath);
    startRequest();
}

void FolderSyncDriver::startUploadRequest(const QString &srcFilePath,
                                           const QString &destFilePath)
{
    m_currentRequest = UploadRequest;
    m_requestArgs.append(srcFilePath);
    m_requestArgs.append(destFilePath);
    startRequest();
}

void FolderSyncDriver::startRemoveRequest(const QString &cloudFilePath)
{
    m_currentRequest = RemoveRequest;
    m_requestArgs.append(cloudFilePath);
    startRequest();
}

void FolderSyncDriver::stopAllRequests()
{
   stopFileOp();
}

QString FolderSyncDriver::getServiceUrl()
{
    return "Generic folder based sync";
}


//-----------------------------------------------------------------------------
// Private slots
//-----------------------------------------------------------------------------

void FolderSyncDriver::stopFileOp()
{
    if (m_fileOpThread) {
        if (m_fileOpThread->isRunning()) {
            m_fileOpThread->terminate();
            m_fileOpThread->wait();
        }
    }
}

void FolderSyncDriver::fileOperationErrorSlot(const QString &message)
{
    m_fileOpThread = nullptr;
    emit errorSignal(message);
}

void FolderSyncDriver::fileOperationFinishedSlot(const QString &srcFileName,
                                                 const QString &destFileName,
                                                 int op)
{
    Q_UNUSED(op)

    m_fileOpThread = nullptr;

    switch (m_currentRequest) {
    case DownloadRequest:
    {
        QFileInfo src(srcFileName);
        emit downloadReady(src.fileName(), destFileName);
    }
        break;
    case UploadRequest:
    {
        QFileInfo dest(destFileName);
        emit uploadReady(srcFileName, dest.fileName());
    }
        break;
    case RemoveRequest:
    {
        QFileInfo file(srcFileName);
        emit removeReady(file.fileName());
    }
        break;
    default:
        break;
    }
}


//-----------------------------------------------------------------------------
// Private
//-----------------------------------------------------------------------------

void FolderSyncDriver::startRequest()
{
    //copy request args and clean them for future requests
    QStringList requestArgs = m_requestArgs;
    m_requestArgs.clear();

    //init command and etra args
    QStringList extraArgs; //extra arguments for specific command
    switch (m_currentRequest) {
    case NoRequest:
        //skip
        break;
    case InitRequest:
    {
        //check if directory is valid and writable
        QDir dir;
        dir.mkpath(m_folderPath);

        QFileInfo dirInfo(m_folderPath);
        if (dirInfo.isDir() && dirInfo.isWritable()) {
            m_folderPath = dirInfo.absoluteFilePath(); //in case path was relative
            emit authenticationUrlReady("skip");
        } else {
            emit errorSignal(tr("Sync directory %1 is not valid or writing permissions are missing.")
                             .arg(m_folderPath));
        }
    }
        break;
    case AuthValidationRequest:
    {
        SettingsManager s;
        s.saveEncodedAccessToken(m_folderPath); //save sync folder as access token
        emit authenticationValidated();
    }
        break;
    case UserNameRequest:
    {
        emit userNameResultReady(m_folderPath);
    }
        break;
    case DownloadRequest:
    {
        //get folder from settings
        SettingsManager sm;
        m_folderPath= sm.restoreEncodedAccessToken();

        QString src, dest;
        src = m_folderPath + "/" + requestArgs.at(0);
        dest = requestArgs.at(1);

        //check if file exists
        if (!QFileInfo::exists(src)) {
            emit downloadFileNotFound(requestArgs.at(0)); //flag without full path
        } else {
            //create file op thread
            m_fileOpThread = new QThread;
            FolderSyncTask *fileTask = new FolderSyncTask; //parent is null bc moved to thread later

            fileTask->configureTask(src, dest, FolderSyncTask::CopyOp);

            fileTask->moveToThread(m_fileOpThread);
            createFileThreadConnections(m_fileOpThread, fileTask);

            m_fileOpThread->start();
        }
    }
        break;
    case RemoveRequest:
    {
        //get folder from settings
        SettingsManager sm;
        m_folderPath= sm.restoreEncodedAccessToken();

        QString file;
        file = m_folderPath + "/" + requestArgs.at(0);

        //create file op thread
        m_fileOpThread = new QThread;
        FolderSyncTask *fileTask = new FolderSyncTask;

        fileTask->configureTask(file, "", FolderSyncTask::RemoveOp);

        fileTask->moveToThread(m_fileOpThread);
        createFileThreadConnections(m_fileOpThread, fileTask);

        m_fileOpThread->start();
    }
        break;
    case UploadRequest:
    {
        //get folder from settings
        SettingsManager sm;
        m_folderPath= sm.restoreEncodedAccessToken();

        QString src, dest;
        src = requestArgs.at(0);
        dest = m_folderPath + "/" + requestArgs.at(1);

        //create file op thread
        m_fileOpThread = new QThread;
        FolderSyncTask *fileTask = new FolderSyncTask;

        fileTask->configureTask(src, dest, FolderSyncTask::CopyOp);

        fileTask->moveToThread(m_fileOpThread);
        createFileThreadConnections(m_fileOpThread, fileTask);

        m_fileOpThread->start();
    }
        break;
    }
}

void FolderSyncDriver::createFileThreadConnections(QThread *thread,
                                                   FolderSyncTask *fileTask)
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
