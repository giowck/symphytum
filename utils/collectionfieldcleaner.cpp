/*
 *  Copyright (c) 2012 Giorgio Wicklein <giowckln@gmail.com>
 */

//-----------------------------------------------------------------------------
// Hearders
//-----------------------------------------------------------------------------

#include "collectionfieldcleaner.h"
#include "../components/metadataengine.h"
#include "../components/alarmmanager.h"

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
        //get all file ids
        QStringList fileIdList = m_metadataEngine->getAllCollectionContentFiles(collectionId, fieldId);
        //rm files
        m_metadataEngine->removeContentFile(fileIdList);
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
