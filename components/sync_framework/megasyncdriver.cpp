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
#include <QtCore/QStandardPaths>


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
    m_requestArgs.append(destFilePath + ".tmp"); //temp file because downloads are not overwritten by megacmd
    startRequest();
}

void MegaSyncDriver::startUploadRequest(const QString &srcFilePath,
                                           const QString &destFilePath)
{
    m_currentRequest = UploadRequestTmpStep;
    m_requestArgs.append(srcFilePath);
    m_requestArgs.append(m_megaFolderPath + destFilePath + ".tmp");
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
        errorMessage = tr("MEGA sync process failed to start.<br /><br />"
                          "Please <b>install</b> MEGAcmd by visiting "
                          "<a href='https://mega.nz/cmd'>mega.nz/cmd</a>.<br />");
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
#ifdef Q_OS_WIN
            if (!result.contains("[err:")) { //windows not using interactive cmd shell,
                                             //just login command, see startRequest()
#else
            if (result.contains("100.00 %")) {
#endif
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
            src.remove(m_megaFolderPath);
            dest = requestArgs.at(1);
            orig = QString(dest).remove(".tmp"); //get original name

            //file found?
            bool notFound = result.contains("Couldn't find");

            if (!result.contains("[err:")) {
                if (!notFound) {
                    //since downloaded files are not overwritten
                    //rename temporary download file to original name
                    bool mvok = false;
                    QFile::remove(orig);
                    mvok = QFile::rename(dest, orig);
                    if (!mvok) {
                        result.prepend(tr("Failed to replace temporary downloaded file: %1\n")
                                       .arg(orig));
                        error = true;
                    }
                }
                if (!error)
                    emit downloadReady(src, orig);
            } else if (notFound) {
                emit downloadFileNotFound(src);
            } else {
                error = true;
            }
        }
            break;
        case UploadRequestTmpStep:
        {
            //NOTE: if the user is logged out or session expired
            //megacmd says fail not found when really it should
            //say not logged in
            //see bug https://github.com/meganz/MEGAcmd/issues/19
            if (!result.contains("[err:")) {
                //do next step
                m_currentRequest = UploadRequestRmStep;
                m_requestArgs = requestArgs;
                startRequest();
            } else if (result.contains("Destination is not valid")) {
                //probably an orphan .tmp file was left over
                //rm .tmp file and redo request
                m_currentRequest = RemoveTmpCloudFileRequest;
                m_requestArgs = requestArgs;
                startRequest();
            } else {
                error = true;
            }
        }
            break;
        case UploadRequestRmStep:
        {
            if (!result.contains("[err:") || result.contains("No such file or directory")) {
                //do next step
                m_currentRequest = UploadRequestMvStep;
                m_requestArgs = requestArgs;
                startRequest();
            } else {
                error = true;
            }
        }
            break;
        case UploadRequestMvStep:
        {
            QString src, dest;
            src = requestArgs.at(0);
            dest = requestArgs.at(1);
            dest.remove(m_megaFolderPath);
            dest.remove(".tmp");

            if (!result.contains("[err:")) {
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
            QString file = requestArgs.at(0);
            file.remove(m_megaFolderPath);
            if (!result.contains("[err:") || result.contains("No such file or directory")) {
                emit removeReady(file);
            } else {
                error = true;
            }
        }
            break;
        case RemoveTmpCloudFileRequest:
        {
            if (!result.contains("[err:")) {
                //redo upload tmp request
                m_currentRequest = UploadRequestTmpStep;
                m_requestArgs = requestArgs;
                startRequest();
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
            if (result.contains("Not logged in"))
                emit authTokenExpired();
            else if (result.contains("connection")) //megacmd doesn't report network fails
                emit connectionFailed();
            else if (result.contains("quota")) //this was never tested
                                               //check the output when storage/bandwidth full
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

    //NOTE: megacmd currently has no progress status output
    /*switch (m_currentRequest) {
    case UploadRequestTmpStep:
        if (m_totUploadChunks) {
            if (m_processOutput.contains("Uploading:")) {
                emit uploadedChunkReady(m_chunksUploaded, m_totUploadChunks);
                m_chunksUploaded++;
            }
        }
        break;
    default:
        break;
    }*/

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
    //megacmd is installed to C:\Users\user\AppData\Local\MEGAcmd
    megaCmdPath =  QString(QStandardPaths::standardLocations(
                               QStandardPaths::GenericDataLocation).at(0))
            .append("/MEGAcmd/").append("MEGAclient.exe");
#endif
#ifdef Q_OS_OSX
    megaCmdPath = QString("/Applications/MEGAcmd.app/Contents/MacOS/");
    megaCmdPath.append("mega-exec");
#endif
#ifdef Q_OS_LINUX
    megaCmdPath = "/usr/bin/mega-exec";
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
#ifdef Q_OS_WIN
        //FIXME: output reading from the mega-cmd shell doesn't work on windows
        //so use login command as a workaround, issue with this is that the password is leaked
        //as a command line argument, not the best solution
        //megaCmdPath.replace("MEGAclient.exe", "MEGAcmdShell.exe"); //use interactive mega shell
        command = "login";
        extraArgs = m_requestArgs;
#elif defined(Q_OS_MACOS)
        megaCmdPath.replace("mega-exec", "MEGAcmdShell"); //use interactive mega shell
#else
        megaCmdPath.replace("mega-exec", "mega-cmd"); //use interactive mega shell
#endif
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
        command = "rm";
        extraArgs = m_requestArgs;
        break;
    case RemoveTmpCloudFileRequest:
        command = "rm";
        extraArgs = m_requestArgs;
        if (extraArgs.size() >= 2) {
            extraArgs.removeFirst(); // remove local file path arg
        }
        break;

    //MEGAcmd doesn't support upload progress at this point
    //so uploading is a 3 step process
    case UploadRequestTmpStep:
        //MEGAcmd doesn't support upload progress at this point
        command = "put";
        extraArgs = m_requestArgs;
        extraArgs.append("-c"); //create path if needed
        break;
    case UploadRequestRmStep:
        command = "rm";
        extraArgs = m_requestArgs;
        if (extraArgs.size() >= 2) {
            extraArgs.removeFirst(); // remove local file path arg
            extraArgs.replace(0, QString(extraArgs.at(0)).remove(".tmp")); //rm original file
        }
        break;
    case UploadRequestMvStep:
        command = "mv";
        extraArgs = m_requestArgs;
        if (extraArgs.size() >= 2) {
            extraArgs.replace(0, extraArgs.at(1)); //replace local path with cloud tmp file
            extraArgs.replace(1, QString(extraArgs.at(1)).remove(".tmp")); //rename to original
        }
        break;
    }

    //init args
    args.append(command);
    args.append(extraArgs);

    m_processOutput.clear();
    m_process->start(megaCmdPath, args);

#ifndef Q_OS_WIN //see above, mega-cmd shell doesn't work with QProcess on windows
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
#endif

    //close write channel to allow
    //ready output signals, see docs
    m_process->closeWriteChannel();
}
