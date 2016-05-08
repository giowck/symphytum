/**
  * \class AlarmListDialog
  * \brief This dialog is used to show triggered alarms
  * \author Giorgio Wicklein - GIOWISYS Software
  * \date 06/11/2012
  */

#ifndef ALARMLISTDIALOG_H
#define ALARMLISTDIALOG_H


//-----------------------------------------------------------------------------
// Headers
//-----------------------------------------------------------------------------

#include <QtWidgets/QDialog>


//-----------------------------------------------------------------------------
// Forward declarations
//-----------------------------------------------------------------------------

namespace Ui {
class AlarmListDialog;
}

class AlarmManager;
class MetadataEngine;


//-----------------------------------------------------------------------------
// AlarmListDialog
//-----------------------------------------------------------------------------

class AlarmListDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit AlarmListDialog(QWidget *parent = 0);
    ~AlarmListDialog();

    /** Check for alarm again */
    void reloadAlarmList();

signals:
    /** Show record request */
    void showRecord(int collectionId,
                    int fieldId,
                    int recordId);

private slots:
    void showButtonClicked();
    void removeButtonClicked();
    
private:
    void populateAlarmList();
    void updateEmptyState();

    Ui::AlarmListDialog *ui;
    AlarmManager *m_alarmManager;
    MetadataEngine *m_metadataEngine;
};

#endif // ALARMLISTDIALOG_H
