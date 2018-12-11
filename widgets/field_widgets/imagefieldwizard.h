/**
  * \class ImageFieldWizard
  * \brief Wizard class for creation and configuration of image type fields
  * \author Giorgio Wicklein - GIOWISYS Software
  * \date 08/10/2012
  */

#ifndef IMAGEFIELDWIZARD_H
#define IMAGEFIELDWIZARD_H


//-----------------------------------------------------------------------------
// Headers
//-----------------------------------------------------------------------------

#include "abstractfieldwizard.h"


//-----------------------------------------------------------------------------
// Forward declarations
//-----------------------------------------------------------------------------

namespace Ui {
class ImageFieldWizard;
}


//-----------------------------------------------------------------------------
// ImageFieldWizard
//-----------------------------------------------------------------------------

class ImageFieldWizard : public AbstractFieldWizard
{
    Q_OBJECT

public:
    explicit ImageFieldWizard(const QString &fieldName,
                              QWidget *parent = nullptr,
                              AbstractFieldWizard::EditMode editMode
                              = AbstractFieldWizard::NewEditMode);
    ~ImageFieldWizard();

    void getFieldProperties(QString &displayProperties,
                            QString &editProperties,
                            QString &triggerProperties);
    void loadField(const int fieldId, const int collectionId);

private:
    Ui::ImageFieldWizard *ui;
};

#endif // IMAGEFIELDWIZARD_H
