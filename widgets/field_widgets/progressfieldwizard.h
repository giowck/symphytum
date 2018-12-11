/**
  * \class ProgressFieldWizard
  * \brief Wizard class for creation and configuration of progress type fields
  * \author Giorgio Wicklein - GIOWISYS Software
  * \date 19/10/2012
  */

#ifndef PROGRESSFIELDWIZARD_H
#define PROGRESSFIELDWIZARD_H


//-----------------------------------------------------------------------------
// Headers
//-----------------------------------------------------------------------------

#include "abstractfieldwizard.h"


//-----------------------------------------------------------------------------
// Forward declarations
//-----------------------------------------------------------------------------

namespace Ui {
class ProgressFieldWizard;
}


//-----------------------------------------------------------------------------
// ProgressFieldWizard
//-----------------------------------------------------------------------------

class ProgressFieldWizard : public AbstractFieldWizard
{
    Q_OBJECT

public:
    explicit ProgressFieldWizard(const QString &fieldName,
                               QWidget *parent = nullptr,
                               AbstractFieldWizard::EditMode editMode
                               = AbstractFieldWizard::NewEditMode);
    ~ProgressFieldWizard();

    void getFieldProperties(QString &displayProperties,
                            QString &editProperties,
                            QString &triggerProperties);
    void loadField(const int fieldId, const int collectionId);

private:
    Ui::ProgressFieldWizard *ui;
};

#endif // PROGRESSFIELDWIZARD_H
