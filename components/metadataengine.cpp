/*
 *  Copyright (c) 2012 Giorgio Wicklein <giowckln@gmail.com>
 */

//-----------------------------------------------------------------------------
// Hearders
//-----------------------------------------------------------------------------

#include "metadataengine.h"
#include "databasemanager.h"
#include "alarmmanager.h"
#include "filemanager.h"
#include "../models/standardmodel.h"

#include <QtSql/QSqlQuery>
#include <QtCore/QVariant>
#include <QtCore/QAbstractItemModel>
#include <QtCore/QStringList>
#include <QtCore/QDateTime>
#include <QtCore/QCryptographicHash>
#include <QtWidgets/QProgressDialog>
#include <QtWidgets/QApplication>


//-----------------------------------------------------------------------------
// Static init
//-----------------------------------------------------------------------------

MetadataEngine* MetadataEngine::m_instance = nullptr;
int MetadataEngine::m_currentCollectionId = 0;
QStringList* MetadataEngine::m_currentCollectionFieldNameList = nullptr;


//-----------------------------------------------------------------------------
// Public
//-----------------------------------------------------------------------------

MetadataEngine& MetadataEngine::getInstance()
{
    if (!m_instance)
        m_instance = new MetadataEngine();
    return *m_instance;
}

void MetadataEngine::destroy()
{
    if (m_instance)
        delete m_instance;
    m_instance = 0;
}

QString MetadataEngine::getCollectionName(const int collectionId) const
{
    QString name("_collection_invalid_"); //invalid placeholder

    QSqlDatabase db = DatabaseManager::getInstance().getDatabase();
    QSqlQuery query(db);

    query.prepare("SELECT name FROM collections WHERE _id=:collectionId");
    query.bindValue(":collectionId", collectionId);
    query.exec();

    if (query.next()) {
        name = query.value(0).toString();
    }

    return name;
}

int MetadataEngine::getCurrentCollectionId()
{
    int id = 0; //0 is an invalid collection id

    //if the current id was already queried return it
    if (m_currentCollectionId) {
        id = m_currentCollectionId;
    } else { //if first time called, query the db
        QSqlDatabase db = DatabaseManager::getInstance().getDatabase();
        QSqlQuery query(db);

        query.exec("SELECT value FROM symphytum_info WHERE key='current_collection'");
        if (query.next()) {
            QString id_string = query.value(0).toString();
            id = id_string.toInt();
        }

        //update collection id cache
        m_currentCollectionId = id;

        //update field name cache
        updateFieldNameCache();
    }

    return id;
}

void MetadataEngine::setCurrentCollectionId(const int id)
{
    //update cached id
    m_currentCollectionId = id;

    //store in db
    QSqlDatabase db = DatabaseManager::getInstance().getDatabase();
    QSqlQuery query(db);

    query.prepare("UPDATE symphytum_info SET value=:id WHERE key='current_collection'");
    query.bindValue(":id", id);
    query.exec();

    //update field name cache
    updateFieldNameCache();

    //inform components that current collection changed
    emit currentCollectionIdChanged(id);
}

QString MetadataEngine::getCurrentCollectionName() const
{
    QString name("_collection_invalid_"); //invalid placeholder

    QSqlDatabase db = DatabaseManager::getInstance().getDatabase();
    QSqlQuery query(db);

    query.exec("SELECT name FROM collections WHERE _id = (SELECT value FROM "
               "symphytum_info WHERE key='current_collection')");

    if (query.next()) {
        name = query.value(0).toString();
    }

    return name;
}

QStringList MetadataEngine::getAllCollections()
{
    QStringList idList;

    QSqlQuery query(DatabaseManager::getInstance().getDatabase());
    query.exec("SELECT _id FROM collections");

    while (query.next()) {
        idList.append(query.value(0).toString());
    }

    return idList;
}

QString MetadataEngine::getTableName(const int collectionId) const
{
    QString tableName("_invalid_table_name_"); //placeholder for invalid table name

    QSqlQuery query(DatabaseManager::getInstance().getDatabase());
    query.prepare("SELECT table_name FROM collections WHERE _id=:id");
    query.bindValue(":id", collectionId);
    query.exec();

    if (query.next())
        tableName = query.value(0).toString();

    return tableName;
}

QString MetadataEngine::getFieldName(const int column, int collectionId) const
{
    QString name("_invalid_column_name_"); //placeholder for invalid column name

    if (collectionId == m_currentCollectionId) {
        //use cache
        if (column < m_currentCollectionFieldNameList->size())
            name = m_currentCollectionFieldNameList->at(column);
    } else {
        //query directly from db
        name = getFieldNameFromDatabase(column, collectionId);
    }

    return name;
}

void MetadataEngine::setFieldName(const int column, const QString &name,
                                  int collectionId)
{
    QString metadataTable = getTableName(collectionId).append("_metadata");
    QSqlQuery query(DatabaseManager::getInstance().getDatabase());
    QString columnKey = QString("col%1_name").arg(column);

    query.prepare(QString("UPDATE '%1' SET value=:name WHERE key=:column_id")
                         .arg(metadataTable)); //arg because bindValue() fails
    query.bindValue(":name", name);
    query.bindValue(":column_id", columnKey);
    query.exec();

    //update name cache
    updateFieldNameCache();
}

int MetadataEngine::getFieldCount(int collectionId) const
{
    if (collectionId == m_currentCollectionId)
        return m_currentCollectionFieldNameList->size();
    else
        return getFieldCountFromDatabase(collectionId);
}

MetadataEngine::FieldType MetadataEngine::getFieldType(int column,
                                                       int collectionId) const
{
    FieldType type = TextType; //default

    QSqlQuery query(DatabaseManager::getInstance().getDatabase());
    QString columnKey = QString("col%1_type").arg(column);
    QString metadataTable = getTableName(collectionId).append("_metadata");

    query.prepare(QString("SELECT value FROM '%1' WHERE key=:key").arg(metadataTable));
    query.bindValue(":key", columnKey);
    query.exec();

    if (query.next())
        type = (FieldType) query.value(0).toInt();

    return type;
}

bool MetadataEngine::getFieldCoordinate(const int column, int &xpos,
                                        int &ypos, int collectionId) const
{
    bool valid = false;
    xpos = ypos = 0; //avoid random garbage values

    QSqlQuery query(DatabaseManager::getInstance().getDatabase());
    QString columnKey = QString("col%1_pos").arg(column);
    QString metadataTable = getTableName(collectionId).append("_metadata");

    query.prepare(QString("SELECT value FROM '%1' WHERE key=:key").arg(metadataTable));
    query.bindValue(":key", columnKey);
    query.exec();

    if (query.next()) {
        //metadata for pos is saved as "x;y" where x is the column and y the row
        QString s = query.value(0).toString();
        QStringList l = s.split(";", QString::SkipEmptyParts);
        if (l.size() == 2) {
            valid = true;
            xpos = l.at(0).toInt();
            ypos = l.at(1).toInt();
            //since the pos "-1;-1" means invalid/unset coordinates
            if ((xpos == -1) || (ypos == -1))
                valid = false;
        }
    }

    return valid;
}

void MetadataEngine::setFieldCoordinate(const int column, const int xpos,
                                        const int ypos, int collectionId)
{
    QString metadataTable = getTableName(collectionId).append("_metadata");
    QSqlQuery query(DatabaseManager::getInstance().getDatabase());
    QString columnKey = QString("col%1_pos").arg(column);

    //buid coordinate string
    QString posString = QString::number(xpos) + ";" + QString::number(ypos);

    query.prepare(QString("UPDATE '%1' SET value=:pos WHERE key=:column_id")
                         .arg(metadataTable)); //arg because bindValue() fails
    query.bindValue(":pos", posString);
    query.bindValue(":column_id", columnKey);
    query.exec();
}

void MetadataEngine::getFieldFormLayoutSize(const int column, int &widthUnits,
                                            int &heightUnits, int collectionId) const
{
    heightUnits = widthUnits = -1; //-1 means not set (use default)

    QSqlQuery query(DatabaseManager::getInstance().getDatabase());
    QString columnKey = QString("col%1_size").arg(column);
    QString metadataTable = getTableName(collectionId).append("_metadata");

    query.prepare(QString("SELECT value FROM '%1' WHERE key=:key").arg(metadataTable));
    query.bindValue(":key", columnKey);
    query.exec();

    if (query.next()) {
        //metadata for size is saved as "a;b"
        //where a is the width and b the height
        QString s = query.value(0).toString();
        QStringList l = s.split(";", QString::SkipEmptyParts);
        if (l.size() == 2) {
            widthUnits = l.at(0).toInt();
            heightUnits = l.at(1).toInt();
        }
    }
}

void MetadataEngine::setFieldFormLayoutSize(const int column, const int widthUnits,
                                            const int heightUnits, int collectionId) const
{
    QString metadataTable = getTableName(collectionId).append("_metadata");
    QSqlQuery query(DatabaseManager::getInstance().getDatabase());
    QString columnKey = QString("col%1_size").arg(column);

    //buid size string
    QString sizeString = QString::number(widthUnits) + ";"
                         + QString::number(heightUnits);

    query.prepare(QString("UPDATE '%1' SET value=:size WHERE key=:column_id")
                         .arg(metadataTable)); //arg because bindValue() fails
    query.bindValue(":size", sizeString);
    query.bindValue(":column_id", columnKey);
    query.exec();
}

QString MetadataEngine::getFieldProperties(FieldProperty propertyType,
                                           const int column,
                                           int collectionId) const
{
    QString s("");

    QSqlQuery query(DatabaseManager::getInstance().getDatabase());
    QString columnKey;
    QString metadataTable = getTableName(collectionId).append("_metadata");

    switch (propertyType) {
    case DisplayProperty:
        columnKey = QString("col%1_display").arg(column);
        break;
    case EditProperty:
        columnKey = QString("col%1_edit").arg(column);
        break;
    case TriggerProperty:
        columnKey = QString("col%1_trigger").arg(column);
        break;
    }

    query.prepare(QString("SELECT value FROM '%1' WHERE key=:key").arg(metadataTable));
    query.bindValue(":key", columnKey);
    query.exec();

    if (query.next()) {
        s = query.value(0).toString();
    }

    return s;
}

void MetadataEngine::setFieldProperties(FieldProperty propertyType,
                                        const int column,
                                        const QString &propertyString,
                                        int collectionId)
{
    QString metadataTable = getTableName(collectionId).append("_metadata");
    QSqlQuery query(DatabaseManager::getInstance().getDatabase());
    QString columnKey;

    switch (propertyType) {
    case DisplayProperty:
        columnKey = QString("col%1_display").arg(column);
        break;
    case EditProperty:
        columnKey = QString("col%1_edit").arg(column);
        break;
    case TriggerProperty:
        columnKey = QString("col%1_trigger").arg(column);
        break;
    }

    query.prepare(QString("UPDATE '%1' SET value=:property WHERE key=:column_id")
                         .arg(metadataTable));
    query.bindValue(":property", propertyString);
    query.bindValue(":column_id", columnKey);
    query.exec();
}

QAbstractItemModel* MetadataEngine::createModel(CollectionType type,
                                               const int collectionId)
{
    QAbstractItemModel *model = nullptr;

    switch (type) {
    case StandardCollection:
        model = createStandardModel(collectionId);
        break;
    }

    return model;
}

int MetadataEngine::createNewCollection()
{
    int id = 0;
    CollectionType type = StandardCollection; //for now the only supported type
    QSqlDatabase db = DatabaseManager::getInstance().getDatabase();
    QSqlQuery query(db);

    //since a new entry in the collection table
    //is added by the CollectionListView
    //get last created collection
    query.exec("SELECT _id FROM collections ORDER BY _id DESC");

    if (query.next()) {
        //get the first, which is the last created collection
        id = query.value(0).toInt();
    }

    //create table name hash
    QByteArray dateArray = QDateTime::currentDateTime().toString().toUtf8();
    QByteArray hash = QCryptographicHash::hash(dateArray,
                                               QCryptographicHash::Md5);
    QString tableName("c" + hash.toHex());
    QString metadataTableName = tableName + "_metadata";

    //start transaction to speed up writes
    db.transaction();

    //update collection meta info
    query.prepare("UPDATE collections SET type=:type, table_name=:table_name WHERE _id=:id");
    query.bindValue(":type", (int) type);
    query.bindValue(":table_name", tableName);
    query.bindValue(":id", id);
    query.exec();

    //create data table
    query.exec(QString("CREATE TABLE '%1' (\"_id\" INTEGER PRIMARY KEY)").arg(tableName));

    //create metadata table
    query.exec(QString("CREATE TABLE '%1' (\"_id\" INTEGER PRIMARY KEY"
                       " , \"key\" TEXT, \"value\" TEXT)").arg(metadataTableName));
    query.exec(QString("INSERT INTO '%1' (\"key\",\"value\") VALUES (\"column_count\", 1)")
               .arg(metadataTableName));

    //commit transaction
    db.commit();

    return id;
}

void MetadataEngine::deleteCollection(int collectionId)
{
    QString tableName;
    QString metadataTableName;
    QSqlDatabase db = DatabaseManager::getInstance().getDatabase();
    QSqlQuery query(db);

    //get table name
    query.prepare("SELECT table_name FROM collections WHERE _id=:id");
    query.bindValue(":id", collectionId);
    query.exec();

    if (query.next()) {
        //get the first, which is the last created collection
        tableName = query.value(0).toString();
        metadataTableName = tableName + "_metadata";
    }

    //start transaction to speed up writes
    db.transaction();

    //delete content data table
    query.exec(QString("DROP TABLE '%1'").arg(tableName));

    //delete metadata table
    query.exec(QString("DROP TABLE '%1'").arg(metadataTableName));

    //commit transaction
    db.commit();
}

int MetadataEngine::duplicateCollection(int collectionId, bool copyMetadataOnly)
{
    int id = 0;

    CollectionType type = StandardCollection; //for now the only supported type
    QSqlDatabase db = DatabaseManager::getInstance().getDatabase();
    QSqlQuery query(db);

    //since a new entry in the collection table
    //is added by the CollectionListView
    //get last created collection
    query.exec("SELECT _id FROM collections ORDER BY _id DESC");

    if (query.next()) {
        //get the first, which is the last created collection
        id = query.value(0).toInt();
    }

    //create table name hash
    QByteArray dateArray = QDateTime::currentDateTime().toString().toUtf8();
    QByteArray hash = QCryptographicHash::hash(dateArray,
                                               QCryptographicHash::Md5);
    QString tableName("c" + hash.toHex());
    QString metadataTableName = tableName + "_metadata";

    //start transaction to speed up writes
    db.transaction();

    //update collection meta info
    query.prepare("UPDATE collections SET type=:type, table_name=:table_name WHERE _id=:id");
    query.bindValue(":type", (int) type);
    query.bindValue(":table_name", tableName);
    query.bindValue(":id", id);
    query.exec();

    //copy collection structure
    QString originalTableName = getTableName(collectionId);
    QString originalTableMetadataName = originalTableName + "_metadata";

    //don't use the following because SQLite can't alter tables afterwards to add primary key constraint
    //query.exec(QString("CREATE TABLE '%1' AS SELECT * FROM '%2' WHERE 0")
    //           .arg(tableName).arg(originalTableName));
    //so we use info from sqlite_master to copy the table structure, including constraints
    query.exec(QString("SELECT sql FROM sqlite_master WHERE tbl_name='%1'")
               .arg(originalTableName));
    if (query.next()) {
        //get the original sql create statement
        QString sql = query.value(0).toString();
        sql.replace(originalTableName, tableName);
        //create duplicate
        query.exec(sql);
    }

    //copy metadata table
    //don't use the following because SQLite can't alter tables afterwards to add primary key constraint
    //query.exec(QString("CREATE TABLE '%1' AS SELECT * FROM '%2' WHERE 0")
    //           .arg(metadataTableName).arg(originalTableMetadataName));
    query.exec(QString("SELECT sql FROM sqlite_master WHERE tbl_name='%1'")
               .arg(originalTableMetadataName));
    if (query.next()) {
        //get the original sql create statement
        QString sql = query.value(0).toString();
        sql.replace(originalTableMetadataName, metadataTableName);
        //create duplicate
        query.exec(sql);
    }
    query.exec(QString("INSERT INTO '%1' SELECT * FROM '%2'")
               .arg(metadataTableName).arg(originalTableMetadataName));

    //copy content data
    if (!copyMetadataOnly) {
        QProgressDialog *pd = new QProgressDialog(nullptr);
        int progressSteps = 3;
        pd->setWindowModality(Qt::ApplicationModal);
        pd->setWindowTitle(tr("Progress"));
        pd->setLabelText(tr("Duplicating collection data... Please wait!"));
        pd->setRange(0, progressSteps);
        pd->setValue(progressSteps++);
        pd->setCancelButton(nullptr);
        pd->show();
        qApp->processEvents();

        //copy collection data
        query.exec(QString("INSERT INTO '%1' SELECT * FROM '%2'")
                   .arg(tableName).arg(originalTableName));
        pd->setValue(progressSteps++);
        qApp->processEvents();

        //copy alarms
        AlarmManager am(this);
        QList<AlarmManager::Alarm> alarmList = am.getAllAlarms(collectionId);
        foreach (AlarmManager::Alarm a, alarmList) {
            am.addOrUpdateAlarm(id, a.alarmFieldId, a.alarmRecordId, a.alarmDateTime);
        }
        pd->setValue(progressSteps++);
        qApp->processEvents();

        //copy files
        FileManager fm(this);
        QString filesDirPath = fm.getFilesDirectory();
        QStringList fileIdsToCopyList = getAllCollectionContentFiles(collectionId);
        QHash<int, int> originalFileIdToDuplicateIdBridge;
        pd->setRange(0, fileIdsToCopyList.size());
        progressSteps = 0;
        pd->setValue(progressSteps);
        pd->setLabelText(tr("Duplicating collection files... Please wait!"));
        qApp->processEvents();
        foreach (QString f, fileIdsToCopyList) {
            pd->setValue(++progressSteps);
            qApp->processEvents();
            QString filePath, fileHashName, fileName, origDirPath;
            QDateTime date;
            int id = f.toInt();
            bool s = getContentFile(id, fileName, fileHashName, date, origDirPath);
            if (s) {
                filePath = filesDirPath + fileHashName;
                //add file and wait until copy task complete
                {
                    QEventLoop loop;
                    loop.connect(&fm, SIGNAL(addFileCompletedSignal(QString)), SLOT(quit()));
                    loop.connect(&fm, SIGNAL(fileOpFailed()), SLOT(quit()));
                    fm.startAddFile(filePath);
                    loop.exec();
                }

                //update file metadata

                //get new file duplicate id from original hash which is temporary used as file name
                int duplicateFileId = 0;
                query.prepare("SELECT _id FROM files WHERE name=:hashName");
                query.bindValue(":hashName", fileHashName);
                query.exec();
                if (query.next()) {
                    duplicateFileId = query.value(0).toInt();
                    //associate old id to new id
                    originalFileIdToDuplicateIdBridge.insert(id, duplicateFileId);
                }

                //update duplicate file metadata (file name and date) to match original
                query.prepare("UPDATE files SET name=:name, date_added=:date_added, "
                              "original_dir_path=:original_dir_path WHERE name=:hashName");
                query.bindValue(":name", fileName);
                query.bindValue(":date_added", date);
                query.bindValue(":hashName", fileHashName);
                query.bindValue(":original_dir_path", origDirPath);
                query.exec();
            }
        }

        //update duplicate collection data to use new duplicate file ids instead of original file ids
        //for each field type that has content files
        pd->setRange(0, 0);
        progressSteps = 0;
        pd->setValue(progressSteps);
        pd->setLabelText(tr("Updating files metadata... Please wait!"));
        qApp->processEvents();
        int count = getFieldCount(collectionId);
        for (int i = 1; i < count; i++) { //1 because of _id
            MetadataEngine::FieldType fieldType = getFieldType(i, collectionId);
            if (fieldType == MetadataEngine::ImageType) {
                auto list = getAllCollectionContentFiles(collectionId, i);
                foreach (QString is, list) {
                    //just update file id since only a single value in database
                    int oid = is.toInt();
                    int did = originalFileIdToDuplicateIdBridge.value(oid);
                    QString sql = QString("UPDATE \"%1\" SET \"%2\"=:dupId WHERE \"%2\"=:originalId")
                            .arg(tableName).arg(i);
                    query.prepare(sql);
                    query.bindValue(":dupId", did);
                    query.bindValue(":originalId", oid);
                    query.exec();
                }
            } else if (fieldType == MetadataEngine::FilesType) {
                //get raw file list data (simple file ids separated by comma)
                QStringList fileIdsRawList;
                query.exec(QString("SELECT \"%1\" FROM \"%2\"").arg(i).arg(tableName));
                while (query.next()) {
                    fileIdsRawList.append(query.value(0).toString());
                }
                //replace each value in list with duplicate id and
                //build new raw file id string to push into the duplicate collection
                foreach (QString originalRawString, fileIdsRawList) {
                    QString duplicateRawString;
                    foreach (QString t, originalRawString.split(',', QString::SkipEmptyParts)) {
                        int d_id = originalFileIdToDuplicateIdBridge.value(t.toInt()) ;
                        if (d_id)
                            duplicateRawString.append(QString::number(d_id) + ",");
                    }
                    if (!duplicateRawString.isEmpty()) {
                        QString sql = QString("UPDATE \"%1\" SET \"%2\"=:dupId WHERE \"%2\"=:originalId")
                                .arg(tableName).arg(i);
                        query.prepare(sql);
                        query.bindValue(":dupId", duplicateRawString);
                        query.bindValue(":originalId", originalRawString);
                        query.exec();
                    }
                }
            } else {
                //not a content file field type
            }
            qApp->processEvents();
        }

        //delete since no parent
        pd->deleteLater();
    }

    //commit transaction
    db.commit();

    return id;
}

void MetadataEngine::deleteAllRecords(int collectionId)
{
    QString table = getTableName(collectionId);

    DatabaseManager::getInstance().truncateTable(table);
}

int MetadataEngine::createField(const QString &fieldName, FieldType type,
                                 const QString &displayProperties,
                                 const QString &editProperties,
                                 const QString &triggerProperties,
                                 int collectionId)
{
    QSqlDatabase db = DatabaseManager::getInstance().getDatabase();
    QSqlQuery query(db);

    int fieldId = getFieldCount(collectionId);
    QString tableName = getTableName(collectionId);
    QString metadataTable = tableName + "_metadata";
    QString dataTypeName = dataTypeSqlName(type);

    //start transaction to speed up writes
    db.transaction();

    //add column to data table
    query.exec(QString("ALTER TABLE '%1' ADD '%2' %3").arg(tableName)
               .arg(fieldId).arg(dataTypeName));

    //update metadata field count
    int columnCount = fieldId + 1;
    query.prepare(QString("UPDATE '%1' SET value=:count WHERE key='column_count'")
                         .arg(metadataTable)); //arg because bindValue() fails
    query.bindValue(":count", columnCount);
    query.exec();

    //metadata column pos
    query.exec(QString("INSERT INTO '%1' (\"key\",\"value\") VALUES (\"col%2_pos\","
                       "\"-1;-1\")").arg(metadataTable).arg(fieldId));

    //metadata column size
    query.exec(QString("INSERT INTO '%1' (\"key\",\"value\") VALUES (\"col%2_size\","
                       "\"-1;-1\")").arg(metadataTable).arg(fieldId));

    //metadata display properties
    query.exec(QString("INSERT INTO '%1' (\"key\",\"value\") VALUES (\"col%2_display\","
                       "\"%3\")").arg(metadataTable).arg(fieldId)
               .arg(displayProperties));

    //metadata edit properties
    query.exec(QString("INSERT INTO '%1' (\"key\",\"value\") VALUES (\"col%2_edit\","
                       "\"%3\")").arg(metadataTable).arg(fieldId)
               .arg(editProperties));

    //metadata trigger properties
    query.exec(QString("INSERT INTO '%1' (\"key\",\"value\") VALUES (\"col%2_trigger\","
                       "\"%3\")").arg(metadataTable).arg(fieldId)
               .arg(triggerProperties));

    //metadata column name
    QString fieldNameEscaped = QString(fieldName).replace("\"", "\"\""); //escape double quotes for SQL
    query.exec(QString("INSERT INTO '%1' (\"key\",\"value\") VALUES (\"col%2_name\","
                       "\"%3\")").arg(metadataTable).arg(fieldId)
               .arg(fieldNameEscaped));

    //metadata column type
    query.exec(QString("INSERT INTO '%1' (\"key\",\"value\") VALUES (\"col%2_type\","
                       "\"%3\")").arg(metadataTable).arg(fieldId)
               .arg((int) type));

    //commit transaction
    db.commit();

    //update cached metadata
    updateFieldNameCache();

    //notify the change
    if (collectionId == m_currentCollectionId)
        emit currentCollectionChanged();

    return fieldId;
}

void MetadataEngine::modifyField(const int &fieldId, const QString &fieldName,
                                 const QString &displayProperties,
                                 const QString &editProperties,
                                 const QString &triggerProperties,
                                 int collectionId)
{
    QSqlDatabase db = DatabaseManager::getInstance().getDatabase();
    QSqlQuery query(db);

    QString tableName = getTableName(collectionId);
    QString metadataTable = tableName + "_metadata";

    //start transaction to speed up writes
    db.transaction();

    //update field name
    query.prepare(QString("UPDATE '%1' SET value=:fieldName WHERE key=:columnKey")
                         .arg(metadataTable));
    query.bindValue(":fieldName", fieldName);
    query.bindValue(":columnKey", QString("col%1_name").arg(fieldId));
    query.exec();

    //update display properties
    query.prepare(QString("UPDATE '%1' SET value=:properties WHERE key=:columnKey")
                         .arg(metadataTable));
    query.bindValue(":properties", displayProperties);
    query.bindValue(":columnKey", QString("col%1_display").arg(fieldId));
    query.exec();

    //update edit properties
    query.prepare(QString("UPDATE '%1' SET value=:properties WHERE key=:columnKey")
                         .arg(metadataTable));
    query.bindValue(":properties", editProperties);
    query.bindValue(":columnKey", QString("col%1_edit").arg(fieldId));
    query.exec();

    //update trigger properties
    query.prepare(QString("UPDATE '%1' SET value=:properties WHERE key=:columnKey")
                         .arg(metadataTable));
    query.bindValue(":properties", triggerProperties);
    query.bindValue(":columnKey", QString("col%1_trigger").arg(fieldId));
    query.exec();

    //commit transaction
    db.commit();

    //update cached metadata
    updateFieldNameCache();

    //notify the change
    if (collectionId == m_currentCollectionId)
        emit currentCollectionChanged();
}

void MetadataEngine::deleteField(const int fieldId, int collectionId)
{
    QSqlDatabase db = DatabaseManager::getInstance().getDatabase();
    QSqlQuery query(db);

    QString tableName = getTableName(collectionId);
    QString metadataTable = tableName + "_metadata";

    //start transaction to speed up writes
    db.transaction();

    //SQLite has no support form column dropping
    //so build custom procedure
    int fieldCount = getFieldCount(collectionId);

    //generate column list to keep
    QString columnsToKeep;
    columnsToKeep.append("\"_id\"");
    for (int i = 1; i < fieldCount; i++) {
        columnsToKeep.append(QString(",\"%1\"").arg(i));
    }
    //remove the field id marked for deletion from list
    columnsToKeep.remove(QString(",\"%1\"").arg(fieldId));

    //create field type list
    QStringList fieldTypes;
    fieldTypes.append("INTEGER PRIMARY KEY"); //0 is _id
    for (int i = 1; i < fieldCount; i++) {
        FieldType type = getFieldType(i, collectionId);
        fieldTypes.append(dataTypeSqlName(type));
    }
    //remove fieldType for column marked for deletion
    fieldTypes.removeAt(fieldId);

    //generate column definition for new table without specified column
    QString columnDefinition;
    columnDefinition.append("\"_id\" INTEGER PRIMARY KEY"); //_id column 0
    for (int i = 1; i < (fieldCount - 1);i++) {
        columnDefinition.append(QString(",\"%1\" %2").arg(i).arg(fieldTypes.at(i)));
    }

    QString dropSQL = QString(
                "CREATE TEMPORARY TABLE t_backup(%1);"
                "INSERT INTO t_backup SELECT %2 FROM '%3';"
                "DROP TABLE '%3';"
                "CREATE TABLE '%3'(%1);"
                "INSERT INTO '%3' SELECT * FROM t_backup;"
                "DROP TABLE t_backup;"
                ).arg(columnDefinition).arg(columnsToKeep).arg(tableName);
    //execute drop column procedure
    //splitting commands because query.exec()
    //can execute only one command at time
    QStringList commands = dropSQL.split(';', QString::SkipEmptyParts);
    foreach (QString q, commands) {
        query.exec(q);
    }

    //update metadata column count
    query.prepare(QString("UPDATE '%1' SET value=:count WHERE key='column_count'")
                         .arg(metadataTable)); //arg because bindValue() fails
    query.bindValue(":count", fieldCount - 1);
    query.exec();

    //mark column keys for deletion
    query.exec(QString("UPDATE '%1' SET key='del_me' WHERE key LIKE 'col%2@_%' ESCAPE '@'")
               .arg(metadataTable).arg(fieldId));

    //make a list of all column keys after the one to delete
    //that need to be decremented
    QStringList columnsToDecrement;
    QStringList decrementedColumns;
    query.exec(QString("SELECT key FROM '%1' WHERE key LIKE 'col%@_%' ESCAPE '@' "
                       "AND key != 'column_count'")
               .arg(metadataTable));
    while (query.next()) {
        QString key = query.value(0).toString();
        QString s = key;
        int column = s.remove(QRegExp("\\D")).toInt(); //extract column id
        if (column > fieldId) {
            columnsToDecrement.append(key);
            QString newKey = key;
            newKey.replace(QString::number(column), QString::number(column-1));
            decrementedColumns.append(newKey);
        }
    }

    //write new column keys
    for (int i = 0; i < columnsToDecrement.size(); i++) {
        query.prepare(QString("UPDATE '%1' SET key=:newKey WHERE key=:oldKey")
                      .arg(metadataTable));
        query.bindValue(":newKey", decrementedColumns.at(i));
        query.bindValue(":oldKey", columnsToDecrement.at(i));
        query.exec();
    }

    //delete column keys marked for deletion
    query.exec(QString("DELETE FROM '%1' WHERE key='del_me'").arg(metadataTable));

    //commit transaction
    db.commit();

    //update cached metadata
    updateFieldNameCache();

    //notify the change
    if (collectionId == m_currentCollectionId)
        emit currentCollectionChanged();
}

int MetadataEngine::addContentFile(const QString &fileName,
                                    const QString &hashName,
                                   const QString &originalDirPath)
{
    QSqlDatabase db = DatabaseManager::getInstance().getDatabase();
    QSqlQuery query(db);
    int id = 0; //invalid id

    //start transaction to speed up writes
    db.transaction();

    //add file
    query.prepare("INSERT INTO files (\"name\",\"hash_name\",\"date_added\",\"original_dir_path\")"
                  " VALUES (:name, :hash_name, :date_added, :original_dir_path)");
    query.bindValue(":name", fileName);
    query.bindValue(":hash_name", hashName);
    query.bindValue(":date_added", QDateTime::currentDateTime());
    query.bindValue(":original_dir_path", originalDirPath);
    query.exec();

    //get id
    query.prepare("SELECT _id FROM files WHERE hash_name=:hashName");
    query.bindValue(":hashName", hashName);
    query.exec();

    if (query.next()) {
        id = query.value(0).toInt();
    }

    //commit transaction
    db.commit();

    return id;
}

void MetadataEngine::removeContentFile(int fileId)
{
    QSqlDatabase db = DatabaseManager::getInstance().getDatabase();
    QSqlQuery query(db);

    //start transaction to speed up writes
    db.transaction();

    //rm file
    query.prepare("DELETE FROM files WHERE _id=:fileId");
    query.bindValue(":fileId", fileId);
    query.exec();

    //commit transaction
    db.commit();
}

void MetadataEngine::removeContentFile(const QStringList &fileIdList)
{
    QSqlDatabase db = DatabaseManager::getInstance().getDatabase();
    QSqlQuery query(db);

    //start transaction to speed up writes
    db.transaction();

    //rm files
    QString sql = QString("DELETE FROM files WHERE _id IN (%1)")
            .arg(fileIdList.join(","));
    query.exec(sql);

    //commit transaction
    db.commit();
}

void MetadataEngine::updateContentFile(int fileId,
                                       const QString &fileName,
                                       const QString &hashName,
                                       const QDateTime &dateAdded)
{
    QSqlDatabase db = DatabaseManager::getInstance().getDatabase();
    QSqlQuery query(db);

    //start transaction to speed up writes
    db.transaction();

    //update file
    query.prepare("UPDATE files SET name=:fileName, hash_name=:hashName"
                  ", date_added=:dateAdded WHERE _id=:id"); //origDirPath should not be changed, so leave it out
    query.bindValue(":name", fileName);
    query.bindValue(":hash_name", hashName);
    query.bindValue(":date_added", dateAdded);
    query.bindValue(":id", fileId);
    query.exec();

    //commit transaction
    db.commit();
}

bool MetadataEngine::getContentFile(int fileId,
                                    QString &fileName,
                                    QString &hashName,
                                    QDateTime &dateAdded,
                                    QString &origDirPath)
{
    QSqlDatabase db = DatabaseManager::getInstance().getDatabase();
    QSqlQuery query(db);

    query.prepare("SELECT name,hash_name,date_added,original_dir_path FROM "
                  "files WHERE _id=:fileId");
    query.bindValue(":fileId", fileId);
    query.exec();

    if (query.next()) {
        fileName = query.value(0).toString();
        hashName = query.value(1).toString();
        dateAdded = query.value(2).toDateTime();
        origDirPath = query.value(3).toString();
        return true;
    } else {
        return false;
    }
}

QHash<int,QString> MetadataEngine::getAllContentFiles()
{
    QSqlDatabase db = DatabaseManager::getInstance().getDatabase();
    QSqlQuery query(db);
    QHash<int,QString> map;

    query.exec("SELECT _id,hash_name FROM files");

    while (query.next()) {
        map.insert(query.value(0).toInt(), query.value(1).toString());
    }

    return map;
}

int MetadataEngine::getContentFileId(const QString &hashName)
{
    int id = 0; //invalid id

    QSqlDatabase db = DatabaseManager::getInstance().getDatabase();
    QSqlQuery query(db);

    query.prepare("SELECT _id FROM files WHERE hash_name=:hashName");
    query.bindValue(":hashName", hashName);
    query.exec();

    if (query.next()) {
        id = query.value(0).toInt();
    }

    return id;
}

QStringList MetadataEngine::getAllCollectionContentFiles(const int collectionId,
                                                         const int fieldId)
{
    QStringList fileIdList;
    QList<int> fileFieldList;
    QSqlDatabase db = DatabaseManager::getInstance().getDatabase();
    QSqlQuery query(db);

    if (fieldId == -1) { //process all fields of type content file
        int fieldsCount = getFieldCount(collectionId);
        for (int i = 1; i < fieldsCount; i++) { //1 because of _id
            switch (getFieldType(i, collectionId)) {
            case MetadataEngine::ImageType:
            case MetadataEngine::FilesType:
                fileFieldList.append(i);
                break;
            default:
                break;
            }
        }
    } else { //process only specified field id
        fileFieldList.append(fieldId);
    }

    //get all file ids
    QString tableName = getTableName(collectionId);
    foreach (int f, fileFieldList) {
        QString sql = QString("SELECT \"%1\" FROM \"%2\"")
                .arg(QString::number(f)).arg(tableName);
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
    }

    return fileIdList;
}


void MetadataEngine::setDirtyCurrentColleectionId()
{
    m_currentCollectionId = 0;
}

int MetadataEngine::getCollectionOrder(const int collectionId)
{
    int order = 0;

    QSqlQuery query(DatabaseManager::getInstance().getDatabase());
    QString sql = QString("SELECT c_order FROM collections WHERE _id='%1'")
            .arg(collectionId);
    query.exec(sql);

    if (query.next())
        order = query.value(0).toInt();

    return order;
}

int MetadataEngine::moveCollectionListOrderOneStepUp(const int collectionId,
                                                        const int currentOrder)
{
    if (currentOrder <= 1) return 0;
    QSqlQuery query(DatabaseManager::getInstance().getDatabase());

    //move every collection up below current one
    QString sql = QString("UPDATE collections SET c_order=c_order+1 WHERE "
                          "c_order=(SELECT c_order-1 FROM collections WHERE _id='%1')")
            .arg(collectionId);
    query.exec(sql);

    //update current collection to new order
    sql = QString("UPDATE collections SET c_order=c_order-1 WHERE _id='%1'")
            .arg(collectionId);
    query.exec(sql);

    return currentOrder - 1;
}

int MetadataEngine::moveCollectionListOrderOneStepDown(const int collectionId,
                                                       const int currentOrder)
{
    if (currentOrder < 1) return 0;
    QSqlQuery query(DatabaseManager::getInstance().getDatabase());

    //move every collection down below current one
    QString sql = QString("UPDATE collections SET c_order=c_order-1 WHERE "
                          "c_order=(SELECT c_order+1 FROM collections WHERE _id='%1')")
            .arg(collectionId);
    query.exec(sql);

    //update current collection to new order
    sql = QString("UPDATE collections SET c_order=c_order+1 WHERE _id='%1'")
            .arg(collectionId);
    query.exec(sql);

    return currentOrder + 1;
}

int MetadataEngine::getMaxCollectionOrderCount()
{
    int maxOrder = 0;

    QSqlQuery query(DatabaseManager::getInstance().getDatabase());
    QString sql = QString("SELECT c_order FROM collections ORDER BY c_order DESC");
    query.exec(sql);

    if (query.next())
        maxOrder = query.value(0).toInt();

    return maxOrder;
}

int MetadataEngine::getNewCollectionOrderCount()
{
    return getMaxCollectionOrderCount() + 1;
}


//-----------------------------------------------------------------------------
// Private
//-----------------------------------------------------------------------------

MetadataEngine::MetadataEngine(QObject *parent) :
    QObject(parent)
{
    m_currentCollectionFieldNameList = new QStringList;
    getCurrentCollectionId(); //load last used collection id to cache
}

MetadataEngine::~MetadataEngine()
{
    m_currentCollectionFieldNameList->clear();
    delete m_currentCollectionFieldNameList;
}

QAbstractItemModel* MetadataEngine::createStandardModel(const int collectionId)
{
    StandardModel *model = new StandardModel(this, 0);

    //if id is specified, init collection
    if (collectionId) {
        QString tableName = getTableName(collectionId);
        model->setTable(tableName);
        model->select();
    }

    return model;
}

void MetadataEngine::updateFieldNameCache()
{
    m_currentCollectionFieldNameList->clear();

    int columnCount = getFieldCountFromDatabase(m_currentCollectionId);
    for (int i = 0; i < columnCount; i++) {
        m_currentCollectionFieldNameList->append(
                    getFieldNameFromDatabase(i, m_currentCollectionId));
    }

}

QString MetadataEngine::getFieldNameFromDatabase(const int column,
                                                 int collectionId) const
{
    if (column == 0) return "ID"; //first column is always _id

    QString name("_invalid_column_name_"); //placeholder for invalid column name
    QSqlDatabase db = DatabaseManager::getInstance().getDatabase();
    QSqlQuery query(db);
    QString tableName = getTableName(collectionId);
    QString columnKey = QString("col%1_name").arg(column);

    //using sql string because bindValue() fails when using placeholder in table names
    QString sql = QString("SELECT value FROM '%1' WHERE key='%2'")
                         .arg(tableName.append("_metadata")).arg(columnKey);
    query.exec(sql);

    if (query.next())
        name = query.value(0).toString();

    return name;

}

int MetadataEngine::getFieldCountFromDatabase(const int collectionId) const
{
    int count = 0;

    QString metadataTable = getTableName(collectionId).append("_metadata");
    QSqlQuery query(DatabaseManager::getInstance().getDatabase());
    QString sql = QString("SELECT value FROM '%1' WHERE key='column_count'")
                         .arg(metadataTable);
    query.exec(sql);

    if (query.next())
        count = query.value(0).toInt();

    return count;
}

void MetadataEngine::setFieldCount(const int collectionId, int columnCount)
{
    QString metadataTable = getTableName(collectionId).append("_metadata");
    QSqlQuery query(DatabaseManager::getInstance().getDatabase());

    query.prepare(QString("UPDATE '%1' SET value=:count WHERE key='column_count'")
                         .arg(metadataTable)); //arg because bindValue() fails
    query.bindValue(":count", columnCount);
    query.exec();
}

QString MetadataEngine::dataTypeSqlName(FieldType type)
{
    QString s;

    //update this on new data types
    switch (type) {
    case TextType:
    case DateType:
    case CreationDateType:
    case ModDateType:
    case FilesType:
    case URLTextType:
    case EmailTextType:
        s = "TEXT";
        break;
    case NumericType:
    case CheckboxType:
    case ImageType:
    case ComboboxType:
    case ProgressType:
        s = "INTEGER";
        break;
    default:
        s = "TEXT";
        break;
    }

    return s;
}
