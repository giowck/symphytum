/*
 *  Copyright (c) 2012 Giorgio Wicklein <giowckln@gmail.com>
 */

//-----------------------------------------------------------------------------
// Hearders
//-----------------------------------------------------------------------------

#include "backupdialog.h"
#include "ui_backupdialog.h"
#include "../components/backupmanager.h"

#include <QtWidgets/QFileDialog>
#include <QtGui/QDesktopServices>
#include <QtCore/QDateTime>


//-----------------------------------------------------------------------------
// Public
//-----------------------------------------------------------------------------

BackupDialog::BackupDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::BackupDialog),
    m_backupFileRestored(false)
{
    ui->setupUi(this);

    //components
    m_backupManager = new BackupManager(this);

    //connections
    connect(ui->chooseCancelButton, SIGNAL(clicked()),
            this, SLOT(reject()));
    connect(ui->cancelBackupProgressButton, SIGNAL(clicked()),
            this, SLOT(reject()));
    connect(ui->finishButton, SIGNAL(clicked()),
            this, SLOT(accept()));
    connect(ui->chooseNextButton, SIGNAL(clicked()),
            this, SLOT(chooseNextButtonClicked()));
    connect(ui->exportBackButton, SIGNAL(clicked()),
            this, SLOT(exportBackButtonClicked()));
    connect(ui->importBackButton, SIGNAL(clicked()),
            this, SLOT(importBackButtonClicked()));
    connect(ui->exportBrowseButton, SIGNAL(clicked()),
            this, SLOT(exportBrowseButtonClicked()));
    connect(ui->importBrowseButton, SIGNAL(clicked()),
            this, SLOT(importBrowseButtonClicked()));
    connect(ui->exportDestLineEdit, SIGNAL(textChanged(QString)),
            this, SLOT(updateBackupButton()));
    connect(ui->importDestLineEdit, SIGNAL(textChanged(QString)),
            this, SLOT(updateRestoreButton()));
    connect(ui->backupButton, SIGNAL(clicked()),
            this, SLOT(backupButtonClicked()));
    connect(ui->restoreButton, SIGNAL(clicked()),
            this, SLOT(restoreButtonClicked()));

    //backup connections
    connect(m_backupManager, SIGNAL(progressSignal(int,int)),
            this, SLOT(progressSlot(int,int)));
    connect(m_backupManager, SIGNAL(backupTaskFailed(QString)),
            this, SLOT(backupTaskFailed(QString)));
    connect(m_backupManager, SIGNAL(exportCompleted()),
            this, SLOT(exportCompletedSlot()));
    connect(m_backupManager, SIGNAL(importCompleted()),
            this, SLOT(importCompletedSlot()));
}

BackupDialog::~BackupDialog()
{
    delete ui;
}

bool BackupDialog::backupRestored()
{
    return m_backupFileRestored;
}


//-----------------------------------------------------------------------------
// Public slots
//-----------------------------------------------------------------------------

void BackupDialog::reject()
{
    //stop task
    m_backupManager->stopBackupTask();

    QDialog::reject();
}


//-----------------------------------------------------------------------------
// Private slots
//-----------------------------------------------------------------------------

void BackupDialog::chooseNextButtonClicked()
{
    if (ui->createRadioButton->isChecked()) {
        ui->stackedWidget->setCurrentIndex(1);
        //set file name
        QString documentsDir = QStandardPaths::standardLocations(
                    QStandardPaths::DocumentsLocation).at(0);
        QString fileName = QDateTime::currentDateTime()
                .toString("yyyy_MM_dd_hhmm").append("_backup.syb");
        QString defaultDest = documentsDir + "/" + fileName;
        ui->exportDestLineEdit->setText(defaultDest);
    } else {
        ui->stackedWidget->setCurrentIndex(2);
    }
}

void BackupDialog::exportBackButtonClicked()
{
    ui->stackedWidget->setCurrentIndex(0);
}

void BackupDialog::importBackButtonClicked()
{
    ui->stackedWidget->setCurrentIndex(0);
}

void BackupDialog::exportBrowseButtonClicked()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save File"),
                                                    ui->exportDestLineEdit->text(),
                                                    tr("Symphytum Backup(*.syb)"));
    if (fileName.isEmpty())
        return;
    if (!fileName.contains(".syb"))
        fileName.append(".syb");
    ui->exportDestLineEdit->setText(fileName);
}

void BackupDialog::importBrowseButtonClicked()
{
    QString documentsDir = QStandardPaths::standardLocations(
                QStandardPaths::DocumentsLocation).at(0);
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"),
                                                    documentsDir,
                                                    tr("Symphytum Backup(*.syb)"));
    if (fileName.isEmpty())
        return;
    ui->importDestLineEdit->setText(fileName);
}

void BackupDialog::updateBackupButton()
{
    bool enabled = !ui->exportDestLineEdit->text().isEmpty();
    ui->backupButton->setEnabled(enabled);
}

void BackupDialog::updateRestoreButton()
{
    bool enabled = !ui->importDestLineEdit->text().isEmpty();
    ui->restoreButton->setEnabled(enabled);
}

void BackupDialog::backupButtonClicked()
{
    ui->stackedWidget->setCurrentIndex(3);
    ui->progressLabel->setText(tr("Exporting..."));
    ui->cancelBackupProgressButton->setEnabled(true);
    m_backupManager->startExport(ui->exportDestLineEdit->text());
}

void BackupDialog::restoreButtonClicked()
{
    ui->stackedWidget->setCurrentIndex(3);
    ui->progressLabel->setText(tr("Importing..."));
    ui->cancelBackupProgressButton->setEnabled(false);
    m_backupManager->startImport(ui->importDestLineEdit->text());
}

void BackupDialog::progressSlot(int currentStep, int totalSteps)
{
    ui->progressBar->setRange(0, totalSteps);
    ui->progressBar->setValue(currentStep);
}

void BackupDialog::backupTaskFailed(const QString &error)
{
    ui->resultLabel->setText(tr("Backup operation failed: %1").arg(error));
    ui->progressBar->hide();
    ui->progressLabel->hide();
    ui->cancelBackupProgressButton->setEnabled(true);
}

void BackupDialog::exportCompletedSlot()
{
    ui->resultLabel->setText(tr("Backup completed successfully!"));
    ui->progressBar->hide();
    ui->progressLabel->hide();
    ui->cancelBackupProgressButton->setEnabled(false);
    ui->finishButton->setEnabled(true);
}

void BackupDialog::importCompletedSlot()
{
    ui->resultLabel->setText(tr("Backup restored successfully!"));
    ui->progressBar->hide();
    ui->progressLabel->hide();
    ui->cancelBackupProgressButton->setEnabled(false);
    ui->finishButton->setEnabled(true);

    //set restored property
    m_backupFileRestored = true;
}


//-----------------------------------------------------------------------------
// Private
//-----------------------------------------------------------------------------
