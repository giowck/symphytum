/*
 *  Copyright (c) 2012 Giorgio Wicklein <giowckln@gmail.com>
 */

//-----------------------------------------------------------------------------
// Hearders
//-----------------------------------------------------------------------------

#include "checkboxfieldwizard.h"
#include "ui_checkboxfieldwizard.h"

#include "../../components/metadataengine.h"
#include "../../utils/metadatapropertiesparser.h"


//-----------------------------------------------------------------------------
// Public
//-----------------------------------------------------------------------------

CheckboxFieldWizard::CheckboxFieldWizard(const QString &fieldName,
                                         QWidget *parent,
                                         AbstractFieldWizard::EditMode editMode) :
    AbstractFieldWizard(fieldName, parent, editMode),
    ui(new Ui::CheckboxFieldWizard)
{
    ui->setupUi(this);

    connect(ui->backButton, SIGNAL(clicked()),
            this, SIGNAL(backSignal()));
    connect(ui->finishButton, SIGNAL(clicked()),
            this, SIGNAL(finishSignal()));

    ui->finishButton->setFocus();
}

CheckboxFieldWizard::~CheckboxFieldWizard()
{
    delete ui;
}

void CheckboxFieldWizard::getFieldProperties(QString &displayProperties,
                                         QString &editProperties,
                                         QString &triggerProperties)
{
    //create display properties metadata string
    //nothing for now
    Q_UNUSED(displayProperties);

    //create edit properties metadata string
    //nothing for now
    Q_UNUSED(editProperties);

    //create trigger properties metadata string
    //nothing for now
    Q_UNUSED(triggerProperties);
}

void CheckboxFieldWizard::loadField(const int fieldId, const int collectionId)
{
    AbstractFieldWizard::loadField(fieldId, collectionId);

    //no properties for now
}


//-----------------------------------------------------------------------------
// Private
//-----------------------------------------------------------------------------
