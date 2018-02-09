/*
 *  Copyright (c) 2018 Oirio Joshi (joshirio)
 */

//-----------------------------------------------------------------------------
// Hearders
//-----------------------------------------------------------------------------

#include "megasyncdriver.h"
#include "../settingsmanager.h"
#include "../../utils/definitionholder.h"

#include <QtCore/QFileInfo>
#include <QtWidgets/QApplication>
//FIXME debug rm
#include <QDebug>

//-----------------------------------------------------------------------------
// Public
//-----------------------------------------------------------------------------

MegaSyncDriver::MegaSyncDriver(QObject *parent) :
    AbstractSyncDriver(parent),
    m_process(0), m_currentRequest(NoRequest),
    m_totUploadChunks(0), m_chunksUploaded(0)
{
    initSecrets();

    m_process = new QProcess(this);
    connect(m_process, SIGNAL(readyReadStandardOutput()),
            this, SLOT(processReadyReadOutput()));
    connect(m_process, SIGNAL(readyReadStandardError()),
            this, SLOT(processReadyReadOutput()));
    connect(m_process, SIGNAL(error(QProcess::ProcessError)),
            this, SLOT(processError(QProcess::ProcessError)));
    connect(m_process, SIGNAL(finished(int,QProcess::ExitStatus)),
            this, SLOT(processFinished(int,QProcess::ExitStatus)));
}

MegaSyncDriver::~MegaSyncDriver()
{
    stopAllRequests();
}

void MegaSyncDriver::startAuthenticationRequest(const QStringList &megaCredentials)
{
    m_currentRequest = AuthRequest;

    //mega email and pass as args
    if (megaCredentials.size() == 2) {
        m_requestArgs.append(megaCredentials.at(0));
        m_requestArgs.append(megaCredentials.at(1));
    }

    startRequest();
}

void MegaSyncDriver::startAuthenticationValidationRequest(const QString &authToken)
{
    m_currentRequest = AuthValidationRequest;
    m_requestArgs.append(authToken);
    startRequest();
}

void MegaSyncDriver::startUserNameRequest()
{
    m_currentRequest = UserNameRequest;
    startRequest();
}

void MegaSyncDriver::startDownloadRequest(const QString &srcFilePath,
                                             const QString &destFilePath)
{
    m_currentRequest = DownloadRequest;
    m_requestArgs.append(srcFilePath);
    m_requestArgs.append(destFilePath);
    startRequest();
}

void MegaSyncDriver::startUploadRequest(const QString &srcFilePath,
                                           const QString &destFilePath)
{
    m_currentRequest = UploadRequest;
    m_requestArgs.append(srcFilePath);
    m_requestArgs.append(destFilePath);
    startRequest();
}

void MegaSyncDriver::startRemoveRequest(const QString &cloudFilePath)
{
    m_currentRequest = RemoveRequest;
    m_requestArgs.append(cloudFilePath);
    startRequest();
}

void MegaSyncDriver::stopAllRequests()
{
   if (m_process) {
        if (m_process->state() != QProcess::NotRunning) {
            m_process->kill();
            m_process->waitForFinished();
            m_currentRequest = NoRequest;
        }
    }
}

QString MegaSyncDriver::getServiceUrl()
{
    return "<a href=\"https://www.mega.nz\">www.mega.nz</a>";
}


//-----------------------------------------------------------------------------
// Private slots
//-----------------------------------------------------------------------------

void MegaSyncDriver::processError(QProcess::ProcessError error)
{
    //if process has been killed (request is cleared)
    if (m_currentRequest == NoRequest) return;

    QString errorMessage;

    switch (error) {
    case QProcess::FailedToStart:
        errorMessage = tr("MEGA sync process failed to start");
        break;
    case QProcess::Crashed:
        errorMessage = tr("MEGA sync process crashed");
        break;
    case QProcess::WriteError:
        errorMessage = tr("MEGA sync process write error");
        break;
    case QProcess::ReadError:
        errorMessage = tr("MEGA sync process read error");
        break;
    case QProcess::UnknownError:
        errorMessage = tr("Unknown error during MEGA sync process");
        break;
    default:
        errorMessage = tr("Unknown error during MEGA sync process "
                          "(in switch default)");
        break;
    }

    //clean request args
    m_requestArgs.clear();

    emit errorSignal(errorMessage);
}

void MegaSyncDriver::processFinished(int exitCode,
                                     QProcess::ExitStatus exitStatus)
{
    Q_UNUSED(exitCode);
    bool error = false;

    //copy request args and clean them for future requests
    QStringList requestArgs = m_requestArgs;
    m_requestArgs.clear();

    if (exitStatus == QProcess::NormalExit) {
        QString result = m_processOutput;

        switch (m_currentRequest) {
        case AuthRequest:
        {
            if (result.contains("100.00 %")) {
                emit authenticationUrlReady("skip");
            } else if (result.contains("Already logged in")) {
                //logout and redo request
                m_currentRequest = LogOutRequest;
                m_requestArgs = requestArgs;
                startRequest();
            } else {
                error = true;

                //clean errors a bit up
                if (result.contains("invalid email or password"))
                    result = tr("Login failed: invalid email or password\n");
            }
        }
            break;
        case AuthValidationRequest:
        {
            if (result.contains("Your (secret) session is:")) {
                QString token;
                QStringList list = result.split("session is: ", QString::SkipEmptyParts);
                if (list.size() >= 2) {
                    token = list.at(1);
                    token.remove("\n");
                }
                QString encodedToken = QString(token.toLatin1().toBase64());
                m_accessTokenEncoded = encodedToken;
                SettingsManager s;
                s.saveEncodedAccessToken(encodedToken);
                emit authenticationValidated();
            } else {
                error = true;
            }
        }
            break;
        case UserNameRequest:
        {
            if (result.contains("Account e-mail:")) {
                QString user;
                QStringList list = result.split("e-mail: ", QString::SkipEmptyParts);
                if (list.size() >= 2) {
                    user = list.at(1);
                    user.remove("\n");
                }
                emit userNameResultReady(user);
            } else {
                error = true;
            }
        }
            break;
        case DownloadRequest:
        {
            QString src, dest;
            src = requestArgs.at(0);
            dest = requestArgs.at(1);

            if (result.contains("Download:OK")) {
                emit downloadReady(src, dest);
            } else if (result.contains("not_found")) {
                emit downloadFileNotFound(src);
            } else {
                error = true;
            }
        }
            break;
        case UploadRequest:
        {
            QString src, dest;
            src = requestArgs.at(0);
            dest = requestArgs.at(1);

            if (result.contains("Upload:OK")) {
                emit uploadReady(src, dest);
            } else {
                error = true;
            }
        }
            break;
        case LogOutRequest:
        {
            if (result.contains("Logging out")) {
                //OK now do login
                startAuthenticationRequest(requestArgs);
            } else {
                error = true;
            }
        }
            break;
        case RemoveRequest:
        {
            QString file;
            file = requestArgs.at(0);

            if (result.contains("Delete:OK") || result.contains("not_found")) {
                emit removeReady(file);
            } else {
                error = true;
            }
        }
            break;
        default:
            break;
        }

        //inform about errors with signals
        if (error) {
            if (result.contains("Not logged in")) //FIXME: todo
                emit authTokenExpired();
            else if (result.contains("Failed to establish a new connection"))
                emit connectionFailed();
            else if (result.contains("503")) //this was never tested
                                             //check the output when storage full
                                             //for the correct err code
                emit storageQuotaExceeded();
            else
                emit errorSignal(result);
        }
    }
}

void MegaSyncDriver::processReadyReadOutput()
{
    QString newOutput;
    newOutput.append(m_process->readAllStandardError());
    newOutput.append(m_process->readAllStandardOutput());
    m_processOutput.append(newOutput);

    //FIXME: debug
    qDebug() << m_processOutput;

    switch (m_currentRequest) {
    case UploadRequest:
        /*if (m_totUploadChunks) {
            if (m_processOutput.contains("Uploading:")) {
                emit uploadedChunkReady(m_chunksUploaded, m_totUploadChunks);
                m_chunksUploaded++;
            }
        }*/
        break;
    default:
        break;
    }

}

//-----------------------------------------------------------------------------
// Private
//-----------------------------------------------------------------------------

void MegaSyncDriver::initSecrets()
{
    //load session key from settings
    //session token is saved just as base64 encoded string
    SettingsManager s;
    m_accessTokenEncoded = s.restoreEncodedAccessToken();
}

void MegaSyncDriver::startRequest()
{
    QStringList args;
    QString megaCmdPath;
    QString sessionKey;
    QString command;

#ifdef Q_OS_WIN
    pythonInterpreterPath = QApplication::applicationDirPath()
            .append("/sync/").append("dropbox_client.exe");
#endif
#ifdef Q_OS_OSX
    if (QSysInfo::MacintoshVersion <= QSysInfo::MV_10_6)
        pythonInterpreterPath = "python2.6";
    else
        pythonInterpreterPath = "python2.7"; //python3 not yet available
    QString path = QApplication::applicationDirPath().append("/sync/");
    path.append("dropbox_client.py");
    args.append(path);
#endif
#ifdef Q_OS_LINUX
    if(DefinitionHolder::APPIMAGE_LINUX) {
        megaCmdPath = QCoreApplication::applicationDirPath()
                + "/../share/symphytum/sync/dropbox_client/dropbox_client";
    } else {
        megaCmdPath = "mega-exec"; //FIXME
        //args.append("/usr/share/symphytum/sync/dropbox_client.py");
    }
#endif

    //decode session token
    sessionKey = QString(
                QByteArray::fromBase64(m_accessTokenEncoded.toLatin1()));

    //init command and etra args
    QStringList extraArgs; //extra arguments for specific command
    switch (m_currentRequest) {
    case NoRequest:
        //skip
        break;
    case AuthRequest:
        megaCmdPath.replace("mega-exec", "mega-cmd"); //use interactive mega shell
        break;
    case LogOutRequest:
        command = "logout";
        break;
    case AuthValidationRequest:
        command = "session";
        break;
    case UserNameRequest:
        command = "whoami";
        break;
    case DownloadRequest:
        command = "download_file";
        extraArgs = m_requestArgs;
        break;
    case RemoveRequest:
        command = "delete_file";
        extraArgs = m_requestArgs;
        break;
    case UploadRequest:
        //files bigger 4MiB are uploaded in chunks
        //FIXME: since there's a timeout bug in python v2 SDK for Dropbox
        //reduce to 1 MiB chunks
        QFileInfo file(m_requestArgs.at(0));
        qint64 size = file.size();
        //qint64 _4MiB = 4194304; //using 1MB to avoid timeouts
        qint64 _1MiB = 1048576;
        if (size <= _1MiB) {
            command = "upload_file";
            m_totUploadChunks = 0;
            m_chunksUploaded = 0;
        } else {
            command = "upload_file_chunked";
            m_chunksUploaded = 0;
            m_totUploadChunks = size / _1MiB;

            //if rest, add smaller chunk
            if (size % _1MiB)
                m_totUploadChunks++;
        }
        extraArgs = m_requestArgs;
        break;
    }

    //init args
    args.append(command);
    args.append(extraArgs);

    m_processOutput.clear();
    m_process->start(megaCmdPath, args);

    //if login command, use interactive command to avoid pass leak
    //instead of command line args
    if (m_currentRequest == AuthRequest) {
        QString megaEmail;
        QString megaPass;
        if (m_requestArgs.size() >= 2) {
            megaEmail = m_requestArgs.at(0);
            megaPass = m_requestArgs.at(1);
        }
        m_process->waitForReadyRead();
        m_process->write("login " + megaEmail.toLatin1() + " " + megaPass.toLatin1());
        m_process->waitForBytesWritten();
    }

    //close write channel to allow
    //ready output signals, see docs
    m_process->closeWriteChannel();
}
