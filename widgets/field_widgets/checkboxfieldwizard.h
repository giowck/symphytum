/**
  * \class CheckboxFieldWizard
  * \brief Wizard class for creation and configuration of checkbox type fields
  * \author Giorgio Wicklein - GIOWISYS Software
  * \date 06/10/2012
  */

#ifndef CHECKBOXFIELDWIZARD_H
#define CHECKBOXFIELDWIZARD_H


//-----------------------------------------------------------------------------
// Headers
//-----------------------------------------------------------------------------

#include "abstractfieldwizard.h"


//-----------------------------------------------------------------------------
// Forward declarations
//-----------------------------------------------------------------------------

namespace Ui {
class CheckboxFieldWizard;
}


//-----------------------------------------------------------------------------
// CheckboxFieldWizard
//-----------------------------------------------------------------------------

class CheckboxFieldWizard : public AbstractFieldWizard
{
    Q_OBJECT

public:
    explicit CheckboxFieldWizard(const QString &fieldName,
                                 QWidget *parent = 0,
                                 AbstractFieldWizard::EditMode editMode
                                 = AbstractFieldWizard::NewEditMode);
    ~CheckboxFieldWizard();

    void getFieldProperties(QString &displayProperties,
                            QString &editProperties,
                            QString &triggerProperties);
    void loadField(const int fieldId, const int collectionId);

private:
    Ui::CheckboxFieldWizard *ui;
};

#endif // CHECKBOXFIELDWIZARD_H
