/**
  * \class ExportDialog
  * \brief This dialog is used to export records.
  * \author Giorgio Wicklein - GIOWISYS Software
  * \date 15/11/2012
  */

#ifndef EXPORTDIALOG_H
#define EXPORTDIALOG_H


//-----------------------------------------------------------------------------
// Headers
//-----------------------------------------------------------------------------

#include <QtWidgets/QDialog>


//-----------------------------------------------------------------------------
// Forward declarations
//-----------------------------------------------------------------------------

namespace Ui {
class ExportDialog;
}

class MetadataEngine;


//-----------------------------------------------------------------------------
// ExportDialog
//-----------------------------------------------------------------------------

class ExportDialog : public QDialog
{
    Q_OBJECT
    
public:
    /**
     * @brief Constructor
     * @param collectionId - the collection to export
     * @param selectionRecordIdList - list of record ids to export (if selection)
     * @param parent - parent widget
     */
    explicit ExportDialog(const int collectionId,
                         QList<int> &selectionRecordIdList,
                         QWidget *parent = 0);
    ~ExportDialog();

private slots:
    void nextButtonClicked();
    void backButtonClicked();
    void cancelProgressButtonClicked();
    void exportCSVButtonClicked();
    
private:
    void initUiCSV();
    void initRecordListToExport();
    QString textTypeItemText(const QVariant &data, int fieldId);
    QString numericTypeItemText(const QVariant &data, int fieldId);
    QString dateTypeItemText(const QVariant &data, int fieldId);
    QString checkboxTypeItemText(const QVariant &data, int fieldId);
    QString comboboxTypeItemText(const QVariant &data, int fieldId);
    QString progressTypeItemText(const QVariant &data, int fieldId);
    QString imageTypeItemText(const QVariant &data, int fieldId);
    QString filesTypeItemText(const QVariant &data, int fieldId);

    Ui::ExportDialog *ui;
    QList<int> m_recordList; /**< List of record ids to export */
    int m_collectionId;
    MetadataEngine *m_metadataEngine;
    bool m_exportCancelled;
};

#endif // EXPORTDIALOG_H
