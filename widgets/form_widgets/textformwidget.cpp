/*
 *  Copyright (c) 2012 Giorgio Wicklein <giowckln@gmail.com>
 */

//-----------------------------------------------------------------------------
// Hearders
//-----------------------------------------------------------------------------

#include "textformwidget.h"
#include "../../widgets/textarea.h"
#include "../../utils/platformcolorservice.h"
#include "../../utils/metadatapropertiesparser.h"
#include "../../utils/formwidgetvalidator.h"
#include "../../components/metadataengine.h"

#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QVBoxLayout>
#include <QtCore/QVariant>


//-----------------------------------------------------------------------------
// Public
//-----------------------------------------------------------------------------

TextFormWidget::TextFormWidget(QWidget *parent) :
    AbstractFormWidget(parent), m_multiLine(false), m_markEmpty(false)
{
    m_fieldNameLabel = new QLabel("Invalid Name", this);
    m_mainLayout = new QVBoxLayout(this);
    m_lineEdit = new QLineEdit(this);
    m_textArea = new TextArea(this);

    //static styling
    m_fieldNameLabel->setStyleSheet("QLabel {color: gray;}");

    //on mac disable focus rect around rounded borders
    m_lineEdit->setAttribute(Qt::WA_MacShowFocusRect, 0);
    m_textArea->setAttribute(Qt::WA_MacShowFocusRect, 0);

    m_mainLayout->addWidget(m_fieldNameLabel);
    m_mainLayout->addWidget(m_lineEdit);
    m_mainLayout->addWidget(m_textArea);
    m_mainLayout->addStretch();

    this->heightUnits = 1;
    this->widthUnits = 1;

    //since heightUnit = 1, make only line edit visible
    m_textArea->setVisible(false);

    //connections
    connect(m_lineEdit, SIGNAL(editingFinished()),
            this, SLOT(validateData()));
    connect(m_textArea, SIGNAL(editingFinished()),
            this, SLOT(validateData()));

    updateFocusPolicy();
}

void TextFormWidget::setFieldName(const QString &name)
{
    m_fieldNameLabel->setText(name);
}

QString TextFormWidget::getFieldName() const
{
    return m_fieldNameLabel->text();
}

void TextFormWidget::clearData()
{
    m_textArea->clear();
    m_lineEdit->clear();
}

void TextFormWidget::setData(const QVariant &data)
{
    if (m_multiLine)
        m_textArea->setText(data.toString());
    else
        m_lineEdit->setText(data.toString());

    //update style sheet according to display properties
    updateStyleSheet();
}

QVariant TextFormWidget::getData() const
{
    if (m_multiLine)
        return m_textArea->toPlainText();
    else
        return m_lineEdit->text();
}

void TextFormWidget::loadMetadataDisplayProperties(const QString &metadata)
{
    MetadataPropertiesParser parser(metadata);
    if (metadata.size() == 0) return;

    QString v;

    v = parser.getValue("markEmpty");
    if (v == "1") m_markEmpty = true;
}

void TextFormWidget::setHeightUnits(int heightUnits)
{
    AbstractFormWidget::setHeightUnits(heightUnits);

    //replace line edit by text edit or viceversa
    if (heightUnits > 1) {
        //check state to avoid overwriting of the original text
        if (!m_multiLine) {
            m_multiLine = true;
            m_textArea->setText(m_lineEdit->text());
            m_mainLayout->takeAt(3); //remove spacer item
        }
    } else {
        //check state to avoid overwriting of the original text
        if (m_multiLine) {
            m_multiLine = false;
            m_lineEdit->setText(m_textArea->toPlainText());
            m_mainLayout->addStretch(); //add spacer item again
        }
    }

    m_lineEdit->setVisible(!m_multiLine);
    m_textArea->setVisible(m_multiLine);
    updateFocusPolicy();
}


//-----------------------------------------------------------------------------
// Protected slots
//-----------------------------------------------------------------------------

void TextFormWidget::validateData()
{
    bool valid;

    QString editMetadata = MetadataEngine::getInstance().getFieldProperties(
                MetadataEngine::EditProperty, getFieldId());
    FormWidgetValidator validator(editMetadata, MetadataEngine::TextType);
    QString errorMessage;

    valid = validator.validate(getData(), errorMessage);

    if (valid) {
        emit dataEdited();
    } else {
        //restore last valid value
        if (m_multiLine)
            m_textArea->undo();
        else
            m_lineEdit->undo();

        //inform FormView that the widget needs attention
        //by animating the widget
        emit requiresAttention(errorMessage);
    }
}


//-----------------------------------------------------------------------------
// Private
//-----------------------------------------------------------------------------

void TextFormWidget::updateStyleSheet()
{
    QColor c = PlatformColorService::getHighlightColor();
    QString style;

    //generate style
    style.append(
                "QLineEdit {"
                "border-radius: 7px;"
                "background-color: palette(base);" //avoid transparency for padding on TextArea
                "padding: 5px;"
                );
    if (m_markEmpty && getData().toString().isEmpty()) {
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
    m_textArea->setStyleSheet(style.replace("QLineEdit", "QTextEdit"));
}

void TextFormWidget::updateFocusPolicy()
{
    if (m_multiLine) {
        m_textArea->setFocusPolicy(Qt::ClickFocus);
        setFocusProxy(m_textArea);
        setFocusPolicy(Qt::StrongFocus);
    } else {
        m_lineEdit->setFocusPolicy(Qt::ClickFocus);
        setFocusProxy(m_lineEdit);
        setFocusPolicy(Qt::StrongFocus);
    }
}
