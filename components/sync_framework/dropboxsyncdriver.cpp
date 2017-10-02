/*
 *  Copyright (c) 2012 Giorgio Wicklein <giowckln@gmail.com>
 */

//-----------------------------------------------------------------------------
// Hearders
//-----------------------------------------------------------------------------

#include "dropboxsyncdriver.h"
#include "../settingsmanager.h"

#include <QtCore/QFileInfo>
#include <QtWidgets/QApplication>


//-----------------------------------------------------------------------------
// Public
//-----------------------------------------------------------------------------

DropboxSyncDriver::DropboxSyncDriver(QObject *parent) :
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

DropboxSyncDriver::~DropboxSyncDriver()
{
    stopAllRequests();
}

void DropboxSyncDriver::startAuthenticationRequest()
{
    m_currentRequest = AuthRequest;
    startRequest();
}

void DropboxSyncDriver::startAuthenticationValidationRequest(QString &authToken)
{
    m_currentRequest = AuthValidationRequest;
    m_requestArgs.append(authToken);
    startRequest();
}

void DropboxSyncDriver::startUserNameRequest()
{
    m_currentRequest = UserNameRequest;
    startRequest();
}

void DropboxSyncDriver::startDownloadRequest(const QString &srcFilePath,
                                             const QString &destFilePath)
{
    m_currentRequest = DownloadRequest;
    m_requestArgs.append(srcFilePath);
    m_requestArgs.append(destFilePath);
    startRequest();
}

void DropboxSyncDriver::startUploadRequest(const QString &srcFilePath,
                                           const QString &destFilePath)
{
    m_currentRequest = UploadRequest;
    m_requestArgs.append(srcFilePath);
    m_requestArgs.append(destFilePath);
    startRequest();
}

void DropboxSyncDriver::startRemoveRequest(const QString &cloudFilePath)
{
    m_currentRequest = RemoveRequest;
    m_requestArgs.append(cloudFilePath);
    startRequest();
}

void DropboxSyncDriver::stopAllRequests()
{
   if (m_process) {
        if (m_process->state() != QProcess::NotRunning) {
            m_process->kill();
            m_process->waitForFinished();
            m_currentRequest = NoRequest;
        }
    }
}

QString DropboxSyncDriver::getServiceUrl()
{
    return "<a href=\"https://www.dropbox.com\">www.dropbox.com</a>";
}


//-----------------------------------------------------------------------------
// Private slots
//-----------------------------------------------------------------------------

void DropboxSyncDriver::processError(QProcess::ProcessError error)
{
    //if process has been killed (request is cleared)
    if (m_currentRequest == NoRequest) return;

    QString errorMessage;

    switch (error) {
    case QProcess::FailedToStart:
        errorMessage = tr("Dropbox sync process failed to start");
        break;
    case QProcess::Crashed:
        errorMessage = tr("Dropbox sync process crashed");
        break;
    case QProcess::WriteError:
        errorMessage = tr("Dropbox sync process write error");
        break;
    case QProcess::ReadError:
        errorMessage = tr("Dropbox sync process read error");
        break;
    case QProcess::UnknownError:
        errorMessage = tr("Unknown error during Dropbox sync process");
        break;
    default:
        errorMessage = tr("Unknown error during Dropbox sync process "
                          "(in switch default)");
        break;
    }

    //clean request args
    m_requestArgs.clear();

    emit errorSignal(errorMessage);
}

void DropboxSyncDriver::processFinished(int exitCode,
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
            QString url;
            if (result.contains("URL:")) {
                QStringList list = result.split(':', QString::SkipEmptyParts);
                for (int i = 0; i < list.size(); i++) {
                    QString s = list.at(i);
                    if (s == "URL") {
                        if ((i + 2) < list.size()) {
                            url = list.at(i + 1);
                            //since https:// contains ':'
                            url.append(":").append(list.at(i + 2));
                        }
                    }
                }
                emit authenticationUrlReady(url);
            } else {
                error = true;
            }
        }
            break;
        case AuthValidationRequest:
        {
            if (result.contains("Access token:")) {
                QString token;
                QStringList list = result.split(':', QString::SkipEmptyParts);
                for (int i = 0; i < list.size(); i++) {
                    QString s = list.at(i);
                    if (s == "Access token") {
                        if ((i + 1) < list.size())
                            token = list.at(i + 1);
                    }
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
            if (result.contains("User:")) {
                QString user;
                QStringList list = result.split(':', QString::SkipEmptyParts);
                for (int i = 0; i < list.size(); i++) {
                    QString s = list.at(i);
                    if (s == "User") {
                        if ((i + 1) < list.size())
                            user = list.at(i + 1);
                    }
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
            if (result.contains("invalid_access_token"))
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

void DropboxSyncDriver::processReadyReadOutput()
{
    QString newOutput;
    newOutput.append(m_process->readAllStandardError());
    newOutput.append(m_process->readAllStandardOutput());
    m_processOutput.append(newOutput);

    switch (m_currentRequest) {
    case UploadRequest:
        if (m_totUploadChunks) {
            if (m_processOutput.contains("Uploading:")) {
                emit uploadedChunkReady(m_chunksUploaded, m_totUploadChunks);
                m_chunksUploaded++;
            }
        }
        break;
    default:
        break;
    }
}

//-----------------------------------------------------------------------------
// Private
//-----------------------------------------------------------------------------

void DropboxSyncDriver::initSecrets()
{
    //init app secret
    //app secret is a base64 encoded string
    //where on the base64 string the second half part
    //is moved ahead, and the original first half becomes the second one:
    //1. Encode app secret in base64
    //2. Result is: firstHalf+secondHalf
    //3. Now switch first and second half
    //4. Result is: secondHalf+firstHalf
    //to decode apply reverse algorithm
    m_appSecretEncoded = "J6d2x2MXR1aHptamhhcD";

    //load encoded access token from settings
    //access token is saved just as base64 encoded string
    SettingsManager s;
    m_accessTokenEncoded = s.restoreEncodedAccessToken();
}

void DropboxSyncDriver::startRequest()
{
    QStringList args;
    QString pythonInterpreterPath;
    QString appSecret;
    QString accessToken;
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
    pythonInterpreterPath = "python3";
    args.append("/usr/share/symphytum/sync/dropbox_client.py");
#endif

    //decode access token
    accessToken = QString(
                QByteArray::fromBase64(m_accessTokenEncoded.toLatin1()));

    //decode app secret
    QString s2;
    QString s1;
    for (int i = 0; i < m_appSecretEncoded.size()/2; i++) {
        s2.append(m_appSecretEncoded.at(i));
    }
    for (int i = m_appSecretEncoded.size()/2;
         i < m_appSecretEncoded.size(); i++) {
        s1.append(m_appSecretEncoded.at(i));
    }
    QString e = s1 + s2;
    appSecret = QString(QByteArray::fromBase64(e.toLatin1()));

    //init command and etra args
    QStringList extraArgs; //extra arguments for specific command
    switch (m_currentRequest) {
    case NoRequest:
        //skip
        break;
    case AuthRequest:
        command = "authorize_url";
        accessToken = "none";
        break;
    case AuthValidationRequest  :
        command = "create_access_token";
        accessToken = "none";
        extraArgs = m_requestArgs;
        break;
    case UserNameRequest:
        command = "user_name";
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
    args.append(appSecret);
    args.append(accessToken);
    args.append(command);
    args.append(extraArgs);

    m_processOutput.clear();
    m_process->start(pythonInterpreterPath, args);

    //close write channel to allow
    //ready output signals, see docs
    m_process->closeWriteChannel();
}
