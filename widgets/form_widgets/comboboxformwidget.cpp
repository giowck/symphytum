/*
 *  Copyright (c) 2012 Giorgio Wicklein <giowckln@gmail.com>
 */

//-----------------------------------------------------------------------------
// Hearders
//-----------------------------------------------------------------------------

#include "comboboxformwidget.h"
#include "../../utils/metadatapropertiesparser.h"
#include "../../utils/formwidgetvalidator.h"
#include "../../components/metadataengine.h"

#include <QtWidgets/QLabel>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QVBoxLayout>
#include <QtCore/QVariant>


//-----------------------------------------------------------------------------
// Public
//-----------------------------------------------------------------------------

ComboboxFormWidget::ComboboxFormWidget(QWidget *parent) :
    AbstractFormWidget(parent), m_default(-1),
    m_markEmpty(false)
{
    m_fieldNameLabel = new QLabel("Invalid Name", this);
    m_mainLayout = new QVBoxLayout(this);
    m_comboBox = new QComboBox(this);
    m_comboBox->setCurrentIndex(-1);

    m_mainLayout->addWidget(m_fieldNameLabel);
    m_mainLayout->addWidget(m_comboBox);
    m_mainLayout->addStretch();

    this->heightUnits = 1;
    this->widthUnits = 1;

    //connections
    connect(m_comboBox, SIGNAL(activated(int)),
            this, SLOT(validateData()));

    setupFocusPolicy();
}

void ComboboxFormWidget::setFieldName(const QString &name)
{
    m_fieldNameLabel->setText(name);
}

QString ComboboxFormWidget::getFieldName() const
{
    return m_fieldNameLabel->text();
}

void ComboboxFormWidget::clearData()
{
    m_comboBox->setCurrentIndex(-1);
}

void ComboboxFormWidget::setData(const QVariant &data)
{
    int value;
    bool validData;

    value = data.toInt(&validData);
    if (validData) {
        m_comboBox->setCurrentIndex(value);
    } else {
        m_comboBox->setCurrentIndex(m_default);
    }

    //update style sheet according to display properties
    updateStyleSheet();
}

QVariant ComboboxFormWidget::getData() const
{
   return m_comboBox->currentIndex();
}

void ComboboxFormWidget::loadMetadataDisplayProperties(const QString &metadata)
{
    MetadataPropertiesParser parser(metadata);
    if (metadata.size() == 0) return;

    QString v;

    //load items
    v = parser.getValue("items");
    m_itemNameList = v.split(',', QString::SkipEmptyParts);
    foreach (QString s, m_itemNameList) {
        //replace some escape codes
        s.replace("\\comma", ",");
        s.replace("\\colon", ":");
        s.replace("\\semicolon", ";");
        m_comboBox->addItem(s);
    }

    v = parser.getValue("default");
    bool ok; int d;
    d = v.toInt(&ok);
    if (ok) m_default = d;

    v = parser.getValue("markEmpty");
    if (v == "1") m_markEmpty = true;
}


//-----------------------------------------------------------------------------
// Protected slots
//-----------------------------------------------------------------------------

void ComboboxFormWidget::validateData()
{
    //always valid
    emit dataEdited();
}


//-----------------------------------------------------------------------------
// Private
//-----------------------------------------------------------------------------

void ComboboxFormWidget::updateStyleSheet()
{
    QString style;

    //generate style
    style.append(
                "QLabel {"
                "color: gray;"
                );
    if (m_markEmpty && (m_comboBox->currentIndex() == -1)) {
        style.append(
                    "background-color: #FFDFDF;"
                    "border: 2px solid #FF7979;"
                    );
    }
    style.append("}");

    m_fieldNameLabel->setStyleSheet(style);
}

void ComboboxFormWidget::setupFocusPolicy()
{
    m_comboBox->setFocusPolicy(Qt::ClickFocus);
    setFocusProxy(m_comboBox);
    setFocusPolicy(Qt::StrongFocus);
}
