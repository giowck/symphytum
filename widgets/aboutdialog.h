/**
  * \class AboutDialog
  * \brief This is about dialog, showing version info and licenses.
  * \author Giorgio Wicklein - GIOWISYS Software
  * \date 17/11/2012
  */

#ifndef ABOUTDIALOG_H
#define ABOUTDIALOG_H


//-----------------------------------------------------------------------------
// Headers
//-----------------------------------------------------------------------------

#include <QtWidgets/QDialog>


//-----------------------------------------------------------------------------
// Forward declarations
//-----------------------------------------------------------------------------

namespace Ui {
class AboutDialog;
}


//-----------------------------------------------------------------------------
// AboutDialog
//-----------------------------------------------------------------------------

class AboutDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit AboutDialog(QWidget *parent = 0);
    ~AboutDialog();
    
private:
    Ui::AboutDialog *ui;
};

#endif // ABOUTDIALOG_H
