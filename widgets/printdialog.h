/**
  * \class PrintDialog
  * \brief This dialog is used to print or save to PDF records.
  * \author Giorgio Wicklein - GIOWISYS Software
  * \date 12/11/2012
  */

#ifndef PRINTDIALOG_H
#define PRINTDIALOG_H


//-----------------------------------------------------------------------------
// Headers
//-----------------------------------------------------------------------------

#include <QtWidgets/QDialog>


//-----------------------------------------------------------------------------
// Forward declarations
//-----------------------------------------------------------------------------

namespace Ui {
class PrintDialog;
}

class MetadataEngine;


//-----------------------------------------------------------------------------
// PrintDialog
//-----------------------------------------------------------------------------

class PrintDialog : public QDialog
{
    Q_OBJECT
    
public:
    /**
     * @brief Constructor
     * @param collectionId - the collection to print
     * @param selectionRecordIdList - list of record ids to print (if print selection)
     * @param parent - parent widget
     */
    explicit PrintDialog(const int collectionId,
                         QList<int> &selectionRecordIdList,
                         QWidget *parent = 0);
    ~PrintDialog();

private slots:
    void pdfButtonClicked();
    void printButtonClicked();
    void cancelPrintButtonClicked();
    
private:
    void initRecordListToPrint();
    void print(bool pdf = false, QString pdfOutputPath = QString());
    QString textTypeItemHtml(const QVariant &data, int fieldId);
    QString numericTypeItemHtml(const QVariant &data, int fieldId);
    QString dateTypeItemHtml(const QVariant &data, int fieldId);
    QString checkboxTypeItemHtml(const QVariant &data, int fieldId);
    QString comboboxTypeItemHtml(const QVariant &data, int fieldId);
    QString progressTypeItemHtml(const QVariant &data, int fieldId);
    QString imageTypeItemHtml(const QVariant &data, int fieldId);
    QString filesTypeItemHtml(const QVariant &data, int fieldId);

    Ui::PrintDialog *ui;
    QList<int> m_recordList; /**< List of record ids to print */
    int m_collectionId;
    MetadataEngine *m_metadataEngine;
    bool m_printCancelled;
};

#endif // PRINTDIALOG_H
