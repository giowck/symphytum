/*
 *  Copyright (c) 2012 Giorgio Wicklein <giowckln@gmail.com>
 */

//-----------------------------------------------------------------------------
// Hearders
//-----------------------------------------------------------------------------

#include "imagefieldwizard.h"
#include "ui_imagefieldwizard.h"

#include "../../components/metadataengine.h"
#include "../../utils/metadatapropertiesparser.h"


//-----------------------------------------------------------------------------
// Public
//-----------------------------------------------------------------------------

ImageFieldWizard::ImageFieldWizard(const QString &fieldName,
                                   QWidget *parent,
                                   AbstractFieldWizard::EditMode editMode) :
    AbstractFieldWizard(fieldName, parent, editMode),
    ui(new Ui::ImageFieldWizard)
{
    ui->setupUi(this);

    connect(ui->backButton, SIGNAL(clicked()),
            this, SIGNAL(backSignal()));
    connect(ui->finishButton, SIGNAL(clicked()),
            this, SIGNAL(finishSignal()));

    ui->finishButton->setFocus();
}

ImageFieldWizard::~ImageFieldWizard()
{
    delete ui;
}

void ImageFieldWizard::getFieldProperties(QString &displayProperties,
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

void ImageFieldWizard::loadField(const int fieldId, const int collectionId)
{
    AbstractFieldWizard::loadField(fieldId, collectionId);

    //no properties for now
}


//-----------------------------------------------------------------------------
// Private
//-----------------------------------------------------------------------------
