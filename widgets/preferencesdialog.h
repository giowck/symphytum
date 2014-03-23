/**
  * \class PreferencesDialog
  * \brief This dialog is used to edit application settings.
  * \author Giorgio Wicklein - GIOWISYS Software
  * \date 16/09/2012
  */

#ifndef PREFERENCESDIALOG_H
#define PREFERENCESDIALOG_H


//-----------------------------------------------------------------------------
// Headers
//-----------------------------------------------------------------------------

#include <QtGui/QDialog>


//-----------------------------------------------------------------------------
// Forward declarations
//-----------------------------------------------------------------------------

namespace Ui {
class PreferencesDialog;
}

class SettingsManager;


//-----------------------------------------------------------------------------
// PreferencesDialog
//-----------------------------------------------------------------------------

class PreferencesDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit PreferencesDialog(QWidget *parent = 0);
    ~PreferencesDialog();

    /** Return whether the cloud config has been edited */
    bool cloudSyncChanged();

    /** Return whether the software reset button was clicked */
    bool softwareResetActivated();

    /** Return whether the appearence config changed */
    bool appearanceChanged();

private slots:
    void currentCategoryChanged();
    void updatesComboBoxChanged();
    void cloudStateComboBoxChanged();
    void cloundUnlinkButtonClicked();
    void softwareResetButtonClicked();
    void formViewColorComboChanged();
    void darkToolbarAmbianceCheckChanged();
    
private:
    void initSettings();
    void loadSettings();

    Ui::PreferencesDialog *ui;
    SettingsManager *m_settingsManager;
    bool m_cloudChanged;
    bool m_softwareReset;
    bool m_appearanceChanged;
};

#endif // PREFERENCESDIALOG_H
