/**
  * \class BackupDialog
  * \brief This dialog is used to backup/restore full databases.
  * \author Giorgio Wicklein - GIOWISYS Software
  * \date 04/10/2012
  */

#ifndef BACKUPDIALOG_H
#define BACKUPDIALOG_H


//-----------------------------------------------------------------------------
// Headers
//-----------------------------------------------------------------------------

#include <QtWidgets/QDialog>


//-----------------------------------------------------------------------------
// Forward declarations
//-----------------------------------------------------------------------------

namespace Ui {
class BackupDialog;
}

class BackupManager;


//-----------------------------------------------------------------------------
// BackupDialog
//-----------------------------------------------------------------------------

class BackupDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit BackupDialog(QWidget *parent = nullptr);
    ~BackupDialog();

    /** Whether a backup file has been restored */
    bool backupRestored();

public slots:
    void reject();

private slots:
    void chooseNextButtonClicked();
    void exportBackButtonClicked();
    void importBackButtonClicked();
    void exportBrowseButtonClicked();
    void importBrowseButtonClicked();
    void updateBackupButton();
    void updateRestoreButton();
    void backupButtonClicked();
    void restoreButtonClicked();
    void progressSlot(int currentStep, int totalSteps);
    void backupTaskFailed(const QString &error);
    void exportCompletedSlot();
    void importCompletedSlot();
    
private:
    Ui::BackupDialog *ui;
    bool m_backupFileRestored;
    BackupManager *m_backupManager;
};

#endif // BACKUPDIALOG_H
