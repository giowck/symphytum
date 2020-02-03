/*
 *  Copyright (c) 2012 Giorgio Wicklein <giowckln@gmail.com>
 */

//-----------------------------------------------------------------------------
// Hearders
//-----------------------------------------------------------------------------

#include "syncconfigdialog.h"
#include "ui_syncconfigdialog.h"
#include "../components/sync_framework/abstractsyncdriver.h"
#include "../components/sync_framework/syncsession.h"
#include "../components/settingsmanager.h"

#include <QtGui/QDesktopServices>
#include <QtCore/QUrl>
#include <QtGui/QRegExpValidator>
#include <QtWidgets/QFileDialog>


//-----------------------------------------------------------------------------
// Public
//-----------------------------------------------------------------------------

SyncConfigDialog::SyncConfigDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SyncConfigDialog), m_syncService(SyncEngine::DropboxSync),
    m_syncDriver(nullptr)
{
    ui->setupUi(this);

    init();
    createConnections();
}

SyncConfigDialog::~SyncConfigDialog()
{
    delete ui;
}

void SyncConfigDialog::reauthenticateSyncService()
{
    SettingsManager s;
    if (s.isCloudSyncActive()) {
        ui->serviceComboBox->setCurrentIndex(s.restoreCurrentCloudSyncService());
        loginButtonClicked();
    }
}


//-----------------------------------------------------------------------------
// Private slots
//-----------------------------------------------------------------------------

void SyncConfigDialog::loginButtonClicked()
{
    m_syncService = (SyncEngine::SyncService) ui->serviceComboBox->currentIndex();
    int configPage;

    //create driver
    m_syncDriver = SyncEngine::createSyncDriver(m_syncService, this);
    createSyncConnections();

    switch (m_syncService) {
    case SyncEngine::DropboxSync:
        configPage = 1;
        m_syncDriver->startAuthenticationRequest();
        break;
    case SyncEngine::MegaSync:
        configPage = 3;
        //start auth req only later after email and pass prompt
        break;
    case SyncEngine::FolderSync:
        configPage = 4;
        //start auth req only later after path configured
        break;
    default:
        configPage = 0;
        break;
    }

    ui->stackedWidget->setCurrentIndex(configPage);
}

void SyncConfigDialog::okButtonClicked()
{
    ui->stackedWidget->setCurrentIndex(2);

    QString authToken = ui->codeLineEdit->text().trimmed();
    m_syncDriver->startAuthenticationValidationRequest(authToken);
}

void SyncConfigDialog::okMegaButtonClicked()
{
    QStringList megaCredentials;
    megaCredentials.append(ui->megaEmailLineEdit->text().trimmed());
    megaCredentials.append(ui->megaPassLineEdit->text().trimmed());
    megaCredentials.append(ui->mega2FALineEdit->text().trimmed());
    m_syncDriver->startAuthenticationRequest(megaCredentials);

    ui->stackedWidget->setCurrentIndex(2);
}

void SyncConfigDialog::okFolderSyncButtonClicked()
{
    ui->stackedWidget->setCurrentIndex(2);

    QString folderPath = ui->folderSyncPathLineEdit->text().trimmed();
    folderPath.append("/SymphytumSync");
    m_syncDriver->startAuthenticationRequest(QStringList() << folderPath);
}

void SyncConfigDialog::finishButtonClicked()
{
    //save cloud service config
    SettingsManager s;
    s.setCloudSyncActive(true);
    s.saveCurrentCloudSyncService(ui->serviceComboBox->currentIndex());

    //configure sync driver on sync engine
    SyncEngine::getInstance().reconfigureSyncDriver();

    //set session
    SyncSession::IS_ENABLED = true;

    accept();
}

void SyncConfigDialog::retryButtonClicked()
{
    ui->stackedWidget->setCurrentIndex(0);

    ui->urlLabelResult->setText(tr("Please wait..."));
    ui->urlLabelResult->show();
    ui->urlLabel->show();
    ui->urlProgressBar->show();
    ui->urlLabelOK->hide();

    ui->authLabel->show();
    ui->authProgressBar->show();
    ui->resultLabel->setText(tr("Please wait..."));
    ui->retryButton->setVisible(false);
}

void SyncConfigDialog::codeLineEditTextEdited()
{
    ui->okButton->setEnabled(!ui->codeLineEdit->text().trimmed().isEmpty());
}

void SyncConfigDialog::megaCredentialsInputEdited()
{
    ui->okMegaButton->setEnabled(!ui->megaEmailLineEdit->text().trimmed().isEmpty() &&
                                 ui->megaEmailLineEdit->text().contains("@") &&
                                 !ui->megaPassLineEdit->text().trimmed().isEmpty());
}

void SyncConfigDialog::folderSyncPathEdited()
{
    ui->okFolderSyncButton->setEnabled(!ui->folderSyncPathLineEdit->text()
                                       .trimmed().isEmpty());
}

void SyncConfigDialog::folderSyncBrowseButtonClicked()
{
    QString documentsDir = QStandardPaths::standardLocations(
                QStandardPaths::DocumentsLocation).at(0);
    QString folderPath = QFileDialog::getExistingDirectory(this,
                                                           tr("Select sync folder"),
                                                           documentsDir,
                                                           QFileDialog::ShowDirsOnly
                                                           | QFileDialog::DontResolveSymlinks);
    ui->folderSyncPathLineEdit->setText(folderPath);
    ui->folderSyncPathLineEdit->setFocus();
}

void SyncConfigDialog::syncError(const QString &message)
{
    if (ui->stackedWidget->currentIndex() == 1) {
        ui->urlLabelResult->setText(message);
        ui->urlLabel->hide();
        ui->urlProgressBar->hide();
    } else {
        ui->stackedWidget->setCurrentIndex(2);
        ui->authLabel->hide();
        ui->authProgressBar->hide();
        ui->resultLabel->setText(tr("%1 Please try again.").arg(message));
        ui->retryButton->setVisible(true);
    }
}

void SyncConfigDialog::syncErrorTokenExpired()
{
    syncError(tr("The authentication token is invalid or expired."));
}

void SyncConfigDialog::syncErrorConnectionFailed()
{
    syncError(tr("Connection to cloud service failed, check your connection."));
}

void SyncConfigDialog::syncUrlAuth(const QString &url)
{
    if ((m_syncService == SyncEngine::MegaSync) ||
            (m_syncService == SyncEngine::FolderSync)) {
        //start validation directly since no URL auth is needed
        this->okButtonClicked();
    } else {
        ui->urlLabelResult->hide();
        ui->urlProgressBar->hide();
        ui->urlLabel->hide();
        ui->urlLabelOK->show();
        ui->codeLabel->setVisible(true);
        ui->codeLineEdit->setVisible(true);
        ui->codeLineEdit->setFocus();

        //open browser with url
        QDesktopServices::openUrl(QUrl(url));
    }
}

void SyncConfigDialog::syncAuthValidated()
{
    ui->authLabel->setText(tr("Accessing..."));
    m_syncDriver->startUserNameRequest();
}

void SyncConfigDialog::syncUserName(const QString &userName)
{
    ui->authLabel->hide();
    ui->authProgressBar->hide();
    ui->resultLabel->setText(tr("Logged in as %1").arg(userName));
    updateFinishButton(true);
}


//-----------------------------------------------------------------------------
// Private
//-----------------------------------------------------------------------------

void SyncConfigDialog::init()
{
    ui->urlLabelOK->setVisible(false);
    ui->retryButton->setVisible(false);
    ui->codeLabel->setVisible(false);
    ui->codeLineEdit->setVisible(false);
    ui->serviceComboBox->addItem(QIcon(":/images/icons/dropbox.png"),
                                 tr("Dropbox"));
    ui->serviceComboBox->addItem(QIcon(":/images/icons/megasync.png"),
                                 tr("MEGA"));
    ui->serviceComboBox->addItem(QIcon(":/images/icons/foldersync.png"),
                                 tr("Generic provider (folder based)"));
    ui->loginButton->setDefault(true);

    //2FA mega
    QRegExp re("^[0-9]{6}$");
    QRegExpValidator *mega2faValidator = new QRegExpValidator(re, this);
    ui->mega2FALineEdit->setValidator(mega2faValidator);
    ui->mega2FAGroupBox->hide();
}

void SyncConfigDialog::createConnections()
{
    connect(ui->cancelButton, SIGNAL(clicked()),
            this, SLOT(reject()));
    connect(ui->cancelAuthButton, SIGNAL(clicked()),
            this, SLOT(reject()));
    connect(ui->loginButton, SIGNAL(clicked()),
            this, SLOT(loginButtonClicked()));
    connect(ui->finishButton, SIGNAL(clicked()),
            this, SLOT(finishButtonClicked()));
    connect(ui->retryButton, SIGNAL(clicked()),
            this, SLOT(retryButtonClicked()));

    //OAuth based like dropbox
    connect(ui->okButton, SIGNAL(clicked()),
            this, SLOT(okButtonClicked()));
    connect(ui->cancelUrlButton, SIGNAL(clicked()),
            this, SLOT(reject()));
    connect(ui->codeLineEdit, SIGNAL(textEdited(QString)),
            this, SLOT(codeLineEditTextEdited()));

    //mega
    connect(ui->okMegaButton, &QPushButton::clicked,
            this, &SyncConfigDialog::okMegaButtonClicked);
    connect(ui->cancelMegaButton, &QPushButton::clicked,
            this, &QDialog::reject);
    connect(ui->megaEmailLineEdit, &QLineEdit::textEdited,
            this, &SyncConfigDialog::megaCredentialsInputEdited);
    connect(ui->megaPassLineEdit, &QLineEdit::textEdited,
            this, &SyncConfigDialog::megaCredentialsInputEdited);

    //foldersync
    connect(ui->folderSyncPathLineEdit, &QLineEdit::textChanged,
            this, &SyncConfigDialog::folderSyncPathEdited);
    connect(ui->okFolderSyncButton, &QPushButton::clicked,
            this, &SyncConfigDialog::okFolderSyncButtonClicked);
    connect(ui->folderSyncBrowseButton, &QPushButton::clicked,
            this, &SyncConfigDialog::folderSyncBrowseButtonClicked);
    connect(ui->cancelFolderSyncButton, &QPushButton::clicked,
            this, &QDialog::reject);
}

void SyncConfigDialog::updateFinishButton(bool enabled)
{
    ui->finishButton->setEnabled(enabled);
    ui->finishButton->setDefault(enabled);
    if (enabled)
        ui->finishButton->setFocus();
}

void SyncConfigDialog::createSyncConnections()
{
    connect(m_syncDriver, SIGNAL(errorSignal(QString)),
            this, SLOT(syncError(QString)));
    connect(m_syncDriver, SIGNAL(authTokenExpired()),
            this, SLOT(syncErrorTokenExpired()));
    connect(m_syncDriver, SIGNAL(connectionFailed()),
            this, SLOT(syncErrorConnectionFailed()));
    connect(m_syncDriver, SIGNAL(authenticationUrlReady(QString)),
            this, SLOT(syncUrlAuth(QString)));
    connect(m_syncDriver, SIGNAL(authenticationValidated()),
            this, SLOT(syncAuthValidated()));
    connect(m_syncDriver, SIGNAL(userNameResultReady(QString)),
            this, SLOT(syncUserName(QString)));
}
