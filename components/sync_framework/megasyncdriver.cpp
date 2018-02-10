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
#include <QtCore/QFile>
//FIXME debug rm
#include <QDebug>

//-----------------------------------------------------------------------------
// Public
//-----------------------------------------------------------------------------

MegaSyncDriver::MegaSyncDriver(QObject *parent) :
    AbstractSyncDriver(parent),
    m_process(0), m_currentRequest(NoRequest)
{
    initSecrets();
    m_megaFolderPath = DefinitionHolder::NAME + "/";

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
    m_requestArgs.append(m_megaFolderPath + srcFilePath);
    m_requestArgs.append(destFilePath + "_tmp"); //temp file because downloads are not overwritten by megacmd
    startRequest();
}

void MegaSyncDriver::startUploadRequest(const QString &srcFilePath,
                                           const QString &destFilePath)
{
    m_currentRequest = UploadRequestTmpStep;
    m_requestArgs.append(srcFilePath);
    m_requestArgs.append(m_megaFolderPath + destFilePath);
    startRequest();
}

void MegaSyncDriver::startRemoveRequest(const QString &cloudFilePath)
{
    m_currentRequest = RemoveRequest;
    m_requestArgs.append(m_megaFolderPath + cloudFilePath);
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
            QString src, dest, orig;
            src = requestArgs.at(0);
            dest = requestArgs.at(1);
            orig = QString(dest).remove("_tmp"); //get original name

            //since downloaded files are not overwritten
            //rename temporary download file to original name
            bool mvok = false;
            QFile::remove(orig);
            mvok = QFile::rename(dest, orig);
            if (!mvok) {
                result.prepend(tr("Failed to replace temporary downloaded file: %1\n")
                               .arg(orig));
                error = true;
            } else {
                if (!result.contains("[err:")) {
                    emit downloadReady(src, orig);
                } else if (result.contains("Couldn't find")) {
                    emit downloadFileNotFound(src);
                } else {
                    error = true;
                }
            }
        }
            break;
        case UploadRequestTmpStep:
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
    case UploadRequestTmpStep:
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
        command = "get";
        extraArgs = m_requestArgs;
        break;
    case RemoveRequest:
        command = "delete_file";
        extraArgs = m_requestArgs;
        break;

    //MEGAcmd doesn't support upload progress at this point
    //so uploading is a 3 step process
    case UploadRequestTmpStep:
        //MEGAcmd doesn't support upload progress at this point
        command = "upload_file";
        //FIXME: because files are not overwritten
        //upload first to a temp file then delete original
        //and finally move temp to original name
        extraArgs = m_requestArgs;
        break;
    case UploadRequestRmStep:
        command = "upload_file";
        //FIXME: because files are not overwritten
        //upload first to a temp file then delete original
        //and finally move temp to original name
        extraArgs = m_requestArgs;
        break;
    case UploadRequestMvStep:
        command = "upload_file";
        //FIXME: because files are not overwritten
        //upload first to a temp file then delete original
        //and finally move temp to original name
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
