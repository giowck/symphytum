/*
 *  Copyright (c) 2012 Giorgio Wicklein <giowckln@gmail.com>
 */

//-----------------------------------------------------------------------------
// Hearders
//-----------------------------------------------------------------------------

#include "datefieldwizard.h"
#include "ui_datefieldwizard.h"

#include "../../components/metadataengine.h"
#include "../../components/alarmmanager.h"
#include "../../utils/metadatapropertiesparser.h"

#include <QtCore/QDateTime>
#include <QtCore/QLocale>


//-----------------------------------------------------------------------------
// Public
//-----------------------------------------------------------------------------

DateFieldWizard::DateFieldWizard(const QString &fieldName,
                                 QWidget *parent,
                                 AbstractFieldWizard::EditMode editMode) :
    AbstractFieldWizard(fieldName, parent, editMode),
    ui(new Ui::DateFieldWizard),
    m_alarmOnDate(false)
{
    ui->setupUi(this);

    //setup format combo
    QDateTime nowDateTime = QDateTime::currentDateTime();
    QLocale locale;
    ui->dateFormatBox->addItem(nowDateTime.toString(
                                   locale.dateTimeFormat(QLocale::ShortFormat)), 1);
    ui->dateFormatBox->addItem(nowDateTime.toString(
                                   locale.dateFormat(QLocale::ShortFormat)), 2);
    ui->dateFormatBox->addItem(nowDateTime.toString("ddd MMM d hh:mm yyyy"), 3);
    ui->dateFormatBox->addItem(nowDateTime.toString("ddd MMM d yyyy"), 4);
    ui->dateFormatBox->addItem(nowDateTime.toString("yyyy-MM-dd hh:mm"), 5);
    ui->dateFormatBox->addItem(nowDateTime.toString("yyyy-MM-dd"), 6);

    connect(ui->backButton, SIGNAL(clicked()),
            this, SIGNAL(backSignal()));
    connect(ui->finishButton, SIGNAL(clicked()),
            this, SLOT(finishButtonClicked()));

    ui->finishButton->setFocus();
}

DateFieldWizard::~DateFieldWizard()
{
    delete ui;
}

void DateFieldWizard::getFieldProperties(QString &displayProperties,
                                           QString &editProperties,
                                           QString &triggerProperties)
{
    //create display properties metadata string
    int index = ui->dateFormatBox->currentIndex();
    int formatCode = ui->dateFormatBox->itemData(index).toInt();
    displayProperties.append(QString("dateFormat:%1;").arg(formatCode));

    //create edit properties metadata string
    //nothing for now
    Q_UNUSED(editProperties);

    //create trigger properties metadata string
    int alarmOnDate = 0;
    if (ui->reminderCheckBox->isChecked())
        alarmOnDate = 1;
    triggerProperties.append(QString("alarmOnDate:%1")
                             .arg(QString::number(alarmOnDate)));
}

void DateFieldWizard::loadField(const int fieldId, const int collectionId)
{
    AbstractFieldWizard::loadField(fieldId, collectionId);

    MetadataEngine *meta = &MetadataEngine::getInstance();

    //display properties
    QString displayProperties = meta->getFieldProperties(meta->DisplayProperty,
                                                      fieldId, collectionId);
    MetadataPropertiesParser displayParser(displayProperties);
    if (displayParser.size()) {
        //date format
        int f = displayParser.getValue("dateFormat").toInt();
        if (f)
            ui->dateFormatBox->setCurrentIndex(f-1);
    }

    //trigger properties
    QString triggerProperties = meta->getFieldProperties(meta->TriggerProperty,
                                                      fieldId, collectionId);
    MetadataPropertiesParser triggerParser(triggerProperties);
    if (triggerParser.size()) {
        m_alarmOnDate = triggerParser.getValue("alarmOnDate").toInt();
        ui->reminderCheckBox->setChecked(m_alarmOnDate);
    }
}


//-----------------------------------------------------------------------------
// Private slots
//-----------------------------------------------------------------------------

void DateFieldWizard::finishButtonClicked()
{
    //if editing, update alarms
    if (this->m_currentEditMode == AbstractFieldWizard::ModifyEditMode) {
        bool oldAlarmOnDate = m_alarmOnDate;
        bool currentAlarmOnDate = ui->reminderCheckBox->isChecked();

        if (oldAlarmOnDate != currentAlarmOnDate) {
            AlarmManager a(this);
            if (currentAlarmOnDate) {
                //set up alarms for existing dates
                a.addAlarmsForExistingRecords(m_collectionId, m_fieldId);
            } else {
                //remove all existing alarms
                a.removeAllAlarms(m_collectionId, m_fieldId);
            }
        }
    }

    emit finishSignal();
}
