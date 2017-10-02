/*
 *  Copyright (c) 2012 Giorgio Wicklein <giowckln@gmail.com>
 */

//-----------------------------------------------------------------------------
// Hearders
//-----------------------------------------------------------------------------

#include "textfieldwizard.h"
#include "ui_textfieldwizard.h"

#include "../../components/metadataengine.h"
#include "../../utils/metadatapropertiesparser.h"


//-----------------------------------------------------------------------------
// Public
//-----------------------------------------------------------------------------

TextFieldWizard::TextFieldWizard(const QString &fieldName,
                                 QWidget *parent,
                                 AbstractFieldWizard::EditMode editMode) :
    AbstractFieldWizard(fieldName, parent, editMode),
    ui(new Ui::TextFieldWizard)
{
    ui->setupUi(this);

    connect(ui->backButton, SIGNAL(clicked()),
            this, SIGNAL(backSignal()));
    connect(ui->finishButton, SIGNAL(clicked()),
            this, SIGNAL(finishSignal()));

    ui->finishButton->setFocus();
}

TextFieldWizard::~TextFieldWizard()
{
    delete ui;
}

void TextFieldWizard::getFieldProperties(QString &displayProperties,
                                         QString &editProperties,
                                         QString &triggerProperties)
{
    //create display properties metadata string
    if (ui->requiredFieldCheckBox->isChecked())
        displayProperties.append("markEmpty:1;");

    //create edit properties metadata string
    if (ui->requiredFieldCheckBox->isChecked())
        editProperties.append("noEmpty:1;");

    //create trigger properties metadata string
    //nothing for now
    Q_UNUSED(triggerProperties);
}

void TextFieldWizard::loadField(const int fieldId, const int collectionId)
{
    AbstractFieldWizard::loadField(fieldId, collectionId);

    MetadataEngine *meta = &MetadataEngine::getInstance();

    //edit properties
    QString editProperties = meta->getFieldProperties(meta->EditProperty,
                                                      fieldId, collectionId);
    MetadataPropertiesParser editParser(editProperties);
    if (editParser.size()) {
        if (editParser.getValue("noEmpty") == "1")
            ui->requiredFieldCheckBox->setChecked(true);
    }
}


//-----------------------------------------------------------------------------
// Private
//-----------------------------------------------------------------------------
