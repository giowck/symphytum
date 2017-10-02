/*
 *  Copyright (c) 2012 Giorgio Wicklein <giowckln@gmail.com>
 */

//-----------------------------------------------------------------------------
// Hearders
//-----------------------------------------------------------------------------

#include "collectionfieldcleaner.h"
#include "../components/metadataengine.h"
#include "../components/databasemanager.h"
#include "../components/alarmmanager.h"

#include <QtSql/QSqlQuery>
#include <QtCore/QVariant>
#include <QtCore/QStringList>


//-----------------------------------------------------------------------------
// Public
//-----------------------------------------------------------------------------

CollectionFieldCleaner::CollectionFieldCleaner(QObject *parent) :
    QObject(parent)
{
    m_metadataEngine = &MetadataEngine::getInstance();
}

void CollectionFieldCleaner::cleanField(int collectionId, int fieldId)
{
    switch (m_metadataEngine->getFieldType(fieldId, collectionId)) {
    case MetadataEngine::ImageType:
    case MetadataEngine::FilesType:
    {
        //delete all files
        QSqlDatabase db = DatabaseManager::getInstance().getDatabase();
        QSqlQuery query(db);

        //start transaction to speed up writes
        db.transaction();

        //get all file ids
        QStringList fileIdList;
        QString tableName = m_metadataEngine->getTableName(collectionId);
        QString sql = QString("SELECT \"%1\" FROM \"%2\"")
                             .arg(QString::number(fieldId)).arg(tableName);
        query.exec(sql);

        while (query.next()) {
            QString rawData = query.value(0).toString();
            if (!rawData.isEmpty()) {
                if (rawData.contains(",")) { //file list type has comma separated ids
                    fileIdList.append(rawData.split(',',
                                                    QString::SkipEmptyParts));
                } else {
                    fileIdList.append(rawData); //img type has only one id
                }
            }
        }

        //rm files
        sql = QString("DELETE FROM files WHERE _id IN (%1)")
                .arg(fileIdList.join(","));
        query.exec(sql);

        //commit transaction
        db.commit();
    }
        break;
    case MetadataEngine::DateType:
    {
        //delete all alarm triggers if any
        AlarmManager a(this);
        a.removeAllAlarms(collectionId, fieldId);
    }
        break;
    default:
        //no special action needed
        break;
    }
}

void CollectionFieldCleaner::cleanCollection(int collectionId)
{
    int count = m_metadataEngine->getFieldCount(collectionId);

    for (int i = 1; i < count; i++) { //1 because of _id
        cleanField(collectionId, i);
    }
}

//-----------------------------------------------------------------------------
// Private
//-----------------------------------------------------------------------------
