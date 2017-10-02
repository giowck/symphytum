/*
 *  Copyright (c) 2012 Giorgio Wicklein <giowckln@gmail.com>
 */

//-----------------------------------------------------------------------------
// Hearders
//-----------------------------------------------------------------------------

#include "standardmodel.h"
#include "../components/databasemanager.h"
#include "../components/metadataengine.h"
#include "../utils/metadatapropertiesparser.h"
#include "../components/alarmmanager.h"

#include <QtSql/QSqlRecord>


//-----------------------------------------------------------------------------
// Public
//-----------------------------------------------------------------------------

StandardModel::StandardModel(MetadataEngine *meta, QObject *parent) :
    QSqlTableModel(parent, DatabaseManager::getInstance().getDatabase()),
    m_metadataEngine(meta)
{
    //save data to db immediately after change
    setEditStrategy(QSqlTableModel::OnFieldChange);
}

QVariant StandardModel::headerData(int section, Qt::Orientation orientation,
                                   int role) const
{
    if ((role == Qt::DisplayRole) && (orientation == Qt::Horizontal)) {
        QString header("_unknown_header_"); //placeholder for invalid header

        //query field name from metadata
        header = m_metadataEngine->getFieldName(section);

        return header;
    } else {
        return QSqlTableModel::headerData(section, orientation, role);
    }
}

void StandardModel::sort(int column, Qt::SortOrder order)
{
    QSqlTableModel::sort(column, order);

    emit modelSortedSignal(column);
}

void StandardModel::addRecord()
{
    QSqlRecord newRecord(record());

    for (int i = 1; i < newRecord.count(); i++) { //starting by 1 because of _id
        switch (m_metadataEngine->getFieldType(i)) {
        case MetadataEngine::CreationDateType:
        case MetadataEngine::DateType:
        {
            //init date
            QDateTime nowDateTime = QDateTime::currentDateTime();

            //if date field has no time part, set time to 00:00 (midnight)
            MetadataPropertiesParser parser(m_metadataEngine->getFieldProperties(
                                                MetadataEngine::DisplayProperty,
                                                i));
            QString v;
            v = parser.getValue("dateFormat");
            if ((v == "2") || (v == "4") || (v == "6")) { //formats without time part
                nowDateTime.setTime(QTime(0, 0));
            }

            //set current date & time
            newRecord.setValue(i, nowDateTime);
        }
            break;
        default:
            break;
        }
    }

    insertRecord(-1, newRecord);
}

void StandardModel::duplicateRecord(int row)
{
    QSqlRecord original(record(row));
    QSqlRecord duplicate(record());

    //this is a fieldId, date map for alarms
    //that need to be added later
    //in case of date fields with alarm property
    QHash<int, QDateTime> addToAlarmsTable;

    for (int i = 1; i < original.count(); i++) { //starting by 1 because of _id
        switch (m_metadataEngine->getFieldType(i)) {
        case MetadataEngine::ImageType:
        case MetadataEngine::FilesType:
        case MetadataEngine::ModDateType:
            //don't duplicate (not supported)
            break;
        case MetadataEngine::CreationDateType:
        {
            //init date
            QDateTime nowDateTime = QDateTime::currentDateTime();
            duplicate.setValue(i, nowDateTime);
        }
            break;
        case MetadataEngine::DateType:
        {
            //if alarm property, add to alarm table
            QModelIndex index = this->index(row, i);
            if (index.isValid()) {
                MetadataPropertiesParser parser(m_metadataEngine->getFieldProperties(
                                                    MetadataEngine::TriggerProperty,
                                                    index.column()));
                if (parser.size() > 0) {
                    if (parser.getValue("alarmOnDate") == "1") {
                        QDateTime d = index.data().toDateTime();
                        if (d > QDateTime::currentDateTime()) //add alarm
                            addToAlarmsTable.insert(i, d);
                    }
                }
            }

            duplicate.setValue(i, original.value(i));
        }
            break;
        default:
            duplicate.setValue(i, original.value(i));
        }
    }

    insertRecord(-1, duplicate);

    //alarm property handler for date type fields
    //this is here because the last inserted record id is needed
    //add alarms if any (in case of date field type with alarm property)
    if (addToAlarmsTable.size()) {
        AlarmManager a(this);
        int collectionId = m_metadataEngine->getCurrentCollectionId();
        bool ok;
        //get last record id (the id of the just inserted record)
        int recordId = index(realRowCount() - 1, 0).data().toInt(&ok);
        if (ok) {
            QHash<int, QDateTime>::iterator i = addToAlarmsTable.begin();
            while (i != addToAlarmsTable.end()) {
                a.addOrUpdateAlarm(collectionId, i.key(), recordId, i.value());
                ++i;
            }
        }
    }
}

bool StandardModel::removeRows(int row, int count, const QModelIndex &parent)
{
    bool r = QSqlTableModel::removeRows(row, count, parent);

    if (r)
        emit rowsDeleted(row, count);

    return r;
}

int StandardModel::realRowCount()
{
    while(canFetchMore())
        fetchMore();

    return rowCount();
}
