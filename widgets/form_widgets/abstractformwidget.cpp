/*
 *  Copyright (c) 2012 Giorgio Wicklein <giowckln@gmail.com>
 */

//-----------------------------------------------------------------------------
// Hearders
//-----------------------------------------------------------------------------

#include "abstractformwidget.h"

#include <QtCore/QString>
#include <QtCore/QVariant>


//-----------------------------------------------------------------------------
// Public
//-----------------------------------------------------------------------------

AbstractFormWidget::AbstractFormWidget(QWidget *parent) :
    QWidget(parent), fieldId(-1)
{
    this->widthUnits = 1;
    this->heightUnits = 1;
}

AbstractFormWidget::~AbstractFormWidget()
{

}

int AbstractFormWidget::getHeightUnits() const
{
    return heightUnits;
}

int AbstractFormWidget::getWidthUnits() const
{
    return widthUnits;
}

void AbstractFormWidget::setHeightUnits(int heightUnits)
{
    this->heightUnits = heightUnits;
}

void AbstractFormWidget::setWidthUnits(int widthUnits)
{
    this->widthUnits = widthUnits;
}

void AbstractFormWidget::setFieldId(int columnId)
{
    this->fieldId = columnId;
}

int AbstractFormWidget::getFieldId()
{
    return this->fieldId;
}

bool AbstractFormWidget::operator == (const AbstractFormWidget& other) const
{
    //compare addresses
    return (this == &other);
}

bool AbstractFormWidget::operator != (const AbstractFormWidget& other) const
{
    return !(*this == other);
}

bool AbstractFormWidget::showHighlightSearchResults(const QString &searchString)
{
    Q_UNUSED(searchString);
    return false;
}
