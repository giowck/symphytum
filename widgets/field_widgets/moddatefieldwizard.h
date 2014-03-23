/**
  * \class ModDateFieldWizard
  * \brief Wizard class for creation and configuration of modification date type fields
  * \author Giorgio Wicklein - GIOWISYS Software
  * \date 01/11/2012
  */

#ifndef MODDATEFIELDWIZARD_H
#define MODDATEFIELDWIZARD_H


//-----------------------------------------------------------------------------
// Headers
//-----------------------------------------------------------------------------

#include "abstractfieldwizard.h"


//-----------------------------------------------------------------------------
// Forward declarations
//-----------------------------------------------------------------------------

namespace Ui {
class ModDateFieldWizard;
}


//-----------------------------------------------------------------------------
// ModDateFieldWizard
//-----------------------------------------------------------------------------

class ModDateFieldWizard : public AbstractFieldWizard
{
    Q_OBJECT

public:
    explicit ModDateFieldWizard(const QString &fieldName,
                                 QWidget *parent = 0,
                                 AbstractFieldWizard::EditMode editMode
                                 = AbstractFieldWizard::NewEditMode);
    ~ModDateFieldWizard();

    void getFieldProperties(QString &displayProperties,
                            QString &editProperties,
                            QString &triggerProperties);
    void loadField(const int fieldId, const int collectionId);

private:
    Ui::ModDateFieldWizard *ui;
};

#endif // MODDATEFIELDWIZARD_H
