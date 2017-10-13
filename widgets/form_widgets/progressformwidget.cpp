/*
 *  Copyright (c) 2012 Giorgio Wicklein <giowckln@gmail.com>
 */

//-----------------------------------------------------------------------------
// Hearders
//-----------------------------------------------------------------------------

#include "progressformwidget.h"
#include "../../utils/metadatapropertiesparser.h"
#include "../../utils/formwidgetvalidator.h"
#include "../../components/metadataengine.h"

#include <QtWidgets/QLabel>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QProgressBar>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QVBoxLayout>
#include <QtCore/QVariant>


//-----------------------------------------------------------------------------
// Public
//-----------------------------------------------------------------------------

ProgressFormWidget::ProgressFormWidget(QWidget *parent) :
    AbstractFormWidget(parent),
    m_maxValue(100),
    m_lastValidProgress(0)
{
    m_fieldNameLabel = new QLabel("Invalid Name", this);
    m_mainLayout = new QVBoxLayout(this);
    m_progressBar = new QProgressBar(this);
    m_spinBox = new QSpinBox(this);

    //static styling
    m_fieldNameLabel->setStyleSheet("QLabel {color: gray;}");

    m_progressLayout = new QHBoxLayout;
    m_progressLayout->addWidget(m_spinBox);
    m_progressLayout->addWidget(m_progressBar);

    m_mainLayout->addWidget(m_fieldNameLabel);
    m_mainLayout->addLayout(m_progressLayout);
    m_mainLayout->addStretch();

    m_spinBox->setSizePolicy(QSizePolicy::Fixed,
                             QSizePolicy::Fixed);
    m_progressBar->setSizePolicy(QSizePolicy::MinimumExpanding,
                                 QSizePolicy::Fixed);

    this->heightUnits = 1;
    this->widthUnits = 2;

    //connections
    connect(m_spinBox, SIGNAL(editingFinished()),
            this, SLOT(validateData()));

    setupFocusPolicy();
}

void ProgressFormWidget::setFieldName(const QString &name)
{
    m_fieldNameLabel->setText(name);
}

QString ProgressFormWidget::getFieldName() const
{
    return m_fieldNameLabel->text();
}

void ProgressFormWidget::clearData()
{
    m_spinBox->clear();
    m_spinBox->setValue(0);
    m_progressBar->setValue(0);
    m_lastValidProgress = 0;
}

void ProgressFormWidget::setData(const QVariant &data)
{
    int value;
    bool validData;

    value = data.toInt(&validData);
    if (validData) {
        m_spinBox->setValue(value);
        m_progressBar->setValue(value);
        m_lastValidProgress = value;
    } else {
        m_spinBox->setValue(0);
        m_progressBar->setValue(0);
        m_lastValidProgress = 0;
    }
}

QVariant ProgressFormWidget::getData() const
{
   return m_spinBox->value();
}

void ProgressFormWidget::loadMetadataDisplayProperties(const QString &metadata)
{
    MetadataPropertiesParser parser(metadata);
    if (metadata.size() == 0) return;

    QString v;

    v = parser.getValue("max");
    int max;
    max = v.toInt();
    m_maxValue = max;

    m_spinBox->setButtonSymbols(QAbstractSpinBox::PlusMinus);
    m_spinBox->setRange(0, max);
    m_spinBox->setSuffix(tr(" of %1").arg(max));

    m_progressBar->setRange(0, max);
}


//-----------------------------------------------------------------------------
// Protected
//-----------------------------------------------------------------------------

void ProgressFormWidget::focusOutEvent(QFocusEvent *event)
{
    validateData();

    QWidget::focusOutEvent(event);
}


//-----------------------------------------------------------------------------
// Protected slots
//-----------------------------------------------------------------------------

void ProgressFormWidget::validateData()
{
    bool valid;

    QString editMetadata = MetadataEngine::getInstance().getFieldProperties(
                MetadataEngine::EditProperty, getFieldId());
    FormWidgetValidator validator(editMetadata, MetadataEngine::ProgressType);
    QString errorMessage;

    valid = validator.validate(getData(), errorMessage);

    if (valid) {
        m_lastValidProgress = m_spinBox->value();
        emit dataEdited();
    } else {
        //restore last valid value
        m_spinBox->setValue(m_lastValidProgress);

        //inform FormView that the widget needs attention
        //by animating the widget
        emit requiresAttention(errorMessage);
    }
}


//-----------------------------------------------------------------------------
// Private
//-----------------------------------------------------------------------------

void ProgressFormWidget::setupFocusPolicy()
{
    m_spinBox->setFocusPolicy(Qt::ClickFocus);
    setFocusProxy(m_spinBox);
    setFocusPolicy(Qt::StrongFocus);
}
