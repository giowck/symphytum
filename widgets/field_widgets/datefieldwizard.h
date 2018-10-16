/**
  * \class DateFieldWizard
  * \brief Wizard class for creation and configuration of date type fields
  * \author Giorgio Wicklein - GIOWISYS Software
  * \date 29/10/2012
  */

#ifndef DATEFIELDWIZARD_H
#define DATEFIELDWIZARD_H


//-----------------------------------------------------------------------------
// Headers
//-----------------------------------------------------------------------------

#include "abstractfieldwizard.h"


//-----------------------------------------------------------------------------
// Forward declarations
//-----------------------------------------------------------------------------

namespace Ui {
class DateFieldWizard;
}


//-----------------------------------------------------------------------------
// DateFieldWizard
//-----------------------------------------------------------------------------

class DateFieldWizard : public AbstractFieldWizard
{
    Q_OBJECT

public:
    explicit DateFieldWizard(const QString &fieldName,
                               QWidget *parent = nullptr,
                               AbstractFieldWizard::EditMode editMode
                               = AbstractFieldWizard::NewEditMode);
    ~DateFieldWizard();

    void getFieldProperties(QString &displayProperties,
                            QString &editProperties,
                            QString &triggerProperties);
    void loadField(const int fieldId, const int collectionId);

private slots:
    void finishButtonClicked();

private:
    Ui::DateFieldWizard *ui;
    bool m_alarmOnDate;
};

#endif // DATEFIELDWIZARD_H
