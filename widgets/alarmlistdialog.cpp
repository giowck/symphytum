/*
 *  Copyright (c) 2012 Giorgio Wicklein <giowckln@gmail.com>
 */

//-----------------------------------------------------------------------------
// Hearders
//-----------------------------------------------------------------------------

#include "alarmlistdialog.h"
#include "ui_alarmlistdialog.h"

#include "../components/alarmmanager.h"
#include "../components/metadataengine.h"
#include "../components/sync_framework/syncsession.h"

#include <QtWidgets/QTableWidgetItem>
#include <QtWidgets/QPushButton>


//-----------------------------------------------------------------------------
// Public
//-----------------------------------------------------------------------------

AlarmListDialog::AlarmListDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AlarmListDialog)
{
    ui->setupUi(this);

    //stay on top
    setWindowFlags(Qt::Tool);

    //components
    m_alarmManager = new AlarmManager(this);
    m_metadataEngine = &MetadataEngine::getInstance();

    //setup table view
    ui->alarmTableWidget->verticalHeader()->hide();
    ui->alarmTableWidget->horizontalHeader()->hide();
    ui->alarmTableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->alarmTableWidget->setShowGrid(false);
    ui->alarmTableWidget->setAlternatingRowColors(true);
    ui->alarmTableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->alarmTableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->alarmTableWidget->setSelectionMode(QAbstractItemView::NoSelection);

    //connections
    connect(ui->closeButton, SIGNAL(clicked()),
            this, SLOT(close()));

    populateAlarmList();
}

AlarmListDialog::~AlarmListDialog()
{
    delete ui;
}

void AlarmListDialog::reloadAlarmList()
{
    populateAlarmList();
}

//-----------------------------------------------------------------------------
// Private slots
//-----------------------------------------------------------------------------

void AlarmListDialog::showButtonClicked()
{
    QPushButton *b = NULL;
    QObject *s = sender();
    if (s)
        b = qobject_cast<QPushButton*>(s);
    if (!b) return;

    //get row id
    int row = -1;
    int rows = ui->alarmTableWidget->rowCount();
    for (int i = 0; i < rows; i++) {
        if (ui->alarmTableWidget->cellWidget(i, 4) == b) {
            row = i;
            break;
        }
    }

    //show record
    if (row != -1) {
        int id;
        bool ok;
        id = ui->alarmTableWidget->item(row, 0)->text().toInt(&ok);
        if (ok) {
            AlarmManager::Alarm a = m_alarmManager->getAlarm(id);
            emit showRecord(a.alarmCollectionId,
                            a.alarmFieldId,
                            a.alarmRecordId);
        }
    }
}

void AlarmListDialog::removeButtonClicked()
{
    QPushButton *b = NULL;
    QObject *s = sender();
    if (s)
        b = qobject_cast<QPushButton*>(s);
    if (!b) return;

    //get row id
    int row = -1;
    int rows = ui->alarmTableWidget->rowCount();
    for (int i = 0; i < rows; i++) {
        if (ui->alarmTableWidget->cellWidget(i, 5) == b) {
            row = i;
            break;
        }
    }

    //remove alarm
    if (row != -1) {
        int id;
        bool ok;
        id = ui->alarmTableWidget->item(row, 0)->text().toInt(&ok);
        if (ok) {
            m_alarmManager->removeAlarm(id);
            ui->alarmTableWidget->removeRow(row);
            updateEmptyState();

            //set local data changed
            SyncSession::LOCAL_DATA_CHANGED = true;
        }
    }
}


//-----------------------------------------------------------------------------
// Private
//-----------------------------------------------------------------------------

void AlarmListDialog::populateAlarmList()
{
    ui->alarmTableWidget->clear();

    QList<AlarmManager::Alarm> alarmList = m_alarmManager->getAllTriggeredAlarms();
    int size = alarmList.size();

    //set col/row count
    ui->alarmTableWidget->setColumnCount(6); //_id,collection,field,date,show,remove
    ui->alarmTableWidget->horizontalHeader()->hideSection(0); //hide _id
    ui->alarmTableWidget->setRowCount(size);

    //create items
    for (int i = 0; i < size; i++) {
        AlarmManager::Alarm a = alarmList.at(i);
        QTableWidgetItem *id = new QTableWidgetItem(
                    QString::number(a.alarmId));
        QTableWidgetItem *collection = new QTableWidgetItem(
                    m_metadataEngine->getCollectionName(a.alarmCollectionId));
        QTableWidgetItem *field = new QTableWidgetItem(
                    m_metadataEngine->getFieldName(a.alarmFieldId,
                                                   a.alarmCollectionId));
        QTableWidgetItem *date = new QTableWidgetItem(
                    a.alarmDateTime.toString(QLocale().dateTimeFormat(QLocale::ShortFormat)));
        ui->alarmTableWidget->setItem(i, 0, id);
        ui->alarmTableWidget->setItem(i, 1, collection);
        ui->alarmTableWidget->setItem(i, 2, field);
        ui->alarmTableWidget->setItem(i, 3, date);
        QPushButton *show = new QPushButton(tr("Show"), this);
        QPushButton *remove = new QPushButton(tr("Remove"), this);
        show->setToolTip(tr("Show affected record"));
        remove->setToolTip(tr("Remove reminder from list"));
        connect(show, SIGNAL(clicked()),
                this, SLOT(showButtonClicked()));
        connect(remove, SIGNAL(clicked()),
                this, SLOT(removeButtonClicked()));
        ui->alarmTableWidget->setCellWidget(i, 4, show);
        ui->alarmTableWidget->setCellWidget(i, 5, remove);
    }

    updateEmptyState();
}

void AlarmListDialog::updateEmptyState()
{
    bool empty = ui->alarmTableWidget->rowCount() == 0;

    ui->alarmTableWidget->setVisible(!empty);
    ui->noAlarmsLabel->setVisible(empty);
}
