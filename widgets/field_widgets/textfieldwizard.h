/**
  * \class TextFieldWizard
  * \brief Wizard class for creation and configuration of text type fields
  * \author Giorgio Wicklein - GIOWISYS Software
  * \date 20/07/2012
  */

#ifndef TEXTFIELDWIZARD_H
#define TEXTFIELDWIZARD_H


//-----------------------------------------------------------------------------
// Headers
//-----------------------------------------------------------------------------

#include "abstractfieldwizard.h"


//-----------------------------------------------------------------------------
// Forward declarations
//-----------------------------------------------------------------------------

namespace Ui {
class TextFieldWizard;
}


//-----------------------------------------------------------------------------
// TextFieldWizard
//-----------------------------------------------------------------------------

class TextFieldWizard : public AbstractFieldWizard
{
    Q_OBJECT
    
public:
    explicit TextFieldWizard(const QString &fieldName,
                             QWidget *parent = 0,
                             AbstractFieldWizard::EditMode editMode
                             = AbstractFieldWizard::NewEditMode);
    ~TextFieldWizard();

    void getFieldProperties(QString &displayProperties,
                            QString &editProperties,
                            QString &triggerProperties);
    void loadField(const int fieldId, const int collectionId);
    
private:
    Ui::TextFieldWizard *ui;
};

#endif // TEXTFIELDWIZARD_H
