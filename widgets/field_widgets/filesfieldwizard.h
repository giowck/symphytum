/**
  * \class FilesFieldWizard
  * \brief Wizard class for creation and configuration of files type fields
  * \author Giorgio Wicklein - GIOWISYS Software
  * \date 23/10/2012
  */

#ifndef FILESFIELDWIZARD_H
#define FILESFIELDWIZARD_H


//-----------------------------------------------------------------------------
// Headers
//-----------------------------------------------------------------------------

#include "abstractfieldwizard.h"


//-----------------------------------------------------------------------------
// Forward declarations
//-----------------------------------------------------------------------------

namespace Ui {
class FilesFieldWizard;
}


//-----------------------------------------------------------------------------
// FilesFieldWizard
//-----------------------------------------------------------------------------

class FilesFieldWizard : public AbstractFieldWizard
{
    Q_OBJECT

public:
    explicit FilesFieldWizard(const QString &fieldName,
                             QWidget *parent = nullptr,
                             AbstractFieldWizard::EditMode editMode
                             = AbstractFieldWizard::NewEditMode);
    ~FilesFieldWizard();

    void getFieldProperties(QString &displayProperties,
                            QString &editProperties,
                            QString &triggerProperties);
    void loadField(const int fieldId, const int collectionId);

private:
    Ui::FilesFieldWizard *ui;
};

#endif // FILESFIELDWIZARD_H
