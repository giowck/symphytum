/**
  * \class AddFieldDialog
  * \brief This dialog is used to add new fields to a collection.
  *        After the field type and field name selection, the apropriate
  *        field wizard is launched to configure the chosen field type.
  *        Each field wizard will take care of the real field creation
  *        algorithm and this class manages only the launch of the apropriate wizard
  *        and its termination. The central component of this widget is the
  *        stacked widget, which contains the start page to select a field type
  *        and name, the next pages are generated according to selected field type
  *        wizard widget.
  * \author Giorgio Wicklein - GIOWISYS Software
  * \date 20/07/2012
  */

#ifndef ADDFIELDDIALOG_H
#define ADDFIELDDIALOG_H


//-----------------------------------------------------------------------------
// Headers
//-----------------------------------------------------------------------------

#include <QtWidgets/QDialog>

#include "abstractfieldwizard.h"
#include "../../components/metadataengine.h"


//-----------------------------------------------------------------------------
// Forward declarations
//-----------------------------------------------------------------------------

namespace Ui {
class AddFieldDialog;
}


//-----------------------------------------------------------------------------
// AddFieldDialog
//-----------------------------------------------------------------------------

class AddFieldDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AddFieldDialog(QWidget *parent = nullptr);
    ~AddFieldDialog();

    /**
     * Prepares the dialog for the specified operation
     * @param mode - the EditMode @see EditMode
     * @param fieldId - specify only if mode is modify or duplicate
     * @param collectionId - specify only if mode is modify or duplicate
     */
    void setCreationMode(AbstractFieldWizard::EditMode mode,
                         int fieldId = 0,
                         int collectionId = 0);

private slots:
    void cancelButtonClicked();
    void nextButtonClicked();
    void updateFieldDescription();
    void updateNextButtonState();

    /** Go to previous page of the stacked widget */
    void backSlot();

    /** The wizard has finished the config process */
    void finishSlot();

    /** Set predefined field names if available */
    void setDefaultNameSlot();
    
private:
    void init();
    void createConnections();

    /** Clean up fields */
    void cleanDialog();

    /** Delete current wizard */
    void deleteCurrentWizard();

    /**
     * Load field properties relevant to the start page only
     * such as field name and field type
     * @param fieldId - the field id/number
     * @param collectionId - if not specified, current will be used
     */
    void loadFieldStartProperties(int fieldId, int collectionId = 0);

    Ui::AddFieldDialog *ui;
    AbstractFieldWizard::EditMode m_currentMode;
    AbstractFieldWizard *m_currentWizard;
    MetadataEngine::FieldType m_fieldType;
    int m_fieldId;
    int m_collectionId;
};

#endif // ADDFIELDDIALOG_H
