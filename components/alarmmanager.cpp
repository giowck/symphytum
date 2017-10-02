/*
 *  Copyright (c) 2012 Giorgio Wicklein <giowckln@gmail.com>
 */

//-----------------------------------------------------------------------------
// Hearders
//-----------------------------------------------------------------------------

#include "alarmmanager.h"
#include "databasemanager.h"
#include "metadataengine.h"

#include <QtCore/QObject>
#include <QtSql/QSqlQuery>
#include <QtCore/QVariant>
#include <QtCore/QStringList>


//-----------------------------------------------------------------------------
// Public
//-----------------------------------------------------------------------------

AlarmManager::AlarmManager(QObject *parent) :
    QObject(parent)
{
    m_metadatEngine = &MetadataEngine::getInstance();
}

void AlarmManager::addOrUpdateAlarm(int collectionId,
                                    int fieldId,
                                    int recordId,
                                    QDateTime &dateTime)
{
    QSqlDatabase db = DatabaseManager::getInstance().getDatabase();
    QSqlQuery query(db);

    db.transaction(); //begin transaction to speed up writes

    //check if alarm exists
    int existingId = -1;
    query.prepare("SELECT _id FROM alarms WHERE collection_id=:colId "
                  "AND field_id=:fieldId AND record_id=:recId");
    query.bindValue(":colId", collectionId);
    query.bindValue(":fieldId", fieldId);
    query.bindValue(":recId", recordId);
    query.exec();

    if (query.next()) {
        bool ok;
        int i = query.value(0).toInt(&ok);
        if (ok)
            existingId = i;
    }

    //update existing alarm
    if (existingId != -1) {
        //update alarm if date is in the future
        if (dateTime > QDateTime::currentDateTime()) {
            query.prepare("UPDATE alarms SET date=:dateTime WHERE _id=:alarmId");
            query.bindValue(":dateTime", dateTime);
            query.bindValue(":alarmId", existingId);
            query.exec();
        } else { //remove invalid alarm (since date is old)
            query.prepare("DELETE FROM alarms WHERE _id=:alarmId");
            query.bindValue(":alarmId", existingId);
            query.exec();
        }
    } else { //add new alarm
        if (dateTime > QDateTime::currentDateTime()) {
            query.prepare("INSERT INTO alarms (\"collection_id\",\"field_id\",\"record_id\",\"date\")"
                          " VALUES (:colId, :fieldId, :recId, :dateTime)");
            query.bindValue(":colId", collectionId);
            query.bindValue(":fieldId", fieldId);
            query.bindValue(":recId", recordId);
            query.bindValue(":dateTime", dateTime);
            query.exec();
        }
    }

    db.commit(); //end transaction
}

void AlarmManager::addAlarmsForExistingRecords(int collectionId, int fieldId)
{
    QSqlDatabase db = DatabaseManager::getInstance().getDatabase();
    QSqlQuery query(db);

    db.transaction(); //begin transaction to speed up writes

    //get all dates that are > than now
    QHash<int, QDateTime> datesToAdd;
    QString tableName = m_metadatEngine->getTableName(collectionId);
    QString sql = QString("SELECT \"_id\",\"%1\" FROM \"%2\" WHERE \"%1\" > "
                          "strftime(\"%Y-%m-%dT%H:%M:%S\", \"now\", \"localtime\")")
                         .arg(fieldId).arg(tableName);
    query.exec(sql);
    while (query.next()) {
        int record_id = query.value(0).toInt();
        QDateTime dateTime = query.value(1).toDateTime();
        datesToAdd.insert(record_id, dateTime);
    }

    //add alarms
    QHash<int, QDateTime>::const_iterator i = datesToAdd.constBegin();
    while (i != datesToAdd.constEnd()) {
        query.prepare("INSERT INTO alarms (\"collection_id\",\"field_id\",\"record_id\",\"date\")"
                      " VALUES (:colId, :fieldId, :recId, :dateTime)");
        query.bindValue(":colId", collectionId);
        query.bindValue(":fieldId", fieldId);
        query.bindValue(":recId", i.key());
        query.bindValue(":dateTime", i.value());
        query.exec();
        i++;
    }

    db.commit(); //end transaction
}

void AlarmManager::removeAllAlarms(int collectionId, int fieldId)
{
    QSqlDatabase db = DatabaseManager::getInstance().getDatabase();
    QSqlQuery query(db);

    //remove existing alarms
    query.prepare("DELETE FROM alarms WHERE collection_id=:colId "
                  "AND field_id=:fieldId");
    query.bindValue(":colId", collectionId);
    query.bindValue(":fieldId", fieldId);
    query.exec();
}

void AlarmManager::removeAlarm(int alarmId)
{
    QSqlDatabase db = DatabaseManager::getInstance().getDatabase();
    QSqlQuery query(db);

    //remove existing alarm
    query.prepare("DELETE FROM alarms WHERE _id=:alarmId");
    query.bindValue(":alarmId", alarmId);
    query.exec();
}

bool AlarmManager::checkAlarms()
{
    QSqlDatabase db = DatabaseManager::getInstance().getDatabase();
    QSqlQuery query(db);

    db.transaction(); //begin transaction to speed up writes

    QList<Alarm> alarmList;
    QStringList alarmIdsToRemove; //list of deprecated alarms (record ids not valid)

    //check if alarms with date < than now are available
    QString sql = QString("SELECT * "
                          " FROM \"alarms\" WHERE \"date\" < "
                          "strftime(\"%Y-%m-%dT%H:%M:%S\", \"now\", \"localtime\")");
    query.exec(sql);
    while (query.next()) {
        Alarm a;
        a.alarmId = query.value(0).toInt();
        a.alarmCollectionId = query.value(1).toInt();
        a.alarmFieldId = query.value(2).toInt();
        a.alarmRecordId = query.value(3).toInt();
        a.alarmDateTime = query.value(4).toDateTime();
        alarmList.append(a);
    }

    //check if alarms are valid
    int i = 0;
    foreach (Alarm a, alarmList) {
        QString tableName = m_metadatEngine->getTableName(a.alarmCollectionId);
        QString sql = QString("SELECT \"_id\" FROM \"%1\" WHERE \"_id\"=%2 AND \"%3\"=\"%4\"")
                .arg(tableName).arg(a.alarmRecordId)
                .arg(a.alarmFieldId).arg(a.alarmDateTime.toString(Qt::ISODate));
        query.exec(sql);
        if (!query.next()) {
            alarmIdsToRemove.append(QString::number(a.alarmId));
            alarmList.removeAt(i);
            i--;
        }
        i++;
    }

    //remove invalid alarms
    if (alarmIdsToRemove.size()) {
        QString sql = QString("DELETE FROM \"alarms\" WHERE \"_id\" IN (%1)")
                .arg(alarmIdsToRemove.join(","));
        query.exec(sql);
    }

    db.commit(); //end transaction

    return alarmList.size() > 0;
}

QList<AlarmManager::Alarm> AlarmManager::getAllTriggeredAlarms()
{
    QSqlDatabase db = DatabaseManager::getInstance().getDatabase();
    QSqlQuery query(db);
    QList<Alarm> alarmList;

    //check if alarms with date < than now are available
    QString sql = QString("SELECT * "
                          "FROM \"alarms\" WHERE \"date\" < "
                          "strftime(\"%Y-%m-%dT%H:%M:%S\", \"now\", \"localtime\")");
    query.exec(sql);
    while (query.next()) {
        Alarm a;
        a.alarmId = query.value(0).toInt();
        a.alarmCollectionId = query.value(1).toInt();
        a.alarmFieldId = query.value(2).toInt();
        a.alarmRecordId = query.value(3).toInt();
        a.alarmDateTime = query.value(4).toDateTime();
        alarmList.append(a);
    }

    return alarmList;
}

AlarmManager::Alarm AlarmManager::getAlarm(const int alarmId) const
{
    Alarm a;
    a.alarmId = -1;

    QSqlDatabase db = DatabaseManager::getInstance().getDatabase();
    QSqlQuery query(db);

    query.prepare("SELECT * FROM \"alarms\" WHERE \"_id\"=:alarmId");
    query.bindValue(":alarmId", alarmId);
    query.exec();

    if (query.next()) {
        a.alarmId = query.value(0).toInt();
        a.alarmCollectionId = query.value(1).toInt();
        a.alarmFieldId = query.value(2).toInt();
        a.alarmRecordId = query.value(3).toInt();
        a.alarmDateTime = query.value(4).toDateTime();
    }

    return a;
}


//-----------------------------------------------------------------------------
// Private
//-----------------------------------------------------------------------------
