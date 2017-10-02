/*
 *  Copyright (c) 2012 Giorgio Wicklein <giowckln@gmail.com>
 */

//-----------------------------------------------------------------------------
// Hearders
//-----------------------------------------------------------------------------

#include "progressfieldwizard.h"
#include "ui_progressfieldwizard.h"

#include "../../components/metadataengine.h"
#include "../../utils/metadatapropertiesparser.h"


//-----------------------------------------------------------------------------
// Public
//-----------------------------------------------------------------------------

ProgressFieldWizard::ProgressFieldWizard(const QString &fieldName,
                                         QWidget *parent,
                                         AbstractFieldWizard::EditMode editMode) :
    AbstractFieldWizard(fieldName, parent, editMode),
    ui(new Ui::ProgressFieldWizard)
{
    ui->setupUi(this);

    connect(ui->backButton, SIGNAL(clicked()),
            this, SIGNAL(backSignal()));
    connect(ui->finishButton, SIGNAL(clicked()),
            this, SIGNAL(finishSignal()));
}

ProgressFieldWizard::~ProgressFieldWizard()
{
    delete ui;
}

void ProgressFieldWizard::getFieldProperties(QString &displayProperties,
                                           QString &editProperties,
                                           QString &triggerProperties)
{
    //create display properties metadata string
    displayProperties.append("max:" +
                             QString::number(ui->spinBox->value())
                             + ";");

    //create edit properties metadata string
    //nothing for now
    Q_UNUSED(editProperties);

    //create trigger properties metadata string
    //nothing for now
    Q_UNUSED(triggerProperties);
}

void ProgressFieldWizard::loadField(const int fieldId, const int collectionId)
{
    AbstractFieldWizard::loadField(fieldId, collectionId);

    MetadataEngine *meta = &MetadataEngine::getInstance();

    //display properties
    QString displayProperties = meta->getFieldProperties(meta->DisplayProperty,
                                                      fieldId, collectionId);
    MetadataPropertiesParser displayParser(displayProperties);
    if (displayParser.size()) {
        bool ok;
        int max = displayParser.getValue("max").toInt(&ok);
        if (ok)
            ui->spinBox->setValue(max);
    }
}
