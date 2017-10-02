/*
 *  Copyright (c) 2012 Giorgio Wicklein <giowckln@gmail.com>
 */

//-----------------------------------------------------------------------------
// Hearders
//-----------------------------------------------------------------------------

#include "creationdateformwidget.h"
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

CreationDateFormWidget::CreationDateFormWidget(QWidget *parent) :
    AbstractFormWidget(parent)
{
    m_fieldNameLabel = new QLabel("Invalid Name", this);
    m_mainLayout = new QVBoxLayout(this);
    m_lineEdit = new QLineEdit(this);
    m_lineEdit->setReadOnly(true);

    //static styling
    m_fieldNameLabel->setStyleSheet("QLabel {color: gray;}");
    QColor c = PlatformColorService::getHighlightColor();
    QString style;
    //generate style
    style.append(
                "QLineEdit {"
                "border-radius: 7px;"
                "padding: 5px;"
                "border: 2px solid lightgray;"
                "}"
                );
    style.append(QString(
                     "QLineEdit:focus { "
                     "border: 2px solid rgb(%1, %2, %3);}")
                 .arg(c.red()).arg(c.green()).arg(c.blue()));
    m_lineEdit->setStyleSheet(style);

    //on mac disable focus rect around rounded borders
    m_lineEdit->setAttribute(Qt::WA_MacShowFocusRect, 0);

    m_mainLayout->addWidget(m_fieldNameLabel);
    m_mainLayout->addWidget(m_lineEdit);
    m_mainLayout->addStretch();

    //date format
    QLocale locale;
    m_dateFormat = locale.dateTimeFormat(QLocale::ShortFormat);

    this->heightUnits = 1;
    this->widthUnits = 1;

    setupFocusPolicy();
}

void CreationDateFormWidget::setFieldName(const QString &name)
{
    m_fieldNameLabel->setText(name);
}

QString CreationDateFormWidget::getFieldName() const
{
    return m_fieldNameLabel->text();
}

void CreationDateFormWidget::clearData()
{
    m_lineEdit->clear();
}

void CreationDateFormWidget::setData(const QVariant &data)
{
    if (!data.isNull()) {
        m_dateTime = data.toDateTime();
        m_lineEdit->setText(m_dateTime.toString(m_dateFormat));
    } else {
        m_lineEdit->clear();
    }
}

QVariant CreationDateFormWidget::getData() const
{
    return m_dateTime;
}

void CreationDateFormWidget::loadMetadataDisplayProperties(const QString &metadata)
{
    MetadataPropertiesParser parser(metadata);
    if (metadata.size() == 0) return;

    QString v;

    QLocale locale;
    v = parser.getValue("dateFormat");
    if (v == "1")
        m_dateFormat = locale.dateTimeFormat(QLocale::ShortFormat);
    else if (v == "2")
        m_dateFormat = locale.dateFormat(QLocale::ShortFormat);
    else if (v == "3")
        m_dateFormat = "ddd MMM d hh:mm yyyy";
    else if (v == "4")
        m_dateFormat = "ddd MMM d yyyy";
    else if (v == "5")
        m_dateFormat = "yyyy-MM-dd hh:mm";
    else if (v == "6")
        m_dateFormat = "yyyy-MM-dd";
}


//-----------------------------------------------------------------------------
// Protected slots
//-----------------------------------------------------------------------------

void CreationDateFormWidget::validateData()
{
    //always valid
    emit dataEdited();
}


//-----------------------------------------------------------------------------
// Private
//-----------------------------------------------------------------------------

void CreationDateFormWidget::setupFocusPolicy()
{
    m_lineEdit->setFocusPolicy(Qt::ClickFocus);
    setFocusProxy(m_lineEdit);
    setFocusPolicy(Qt::StrongFocus);
}
