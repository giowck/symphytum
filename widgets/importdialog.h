/**
  * \class ImportDialog
  * \brief This dialog is used to export records.
  * \author Giorgio Wicklein - GIOWISYS Software
  * \date 15/11/2012
  */

#ifndef IMPORTDIALOG_H
#define IMPORTDIALOG_H


//-----------------------------------------------------------------------------
// Headers
//-----------------------------------------------------------------------------

#include <QtWidgets/QDialog>


//-----------------------------------------------------------------------------
// Forward declarations
//-----------------------------------------------------------------------------

namespace Ui {
class ImportDialog;
}

class MetadataEngine;
class QTextStream;


//-----------------------------------------------------------------------------
// ImportDialog
//-----------------------------------------------------------------------------

class ImportDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit ImportDialog(QWidget *parent = nullptr);
    ~ImportDialog();

private slots:
    void nextButtonClicked();
    void backButtonClicked();
    void cancelImportButtonClicked();
    void updateImportCSVButton();
    void importCSVButtonClicked();
    
private:
    void initUiCSV();
    QString readCSVLine(QTextStream *in,
                        const QChar &embeddedEscape);
    QStringList parseCSVLine(const QString &csvData,
                             const QChar &separator,
                             const QChar &embeddedEscape);

    Ui::ImportDialog *ui;
    MetadataEngine *m_metadataEngine;
    bool m_importCancelled;
};

#endif // IMPORTDIALOG_H
