/**
  * \class CreationDateFieldWizard
  * \brief Wizard class for creation and configuration of creation date type fields
  * \author Giorgio Wicklein - GIOWISYS Software
  * \date 01/11/2012
  */

#ifndef CREATIONDATEFIELDWIZARD_H
#define CREATIONDATEFIELDWIZARD_H


//-----------------------------------------------------------------------------
// Headers
//-----------------------------------------------------------------------------

#include "abstractfieldwizard.h"


//-----------------------------------------------------------------------------
// Forward declarations
//-----------------------------------------------------------------------------

namespace Ui {
class CreationDateFieldWizard;
}


//-----------------------------------------------------------------------------
// CreationDateFieldWizard
//-----------------------------------------------------------------------------

class CreationDateFieldWizard : public AbstractFieldWizard
{
    Q_OBJECT

public:
    explicit CreationDateFieldWizard(const QString &fieldName,
                                 QWidget *parent = 0,
                                 AbstractFieldWizard::EditMode editMode
                                 = AbstractFieldWizard::NewEditMode);
    ~CreationDateFieldWizard();

    void getFieldProperties(QString &displayProperties,
                            QString &editProperties,
                            QString &triggerProperties);
    void loadField(const int fieldId, const int collectionId);

private:
    Ui::CreationDateFieldWizard *ui;
};

#endif // CREATIONDATEFIELDWIZARD_H
