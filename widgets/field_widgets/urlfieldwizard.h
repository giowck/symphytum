/**
  * \class URLFieldWizard
  * \brief Wizard class for creation and configuration of URL text type fields
  * \author Giorgio Wicklein
  * \date 28/05/2016
  */

#ifndef URLFIELDWIZARD_H
#define URLFIELDWIZARD_H


//-----------------------------------------------------------------------------
// Headers
//-----------------------------------------------------------------------------

#include "abstractfieldwizard.h"


//-----------------------------------------------------------------------------
// Forward declarations
//-----------------------------------------------------------------------------

namespace Ui {
class URLFieldWizard;
}


//-----------------------------------------------------------------------------
// URLFieldWizard
//-----------------------------------------------------------------------------

class URLFieldWizard : public AbstractFieldWizard
{
    Q_OBJECT

public:
    explicit URLFieldWizard(const QString &fieldName,
                             QWidget *parent = 0,
                             AbstractFieldWizard::EditMode editMode
                             = AbstractFieldWizard::NewEditMode);
    ~URLFieldWizard();

    void getFieldProperties(QString &displayProperties,
                            QString &editProperties,
                            QString &triggerProperties);
    void loadField(const int fieldId, const int collectionId);

private:
    Ui::URLFieldWizard *ui;
};

#endif // URLFIELDWIZARD_H
