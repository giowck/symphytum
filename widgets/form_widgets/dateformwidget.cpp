/*
 *  Copyright (c) 2012 Giorgio Wicklein <giowckln@gmail.com>
 */

//-----------------------------------------------------------------------------
// Hearders
//-----------------------------------------------------------------------------

#include "dateformwidget.h"
#include "../../utils/platformcolorservice.h"
#include "../../utils/metadatapropertiesparser.h"
#include "../../components/metadataengine.h"
#include "../../views/formview/formview.h"
#include "../../components/alarmmanager.h"

#include <QtWidgets/QLabel>
#include <QtWidgets/QDateTimeEdit>
#include <QtCore/QDateTime>
#include <QtWidgets/QVBoxLayout>
#include <QtCore/QVariant>
#include <QtCore/QLocale>


//-----------------------------------------------------------------------------
// Public
//-----------------------------------------------------------------------------

DateFormWidget::DateFormWidget(QWidget *parent) :
    AbstractFormWidget(parent)
{
    m_fieldNameLabel = new QLabel("Invalid Name", this);
    m_dateTimeEdit = new QDateTimeEdit(this);
    m_mainLayout = new QVBoxLayout(this);

    //static styling
    m_fieldNameLabel->setStyleSheet("QLabel {color: gray;}");
    QColor c = PlatformColorService::getHighlightColor();
    QString style;
    //generate style
    style.append(
                "QDateTimeEdit {"
                "border-radius: 7px;"
                "padding: 5px;"
                "border: 2px solid lightgray;"
                "}"
                );
    style.append(QString(
                     "QDateTimeEdit:focus { "
                     "border: 2px solid rgb(%1, %2, %3);}")
                 .arg(c.red()).arg(c.green()).arg(c.blue()));
    m_dateTimeEdit->setStyleSheet(style);

    //on mac disable focus rect around rounded borders
    m_dateTimeEdit->setAttribute(Qt::WA_MacShowFocusRect, 0);

    m_mainLayout->addWidget(m_fieldNameLabel);
    m_mainLayout->addWidget(m_dateTimeEdit);
    m_mainLayout->addStretch();

    this->heightUnits = 1;
    this->widthUnits = 1;

    //connections
    connect(m_dateTimeEdit, SIGNAL(editingFinished()),
            this, SLOT(editingFinishedSlot()));

    setupFocusPolicy();
}

void DateFormWidget::setFieldName(const QString &name)
{
    m_fieldNameLabel->setText(name);
}

QString DateFormWidget::getFieldName() const
{
    return m_fieldNameLabel->text();
}

void DateFormWidget::clearData()
{
    m_dateTimeEdit->setDateTime(QDateTime(QDate(2000, 01, 01), QTime(00, 00)));
}

void DateFormWidget::setData(const QVariant &data)
{
    if (!data.isNull())
        m_dateTimeEdit->setDateTime(data.toDateTime());
    else
        clearData();
}

QVariant DateFormWidget::getData() const
{
    return m_dateTimeEdit->dateTime();
}

void DateFormWidget::loadMetadataDisplayProperties(const QString &metadata)
{
    MetadataPropertiesParser parser(metadata);
    if (metadata.size() == 0) return;

    QString v;

    QLocale locale;
    QString dateFormat = locale.dateTimeFormat(QLocale::ShortFormat);
    v = parser.getValue("dateFormat");
    if (v == "1")
        dateFormat = locale.dateTimeFormat(QLocale::ShortFormat);
    else if (v == "2")
        dateFormat = locale.dateFormat(QLocale::ShortFormat);
    else if (v == "3")
        dateFormat = "ddd MMM d hh:mm yyyy";
    else if (v == "4")
        dateFormat = "ddd MMM d yyyy";
    else if (v == "5")
        dateFormat = "yyyy-MM-dd hh:mm";
    else if (v == "6")
        dateFormat = "yyyy-MM-dd";

    //setup date time edit
    m_dateTimeEdit->setCalendarPopup(true);
    m_dateTimeEdit->setDisplayFormat(dateFormat);
}


//-----------------------------------------------------------------------------
// Protected slots
//-----------------------------------------------------------------------------

void DateFormWidget::validateData()
{
    //always valid
    emit dataEdited();
}

void DateFormWidget::editingFinishedSlot()
{
    //Get current field id (column) and record id
    int fieldId = -1;
    int recordId = -1;
    FormView *view = 0;
    QWidget *parent = parentWidget();
    if (parent)
        view = qobject_cast<FormView*>(parent->parentWidget());
    if (view) {
        int row = view->getCurrentRow();
        fieldId = view->getCurrentColumn();
        if (row != -1) {
            bool ok;
            int i = view->model()->data(view->model()->index(row, 0)).toInt(&ok);
            if (ok)
                recordId = i;
        }
    }

    //check if alarm trigger property is enabled
    //if it is, update alarm trigger in alarms table
    if (fieldId != -1) {
        MetadataEngine *meta = &MetadataEngine::getInstance();
        MetadataPropertiesParser parser(meta->getFieldProperties(
                                            MetadataEngine::TriggerProperty,
                                            fieldId));
        if (parser.size() > 0) {
            bool alarmTrigger = parser.getValue("alarmOnDate") == "1";

            //add or update alarm
            if ((recordId != -1) && (alarmTrigger)) {
                QDateTime dateTime(m_dateTimeEdit->dateTime());
                AlarmManager a(this);
                a.addOrUpdateAlarm(meta->getCurrentCollectionId(),
                                   fieldId, recordId, dateTime);
            }
        }
    }

    //save
    validateData();
}


//-----------------------------------------------------------------------------
// Private
//-----------------------------------------------------------------------------

void DateFormWidget::setupFocusPolicy()
{
    m_dateTimeEdit->setFocusPolicy(Qt::ClickFocus);
    setFocusProxy(m_dateTimeEdit);
    setFocusPolicy(Qt::StrongFocus);
}
