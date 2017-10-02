/*
 *  Copyright (c) 2012 Giorgio Wicklein <giowckln@gmail.com>
 */

//-----------------------------------------------------------------------------
// Hearders
//-----------------------------------------------------------------------------

#include "numberformwidget.h"
#include "../../utils/platformcolorservice.h"
#include "../../utils/metadatapropertiesparser.h"
#include "../../utils/formwidgetvalidator.h"
#include "../../components/metadataengine.h"

#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QVBoxLayout>
#include <QtCore/QVariant>
#include <QtCore/QLocale>


//-----------------------------------------------------------------------------
// Public
//-----------------------------------------------------------------------------

NumberFormWidget::NumberFormWidget(QWidget *parent) :
    AbstractFormWidget(parent), m_precision(6), m_markNegative(false),
    m_markEmpty(false), m_displayMode(AutoDetect)
{
    m_fieldNameLabel = new QLabel("Invalid Name", this);
    m_mainLayout = new QVBoxLayout(this);
    m_lineEdit = new QLineEdit(this);

    //static styling
    m_fieldNameLabel->setStyleSheet("QLabel {color: gray;}");

    //on mac disable focus rect around rounded borders
    m_lineEdit->setAttribute(Qt::WA_MacShowFocusRect, 0);

    m_mainLayout->addWidget(m_fieldNameLabel);
    m_mainLayout->addWidget(m_lineEdit);
    m_mainLayout->addStretch();

    this->heightUnits = 1;
    this->widthUnits = 1;

    //connections
    connect(m_lineEdit, SIGNAL(editingFinished()),
            this, SLOT(validateData()));

    setupFocusPolicy();
}

void NumberFormWidget::setFieldName(const QString &name)
{
    m_fieldNameLabel->setText(name);
}

QString NumberFormWidget::getFieldName() const
{
    return m_fieldNameLabel->text();
}

void NumberFormWidget::clearData()
{
    m_lineEdit->clear();
}

void NumberFormWidget::setData(const QVariant &data)
{
    QString value;
    QLocale locale;
    bool empty = data.toString().isEmpty();

    switch (m_displayMode) {
    case AutoDetect:
        if (!empty)
            value = data.toString();
        break;
    case DecimalNormal:
        if (!empty)
            value = QString::number(data.toDouble(), 'f', m_precision);
        break;
    case ScientificNotation:
        if (!empty)
            value = QString::number(data.toDouble(), 'e', m_precision);
        break;
    }

    //make use of the correct decimal point char
    value.replace(".", locale.decimalPoint());

    m_lineEdit->setText(value);

    //update style sheet according to display properties
    updateStyleSheet();
}

QVariant NumberFormWidget::getData() const
{
    QString s = m_lineEdit->text();

    //determine which of '.' or ',' is decimal point
    QChar notDecimal;
    QLocale locale;
    if (locale.decimalPoint() == '.')
        notDecimal = ',';
    else
        notDecimal = '.';

    //remove all invalid characters ('e' and 'E' is for scientific notation)
    s.remove(QRegExp(QString("[A-DF-Za-df-z\\%1]").arg(notDecimal)));

    //toDouble handles automagically (LOL)
    //all conversions between the different formats
    bool ok;
    double d = s.toDouble(&ok);
    if (ok)
        return d;
    else
        return ""; //empty value
}

void NumberFormWidget::loadMetadataDisplayProperties(const QString &metadata)
{   
    MetadataPropertiesParser parser(metadata);
    if (metadata.size() == 0) return;

    QString v;

    v = parser.getValue("displayMode");
    if (v == "auto")
        m_displayMode = AutoDetect;
    else if (v == "decimal")
        m_displayMode = DecimalNormal;
    else if (v == "scientific")
        m_displayMode = ScientificNotation;

    v = parser.getValue("precision");
    bool ok; int p;
    p = v.toInt(&ok);
    if (ok) m_precision = p;

    v = parser.getValue("markNegative");
    if (v == "1") m_markNegative = true;

    v = parser.getValue("markEmpty");
    if (v == "1") m_markEmpty = true;
}


//-----------------------------------------------------------------------------
// Protected slots
//-----------------------------------------------------------------------------

void NumberFormWidget::validateData()
{
    bool valid;

    QString editMetadata = MetadataEngine::getInstance().getFieldProperties(
                MetadataEngine::EditProperty, getFieldId());
    FormWidgetValidator validator(editMetadata, MetadataEngine::NumericType);
    QString errorMessage;

    valid = validator.validate(m_lineEdit->text(), errorMessage);

    if (valid) {
        emit dataEdited();
    } else {
        m_lineEdit->undo(); //restore last valid value

        //inform FormView that the widget needs attention
        //by animating the widget
        emit requiresAttention(errorMessage);
    }
}


//-----------------------------------------------------------------------------
// Private
//-----------------------------------------------------------------------------

void NumberFormWidget::updateStyleSheet()
{
    QColor c = PlatformColorService::getHighlightColor();
    QString style;

    //generate style
    style.append(
                "QLineEdit {"
                "border-radius: 7px;"
                "padding: 5px;"
                );
    if (m_markNegative) {
        if (m_lineEdit->text().toDouble() < 0.0)
            style.append("color: red;");
    }
    if (m_markEmpty && m_lineEdit->text().isEmpty()) {
        style.append(
                    "background-color: #FFDFDF;"
                    "border: 2px solid #FF7979;"
                    );
    } else {
        style.append("border: 2px solid lightgray;");
    }
    style.append(QString(
                     "}"
                     "QLineEdit:focus { "
                     "border: 2px solid rgb(%1, %2, %3);}")
                 .arg(c.red()).arg(c.green()).arg(c.blue()));

    m_lineEdit->setStyleSheet(style);
}

void NumberFormWidget::setupFocusPolicy()
{
    m_lineEdit->setFocusPolicy(Qt::ClickFocus);
    setFocusProxy(m_lineEdit);
    setFocusPolicy(Qt::StrongFocus);
}
