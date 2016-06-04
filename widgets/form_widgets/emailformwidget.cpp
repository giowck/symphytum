/*
 *  Copyright (c) 2016 Giorgio Wicklein <giowckln@gmail.com>
 */

//-----------------------------------------------------------------------------
// Hearders
//-----------------------------------------------------------------------------

#include "emailformwidget.h"
#include "../../utils/platformcolorservice.h"
#include "../../utils/metadatapropertiesparser.h"
#include "../../utils/formwidgetvalidator.h"
#include "../../components/metadataengine.h"

#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QVBoxLayout>
#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtGui/QDesktopServices>
#include <QtCore/QUrl>


//-----------------------------------------------------------------------------
// Public
//-----------------------------------------------------------------------------

EmailFormWidget::EmailFormWidget(QWidget *parent) :
    AbstractFormWidget(parent), m_markEmpty(false)
{
    m_fieldNameLabel = new QLabel("Invalid Name", this);
    m_mainLayout = new QVBoxLayout(this);
    m_lineEdit = new QLineEdit(this);
    m_openURLAction = m_lineEdit->addAction(QIcon(":/images/icons/mail.png"),
                                            QLineEdit::TrailingPosition);
    m_openURLAction->setToolTip(tr("Write new email"));

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
    connect(m_openURLAction, SIGNAL(triggered()),
            this, SLOT(openURLActionTriggered()));

    updateFocusPolicy();
}

void EmailFormWidget::setFieldName(const QString &name)
{
    m_fieldNameLabel->setText(name);
}

QString EmailFormWidget::getFieldName() const
{
    return m_fieldNameLabel->text();
}

void EmailFormWidget::clearData()
{
    m_lineEdit->clear();
}

void EmailFormWidget::setData(const QVariant &data)
{
    m_lineEdit->setText(data.toString());

    //update style sheet according to display properties
    updateStyleSheet();
}

QVariant EmailFormWidget::getData() const
{
    return m_lineEdit->text();
}

void EmailFormWidget::loadMetadataDisplayProperties(const QString &metadata)
{
    MetadataPropertiesParser parser(metadata);
    if (metadata.size() == 0) return;

    QString v;

    v = parser.getValue("markEmpty");
    if (v == "1") m_markEmpty = true;
}


//-----------------------------------------------------------------------------
// Protected slots
//-----------------------------------------------------------------------------

void EmailFormWidget::validateData()
{
    bool valid;

    QString editMetadata = MetadataEngine::getInstance().getFieldProperties(
                MetadataEngine::EditProperty, getFieldId());
    FormWidgetValidator validator(editMetadata, MetadataEngine::URLTextType);
    QString errorMessage;

    valid = validator.validate(getData(), errorMessage);

    if (valid) {
        emit dataEdited();
    } else {
        //restore last valid value
        m_lineEdit->undo();

        //inform FormView that the widget needs attention
        //by animating the widget
        emit requiresAttention(errorMessage);
    }
}



//-----------------------------------------------------------------------------
// Private slots
//-----------------------------------------------------------------------------

void EmailFormWidget::openURLActionTriggered()
{
    QDesktopServices::openUrl(QString("mailto:").append(m_lineEdit->text()));
}


//-----------------------------------------------------------------------------
// Private
//-----------------------------------------------------------------------------

void EmailFormWidget::updateStyleSheet()
{
    QColor c = PlatformColorService::getHighlightColor();
    QString style;

    //generate style
    style.append(
                "QLineEdit {"
                "border-radius: 7px;"
                "background-color: palette(base);"
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
}

void EmailFormWidget::updateFocusPolicy()
{
    m_lineEdit->setFocusPolicy(Qt::ClickFocus);
    setFocusProxy(m_lineEdit);
    setFocusPolicy(Qt::StrongFocus);
}
