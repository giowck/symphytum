/**
  * \class EmailFieldWizard
  * \brief Wizard class for creation and configuration of email text type fields
  * \author Giorgio Wicklein
  * \date 28/05/2016
  */

#ifndef EMAILFIELDWIZARD_H
#define EMAILFIELDWIZARD_H


//-----------------------------------------------------------------------------
// Headers
//-----------------------------------------------------------------------------

#include "abstractfieldwizard.h"


//-----------------------------------------------------------------------------
// Forward declarations
//-----------------------------------------------------------------------------

namespace Ui {
class EmailFieldWizard;
}


//-----------------------------------------------------------------------------
// EmailFieldWizard
//-----------------------------------------------------------------------------

class EmailFieldWizard : public AbstractFieldWizard
{
    Q_OBJECT

public:
    explicit EmailFieldWizard(const QString &fieldName,
                             QWidget *parent = 0,
                             AbstractFieldWizard::EditMode editMode
                             = AbstractFieldWizard::NewEditMode);
    ~EmailFieldWizard();

    void getFieldProperties(QString &displayProperties,
                            QString &editProperties,
                            QString &triggerProperties);
    void loadField(const int fieldId, const int collectionId);

private:
    Ui::EmailFieldWizard *ui;
};

#endif // EMAILFIELDWIZARD_H
