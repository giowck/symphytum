/**
  * \class NumberFieldWizard
  * \brief Wizard class for creation and configuration of number type fields
  * \author Giorgio Wicklein - GIOWISYS Software
  * \date 31/07/2012
  */

#ifndef NUMBERFIELDWIZARD_H
#define NUMBERFIELDWIZARD_H


//-----------------------------------------------------------------------------
// Headers
//-----------------------------------------------------------------------------

#include "abstractfieldwizard.h"


//-----------------------------------------------------------------------------
// Forward declarations
//-----------------------------------------------------------------------------

namespace Ui {
class NumberFieldWizard;
}


//-----------------------------------------------------------------------------
// NumberFieldWizard
//-----------------------------------------------------------------------------

class NumberFieldWizard : public AbstractFieldWizard
{
    Q_OBJECT

public:
    explicit NumberFieldWizard(const QString &fieldName,
                               QWidget *parent = 0,
                               AbstractFieldWizard::EditMode editMode
                               = AbstractFieldWizard::NewEditMode);
    ~NumberFieldWizard();

    void getFieldProperties(QString &displayProperties,
                            QString &editProperties,
                            QString &triggerProperties);
    void loadField(const int fieldId, const int collectionId);

private slots:
    void updateNotationBox();

private:
    Ui::NumberFieldWizard *ui;
};

#endif // NUMBERFIELDWIZARD_H
