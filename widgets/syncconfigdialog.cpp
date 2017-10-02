/*
 *  Copyright (c) 2012 Giorgio Wicklein <giowckln@gmail.com>
 */

//-----------------------------------------------------------------------------
// Hearders
//-----------------------------------------------------------------------------

#include "syncconfigdialog.h"
#include "ui_syncconfigdialog.h"
#include "../components/sync_framework/abstractsyncdriver.h"
#include "../components/sync_framework/syncengine.h"
#include "../components/sync_framework/syncsession.h"
#include "../components/settingsmanager.h"

#include <QtGui/QDesktopServices>
#include <QtCore/QUrl>


//-----------------------------------------------------------------------------
// Public
//-----------------------------------------------------------------------------

SyncConfigDialog::SyncConfigDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SyncConfigDialog), m_syncService(-1),
    m_syncDriver(0)
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
    m_syncService = ui->serviceComboBox->currentIndex();
    ui->stackedWidget->setCurrentIndex(1);

    //create driver
    m_syncDriver = SyncEngine::createSyncDriver(
                (SyncEngine::SyncService) m_syncService, this);
    createSyncConnections();

    m_syncDriver->startAuthenticationRequest();
}

void SyncConfigDialog::okButtonClicked()
{
    ui->stackedWidget->setCurrentIndex(2);

    QString authToken = ui->codeLineEdit->text().trimmed();
    m_syncDriver->startAuthenticationValidationRequest(authToken);
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

void SyncConfigDialog::syncError(const QString &message)
{
    if (ui->stackedWidget->currentIndex() == 2) {
        ui->authLabel->hide();
        ui->authProgressBar->hide();
        ui->resultLabel->setText(tr("%1 Please try again.").arg(message));
        ui->retryButton->setVisible(true);
    } else if (ui->stackedWidget->currentIndex() == 1) {
        ui->urlLabelResult->setText(message);
        ui->urlLabel->hide();
        ui->urlProgressBar->hide();
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
    ui->loginButton->setDefault(true);
}

void SyncConfigDialog::createConnections()
{
    connect(ui->cancelButton, SIGNAL(clicked()),
            this, SLOT(reject()));
    connect(ui->cancelAuthButton, SIGNAL(clicked()),
            this, SLOT(reject()));
    connect(ui->loginButton, SIGNAL(clicked()),
            this, SLOT(loginButtonClicked()));
    connect(ui->okButton, SIGNAL(clicked()),
            this, SLOT(okButtonClicked()));
    connect(ui->cancelUrlButton, SIGNAL(clicked()),
            this, SLOT(reject()));
    connect(ui->finishButton, SIGNAL(clicked()),
            this, SLOT(finishButtonClicked()));
    connect(ui->retryButton, SIGNAL(clicked()),
            this, SLOT(retryButtonClicked()));
    connect(ui->codeLineEdit, SIGNAL(textEdited(QString)),
            this, SLOT(codeLineEditTextEdited()));
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
