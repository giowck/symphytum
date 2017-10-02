/*
 *  Copyright (c) 2012 Giorgio Wicklein <giowckln@gmail.com>
 */

//-----------------------------------------------------------------------------
// Hearders
//-----------------------------------------------------------------------------

#include "numberfieldwizard.h"
#include "ui_numberfieldwizard.h"

#include "../../components/metadataengine.h"
#include "../../utils/metadatapropertiesparser.h"


//-----------------------------------------------------------------------------
// Public
//-----------------------------------------------------------------------------

NumberFieldWizard::NumberFieldWizard(const QString &fieldName,
                                     QWidget *parent,
                                     AbstractFieldWizard::EditMode editMode) :
    AbstractFieldWizard(fieldName, parent, editMode),
    ui(new Ui::NumberFieldWizard)
{
    ui->setupUi(this);

    connect(ui->backButton, SIGNAL(clicked()),
            this, SIGNAL(backSignal()));
    connect(ui->finishButton, SIGNAL(clicked()),
            this, SIGNAL(finishSignal()));
    connect(ui->notationComboBox, SIGNAL(currentIndexChanged(int)),
            this, SLOT(updateNotationBox()));

    ui->finishButton->setFocus();
}

NumberFieldWizard::~NumberFieldWizard()
{
    delete ui;
}

void NumberFieldWizard::getFieldProperties(QString &displayProperties,
                                           QString &editProperties,
                                           QString &triggerProperties)
{
    //create display properties metadata string
    if (ui->requiredFieldCheckBox->isChecked())
        displayProperties.append("markEmpty:1;");
    if (ui->markNegativeCheckBox->isChecked())
        displayProperties.append("markNegative:1;");
    displayProperties.append(QString("precision:%1;")
                             .arg(ui->precisionSpinBox->value()));
    //display mode
    QString displayMode;
    switch (ui->notationComboBox->currentIndex()) {
    case 0:
        displayMode = "auto";
        break;
    case 1:
        displayMode = "decimal";
        break;
    case 2:
        displayMode = "scientific";
        break;
    default:
        displayMode = "auto";
        break;
    }
    displayProperties.append("displayMode:" + displayMode + ";");

    //create edit properties metadata string
    if (ui->requiredFieldCheckBox->isChecked())
        editProperties.append("noEmpty:1;");

    //create trigger properties metadata string
    //nothing for now
    Q_UNUSED(triggerProperties);
}

void NumberFieldWizard::loadField(const int fieldId, const int collectionId)
{
    AbstractFieldWizard::loadField(fieldId, collectionId);

    MetadataEngine *meta = &MetadataEngine::getInstance();

    //display properties
    QString displayProperties = meta->getFieldProperties(meta->DisplayProperty,
                                                      fieldId, collectionId);
    MetadataPropertiesParser displayParser(displayProperties);
    if (displayParser.size()) {
        if (displayParser.getValue("markNegative") == "1")
            ui->markNegativeCheckBox->setChecked(true);
        ui->precisionSpinBox->setValue(displayParser.getValue("precision").toInt());

        //display mode
        QString m = displayParser.getValue("displayMode");
        if (m == QString("auto"))
            ui->notationComboBox->setCurrentIndex(0);
        else if (m == QString("decimal"))
            ui->notationComboBox->setCurrentIndex(1);
        else if (m == QString("scientific"))
            ui->notationComboBox->setCurrentIndex(2);
    }

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
// Private slots
//-----------------------------------------------------------------------------

void NumberFieldWizard::updateNotationBox()
{
    bool enabled = ui->notationComboBox->currentIndex() != 0;
    ui->precisionGroupBox->setEnabled(enabled);
}
