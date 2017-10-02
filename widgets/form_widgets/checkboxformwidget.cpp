/*
 *  Copyright (c) 2012 Giorgio Wicklein <giowckln@gmail.com>
 */

//-----------------------------------------------------------------------------
// Hearders
//-----------------------------------------------------------------------------

#include "checkboxformwidget.h"
#include "../../utils/metadatapropertiesparser.h"
#include "../../utils/formwidgetvalidator.h"
#include "../../components/metadataengine.h"

#include <QtWidgets/QLabel>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QVBoxLayout>
#include <QtCore/QVariant>


//-----------------------------------------------------------------------------
// Public
//-----------------------------------------------------------------------------

CheckboxFormWidget::CheckboxFormWidget(QWidget *parent) :
    AbstractFormWidget(parent)
{
    m_mainLayout = new QVBoxLayout(this);
    m_checkbox = new QCheckBox(this);

    m_mainLayout->addStretch();
    m_mainLayout->addWidget(m_checkbox);
    m_mainLayout->addStretch();

    this->heightUnits = 1;
    this->widthUnits = 1;

    //connections
    connect(m_checkbox, SIGNAL(clicked()),
            this, SLOT(validateData()));

    setupFocusPolicy();
}

void CheckboxFormWidget::setFieldName(const QString &name)
{
    m_checkbox->setText(name);
}

QString CheckboxFormWidget::getFieldName() const
{
    return m_checkbox->text();
}

void CheckboxFormWidget::clearData()
{
    m_checkbox->setChecked(false);
}

void CheckboxFormWidget::setData(const QVariant &data)
{
    m_checkbox->setChecked(data.toInt());
}

QVariant CheckboxFormWidget::getData() const
{
    int checked = 0;

    if (m_checkbox->isChecked())
        checked = 1;

    return checked;
}

void CheckboxFormWidget::loadMetadataDisplayProperties(const QString &metadata)
{
    //none for now
    Q_UNUSED(metadata);
}


//-----------------------------------------------------------------------------
// Protected slots
//-----------------------------------------------------------------------------

void CheckboxFormWidget::validateData()
{
    //always valid
    emit dataEdited();
}


//-----------------------------------------------------------------------------
// Private
//-----------------------------------------------------------------------------

void CheckboxFormWidget::setupFocusPolicy()
{
    m_checkbox->setFocusPolicy(Qt::ClickFocus);
    setFocusProxy(m_checkbox);
    setFocusPolicy(Qt::StrongFocus);
}
