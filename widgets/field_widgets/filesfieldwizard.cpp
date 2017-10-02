/*
 *  Copyright (c) 2012 Giorgio Wicklein <giowckln@gmail.com>
 */

//-----------------------------------------------------------------------------
// Hearders
//-----------------------------------------------------------------------------

#include "filesfieldwizard.h"
#include "ui_filesfieldwizard.h"

#include "../../components/metadataengine.h"
#include "../../utils/metadatapropertiesparser.h"


//-----------------------------------------------------------------------------
// Public
//-----------------------------------------------------------------------------

FilesFieldWizard::FilesFieldWizard(const QString &fieldName,
                                   QWidget *parent,
                                   AbstractFieldWizard::EditMode editMode) :
    AbstractFieldWizard(fieldName, parent, editMode),
    ui(new Ui::FilesFieldWizard)
{
    ui->setupUi(this);

    connect(ui->backButton, SIGNAL(clicked()),
            this, SIGNAL(backSignal()));
    connect(ui->finishButton, SIGNAL(clicked()),
            this, SIGNAL(finishSignal()));

    ui->finishButton->setFocus();
}

FilesFieldWizard::~FilesFieldWizard()
{
    delete ui;
}

void FilesFieldWizard::getFieldProperties(QString &displayProperties,
                                          QString &editProperties,
                                          QString &triggerProperties)
{
    //create display properties metadata string
    if (ui->fileTypeCheckBox->isChecked())
        displayProperties.append("showFileType:1;");
    if (ui->dateCheckBox->isChecked())
        displayProperties.append("showAddedDate:1;");

    //create edit properties metadata string
    //nothing for now
    Q_UNUSED(editProperties);

    //create trigger properties metadata string
    //nothing for now
    Q_UNUSED(triggerProperties);
}

void FilesFieldWizard::loadField(const int fieldId, const int collectionId)
{
    AbstractFieldWizard::loadField(fieldId, collectionId);

    MetadataEngine *meta = &MetadataEngine::getInstance();

    //display properties
    QString displayProperties = meta->getFieldProperties(meta->DisplayProperty,
                                                         fieldId, collectionId);
    MetadataPropertiesParser displayParser(displayProperties);
    if (displayParser.size()) {
        if (displayParser.getValue("showFileType") == "1")
            ui->fileTypeCheckBox->setChecked(true);
        if (displayParser.getValue("showAddedDate") == "1")
            ui->dateCheckBox->setChecked(true);
    }
}


//-----------------------------------------------------------------------------
// Private
//-----------------------------------------------------------------------------
