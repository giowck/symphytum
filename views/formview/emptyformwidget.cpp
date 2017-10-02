/*
 *  Copyright (c) 2012 Giorgio Wicklein <giowckln@gmail.com>
 */

//-----------------------------------------------------------------------------
// Hearders
//-----------------------------------------------------------------------------

#include "emptyformwidget.h"
#include "ui_emptyformwidget.h"


//-----------------------------------------------------------------------------
// Public
//-----------------------------------------------------------------------------

EmptyFormWidget::EmptyFormWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::EmptyFormWidget)
{
    ui->setupUi(this);
}

EmptyFormWidget::~EmptyFormWidget()
{
    delete ui;
}

void EmptyFormWidget::setState(State s)
{
    bool recordMissing, fieldMissing, collectionMissing, modelMissing;
    recordMissing = fieldMissing = collectionMissing = modelMissing = false;

    switch (s) {
    case AllMissing:
        collectionMissing = fieldMissing = recordMissing = true;
        break;
    case FieldMissing:
        fieldMissing = recordMissing = true;
        break;
    case RecordMissing:
        recordMissing = true;
        break;
    case MissingModel:
        modelMissing = true;
    }

    ui->noCollectionIcon->setVisible(collectionMissing);
    ui->noCollectionLabel->setVisible(collectionMissing);
    ui->noFieldIcon->setVisible(fieldMissing);
    ui->noFieldLabel->setVisible(fieldMissing);
    ui->noRecordIcon->setVisible(recordMissing);
    ui->noRecordLabel->setVisible(recordMissing);

    if (modelMissing)
        ui->label->hide();
    else
        ui->label->show();
}
