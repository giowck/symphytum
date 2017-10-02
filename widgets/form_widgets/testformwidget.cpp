/*
 *  Copyright (c) 2012 Giorgio Wicklein <giowckln@gmail.com>
 */

//-----------------------------------------------------------------------------
// Hearders
//-----------------------------------------------------------------------------

#include "testformwidget.h"

#include <QtWidgets/QLabel>
#include <QtWidgets/QVBoxLayout>
#include <QtCore/QVariant>


//-----------------------------------------------------------------------------
// Public
//-----------------------------------------------------------------------------

TestFormWidget::TestFormWidget(QWidget *parent) :
    AbstractFormWidget(parent)
{
    testLabel = new QLabel("Test Form Widget", this);
    testLabel->setAlignment(Qt::AlignCenter);
    mainLayout = new QVBoxLayout(this);

    mainLayout->addWidget(testLabel);
    this->setStyleSheet("border: 1px solid black; border-radius: 5px;");

    this->heightUnits = 1;
    this->widthUnits = 1;
    this->data = QString("");
}

void TestFormWidget::setFieldName(const QString &name)
{
    this->testLabel->setText(name);
}

QString TestFormWidget::getFieldName() const
{
    return this->testLabel->text();
}

void TestFormWidget::clearData()
{
    this->data.clear();
}

void TestFormWidget::setData(const QVariant &data)
{
    this->data = data.toString();
}

QVariant TestFormWidget::getData() const
{
    return this->data;
}
