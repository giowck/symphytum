/*
 *  Copyright (c) 2012 Giorgio Wicklein <giowckln@gmail.com>
 */

//-----------------------------------------------------------------------------
// Hearders
//-----------------------------------------------------------------------------

#include "databasemanager.h"
#include "../utils/definitionholder.h"
#include "../components/settingsmanager.h"
#include "filemanager.h"

#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlError>
#include <QtCore/QVariant>
#include <QtWidgets/QMessageBox>
#include <QtGui/QDesktopServices>
#include <QtCore/QDir>


//-----------------------------------------------------------------------------
// Defines
//-----------------------------------------------------------------------------

#define SQL_CREATE_TABLE_COLLECTIONS \
    "CREATE TABLE \"collections\" (\"_id\" INTEGER PRIMARY KEY, \"name\" TEXT," \
    " \"type\" INTEGER, \"table_name\" TEXT, \"c_order\" INTEGER)"
#define SQL_CREATE_TABLE_INFO \
    "CREATE TABLE \"symphytum_info\" (\"_id\" INTEGER PRIMARY KEY , \"key\"" \
    " TEXT, \"value\" TEXT)"
#define SQL_CREATE_TABLE_FILES \
    "CREATE TABLE \"files\" (\"_id\" INTEGER PRIMARY KEY, \"name\" TEXT," \
    " \"hash_name\" TEXT, \"date_added\" TEXT, \"original_dir_path\" TEXT)"

#define SQL_CREATE_TABLE_ALARMS \
    "CREATE TABLE \"alarms\" (\"_id\" INTEGER PRIMARY KEY, \"collection_id\" INTEGER," \
    " \"field_id\" INTEGER, \"record_id\" INTEGER, \"date\" TEXT)"


//-----------------------------------------------------------------------------
// Static init
//-----------------------------------------------------------------------------

DatabaseManager* DatabaseManager::m_instance = nullptr;

//-----------------------------------------------------------------------------
// Public
//-----------------------------------------------------------------------------

DatabaseManager& DatabaseManager::getInstance()
{
    if (!m_instance)
        m_instance = new DatabaseManager();
    return *m_instance;
}

void DatabaseManager::destroy()
{
    if (m_instance)
        delete m_instance;
    m_instance = nullptr;
}

QSqlDatabase DatabaseManager::getDatabase() const
{
    return QSqlDatabase::database("main");
}

void DatabaseManager::beginTransaction()
{
    QSqlDatabase::database("main").transaction();
}

void DatabaseManager::endTransaction()
{
    QSqlDatabase::database("main").commit();
}

void DatabaseManager::optimizeDatabaseSize()
{
    QSqlQuery query(getDatabase());

    //SQLite has a VACUUM statement
    //to free unused space from
    //database file
    query.exec(QString("VACUUM;"));
}

void DatabaseManager::truncateTable(const QString &tableName)
{
    QSqlQuery query(getDatabase());

    //SQLite doesn't have a TRUNCATE TABLE statement
    //but when using a DELETE statement without WHERE
    //clause SQLite will optimize the query by using an
    //intern truncate-like operation
    query.exec(QString("DELETE FROM %1").arg(tableName));
}

int DatabaseManager::getDatabaseFileSize()
{
    int bytes = 0;

    QFileInfo file(m_databasePath);
    bytes = file.size();

    return bytes;
}

QString DatabaseManager::getDatabaseName()
{
    return m_databaseName;
}

QString DatabaseManager::getDatabasePath()
{
    return m_databasePath;
}


//-----------------------------------------------------------------------------
// Private
//-----------------------------------------------------------------------------

DatabaseManager::DatabaseManager()
{
    SettingsManager sm;
    QString dataDir = sm.restoreCustomDatabaseDir();
    if (dataDir.isEmpty()) { //use default
        if (DefinitionHolder::WIN_PORTABLE) {
            dataDir = "portable_data";
        } else {
            dataDir = QStandardPaths::standardLocations(
                        QStandardPaths::DataLocation).at(0);
        }
    }
    m_databaseName = "data.db";
    m_databasePath = dataDir.append("/");
    m_databasePath.append(m_databaseName);

    if (!QDir(dataDir).exists()) {
        QDir::current().mkpath(dataDir);
    }

    openDatabase();
}

DatabaseManager::~DatabaseManager()
{
    closeDatabase();
}

void DatabaseManager::openDatabase()
{
    bool db_exists = databaseExists();

    //if already open
    if (QSqlDatabase::database("main").isValid())
        return;

    QSqlDatabase database = QSqlDatabase::addDatabase("QSQLITE", "main");
    database.setDatabaseName(m_databasePath);

    bool open = database.open();

    if (!open) {
        QString err = database.lastError().text();
        QMessageBox::critical(nullptr, QObject::tr("Database Error"),
                              QObject::tr("Failed to open the database file: %1")
                              .arg(err));
        return;
    }

    if (!db_exists && open) {
        initDatabase(database);
    }


    //check database version
    //upgrade if possible
    int version = getDatabaseVersion();
    if (version < DefinitionHolder::DATABASE_VERSION) {
        //create backup in case the upgrade fails
        if (!QFile::copy(m_databasePath, m_databasePath + ".backup")) {
            QMessageBox::critical(nullptr, QObject::tr("Database Version Upgrade Failed"),
                                  QObject::tr("Failed to create database backup!"));
            closeDatabase();
            return;
        }

        if (!upgradeDatabase(version)) {
            QMessageBox::critical(nullptr, QObject::tr("Database Version Upgrade Failed"),
                                  QObject::tr("Failed to upgrade the database file: db_version %1")
                                  .arg(version));
            closeDatabase();

            //restore backup
            if ((!QFile::remove(m_databasePath)) ||
                    (!QFile::copy(m_databasePath + ".backup", m_databasePath))) {
                QMessageBox::critical(nullptr, QObject::tr("Database Version Upgrade Failed"),
                                      QObject::tr("Failed to restore database backup"));
            } else {
                QFile::remove(m_databasePath + ".backup");
            }

            return;
        }

        //remove backup
        QFile::remove(m_databasePath + ".backup");
    } else if (version > DefinitionHolder::DATABASE_VERSION) {
        QMessageBox::critical(nullptr, QObject::tr("Database Version Incompatible"),
                              QObject::tr("Failed to open the database file: db_version %1. "
                                          " Please upgrade %2 to a newer version "
                                          "and then try again!")
                              .arg(version).arg(DefinitionHolder::NAME));
        closeDatabase();
        return;
    }
}

void DatabaseManager::closeDatabase()
{
    QSqlDatabase database = QSqlDatabase::database("main");
    database.close();
    QSqlDatabase::removeDatabase("main");
}

bool DatabaseManager::databaseExists() const
{
    return QFile::exists(m_databasePath);
}

void DatabaseManager::initDatabase(QSqlDatabase &database)
{
    QSqlQuery query(database);

    //to speed things massively up
    //move operations in one big transaction
    database.transaction();

    //create collections table
    query.exec(SQL_CREATE_TABLE_COLLECTIONS);

    //create info table
    query.exec(SQL_CREATE_TABLE_INFO);

    //create file table
    query.exec(SQL_CREATE_TABLE_FILES);

    //create alarm table
    query.exec(SQL_CREATE_TABLE_ALARMS);

    //init info data
    query.prepare("INSERT INTO \"symphytum_info\" (\"key\",\"value\") VALUES"
                  "(\"db_version\", :version)");
    query.bindValue(":version", DefinitionHolder::DATABASE_VERSION);
    query.exec();

    //init example data
    query.exec("INSERT INTO \"collections\" (\"name\",\"type\",\"table_name\", \"c_order\")"
               " VALUES (\"Medicinal Plants\",1,\"cb92ee55f44577b584464c13f47fa3771\", 1)");
    query.exec("CREATE TABLE \"cb92ee55f44577b584464c13f47fa3771\" (\"_id\" "
               "INTEGER PRIMARY KEY , \"1\" TEXT, \"2\" TEXT, \"3\" TEXT, \"4\" INTEGER,"
               " \"5\" INTEGER, \"6\" INTEGER)");
    query.exec("INSERT INTO \"cb92ee55f44577b584464c13f47fa3771\" "
               "VALUES (\"1\",\"Symphytum officinale\",\"Synthesis, Recovery\",\"Painkiller,"
               " anti-inflammatory property on skin and mucosae\",\"1\",\"1\",\"1\")");
    query.exec("INSERT INTO \"cb92ee55f44577b584464c13f47fa3771\" VALUES (\"2\",\"Calendula"
               " officinalis\",\"Healing, Wound healing\",\"Stomach discomfort, "
               "severe wounds, soul injury\",\"2\",\"0\",\"2\")");
    query.exec("INSERT INTO \"cb92ee55f44577b584464c13f47fa3771\" VALUES "
               "(\"3\",\"Coffea arabica\",\"Radical change, Adaption\",\"Balance "
               "of neurotransmitters, nervous system, heart, circulation, "
               "leave illusions, liberation\",\"0\",\"1\",\"3\")");
    query.exec("CREATE TABLE \"cb92ee55f44577b584464c13f47fa3771_metadata\""
               " (\"_id\" INTEGER PRIMARY KEY , \"key\" TEXT, \"value\" TEXT)");
    query.exec("INSERT INTO \"cb92ee55f44577b584464c13f47fa3771_metadata\" VALUES (\"1\",\"column_count\",\"7\")");
    query.exec("INSERT INTO \"cb92ee55f44577b584464c13f47fa3771_metadata\" VALUES (\"2\",\"col1_pos\",\"0;0\")");
    query.exec("INSERT INTO \"cb92ee55f44577b584464c13f47fa3771_metadata\" VALUES (\"3\",\"col1_size\",\"1;1\")");
    query.exec("INSERT INTO \"cb92ee55f44577b584464c13f47fa3771_metadata\" VALUES (\"4\",\"col1_display\",\"\")");
    query.exec("INSERT INTO \"cb92ee55f44577b584464c13f47fa3771_metadata\" VALUES (\"5\",\"col1_edit\",\"\")");
    query.exec("INSERT INTO \"cb92ee55f44577b584464c13f47fa3771_metadata\" VALUES (\"6\",\"col1_trigger\",\"\")");
    query.exec("INSERT INTO \"cb92ee55f44577b584464c13f47fa3771_metadata\" VALUES (\"7\",\"col1_name\",\"Name\")");
    query.exec("INSERT INTO \"cb92ee55f44577b584464c13f47fa3771_metadata\" VALUES (\"8\",\"col1_type\",\"1\")");
    query.exec("INSERT INTO \"cb92ee55f44577b584464c13f47fa3771_metadata\" VALUES (\"9\",\"col2_pos\",\"1;0\")");
    query.exec("INSERT INTO \"cb92ee55f44577b584464c13f47fa3771_metadata\" VALUES (\"10\",\"col2_size\",\"1;1\")");
    query.exec("INSERT INTO \"cb92ee55f44577b584464c13f47fa3771_metadata\" VALUES (\"11\",\"col2_display\",\"\")");
    query.exec("INSERT INTO \"cb92ee55f44577b584464c13f47fa3771_metadata\" VALUES (\"12\",\"col2_edit\",\"\")");
    query.exec("INSERT INTO \"cb92ee55f44577b584464c13f47fa3771_metadata\" VALUES (\"13\",\"col2_trigger\",\"\")");
    query.exec("INSERT INTO \"cb92ee55f44577b584464c13f47fa3771_metadata\" VALUES (\"14\",\"col2_name\",\"Keywords\")");
    query.exec("INSERT INTO \"cb92ee55f44577b584464c13f47fa3771_metadata\" VALUES (\"15\",\"col2_type\",\"1\")");
    query.exec("INSERT INTO \"cb92ee55f44577b584464c13f47fa3771_metadata\" VALUES (\"16\",\"col3_pos\",\"0;1\")");
    query.exec("INSERT INTO \"cb92ee55f44577b584464c13f47fa3771_metadata\" VALUES (\"17\",\"col3_size\",\"1;2\")");
    query.exec("INSERT INTO \"cb92ee55f44577b584464c13f47fa3771_metadata\" VALUES (\"18\",\"col3_display\",null)");
    query.exec("INSERT INTO \"cb92ee55f44577b584464c13f47fa3771_metadata\" VALUES (\"19\",\"col3_edit\",null)");
    query.exec("INSERT INTO \"cb92ee55f44577b584464c13f47fa3771_metadata\" VALUES (\"20\",\"col3_trigger\",null)");
    query.exec("INSERT INTO \"cb92ee55f44577b584464c13f47fa3771_metadata\" VALUES (\"21\",\"col3_name\",\"Healing effect\")");
    query.exec("INSERT INTO \"cb92ee55f44577b584464c13f47fa3771_metadata\" VALUES (\"22\",\"col3_type\",\"1\")");
    query.exec("INSERT INTO \"cb92ee55f44577b584464c13f47fa3771_metadata\" VALUES (\"23\",\"col4_pos\",\"1;2\")");
    query.exec("INSERT INTO \"cb92ee55f44577b584464c13f47fa3771_metadata\" VALUES (\"24\",\"col4_size\",\"1;1\")");
    query.exec("INSERT INTO \"cb92ee55f44577b584464c13f47fa3771_metadata\" VALUES (\"25\",\"col4_display\",\"items:10 ml,30 ml,50 ml;\")");
    query.exec("INSERT INTO \"cb92ee55f44577b584464c13f47fa3771_metadata\" VALUES (\"26\",\"col4_edit\",null)");
    query.exec("INSERT INTO \"cb92ee55f44577b584464c13f47fa3771_metadata\" VALUES (\"27\",\"col4_trigger\",null)");
    query.exec("INSERT INTO \"cb92ee55f44577b584464c13f47fa3771_metadata\" VALUES (\"28\",\"col4_name\",\"Recommended dosage\")");
    query.exec("INSERT INTO \"cb92ee55f44577b584464c13f47fa3771_metadata\" VALUES (\"29\",\"col4_type\",\"7\")");
    query.exec("INSERT INTO \"cb92ee55f44577b584464c13f47fa3771_metadata\" VALUES (\"30\",\"col5_pos\",\"1;1\")");
    query.exec("INSERT INTO \"cb92ee55f44577b584464c13f47fa3771_metadata\" VALUES (\"31\",\"col5_size\",\"1;1\")");
    query.exec("INSERT INTO \"cb92ee55f44577b584464c13f47fa3771_metadata\" VALUES (\"32\",\"col5_display\",null)");
    query.exec("INSERT INTO \"cb92ee55f44577b584464c13f47fa3771_metadata\" VALUES (\"33\",\"col5_edit\",null)");
    query.exec("INSERT INTO \"cb92ee55f44577b584464c13f47fa3771_metadata\" VALUES (\"34\",\"col5_trigger\",null)");
    query.exec("INSERT INTO \"cb92ee55f44577b584464c13f47fa3771_metadata\" VALUES (\"35\",\"col5_name\",\"Harvest before bloom\")");
    query.exec("INSERT INTO \"cb92ee55f44577b584464c13f47fa3771_metadata\" VALUES (\"36\",\"col5_type\",\"6\")");
    query.exec("INSERT INTO \"cb92ee55f44577b584464c13f47fa3771_metadata\" VALUES (\"37\",\"col6_pos\",\"2;0\")");
    query.exec("INSERT INTO \"cb92ee55f44577b584464c13f47fa3771_metadata\" VALUES (\"38\",\"col6_size\",\"1;3\")");
    query.exec("INSERT INTO \"cb92ee55f44577b584464c13f47fa3771_metadata\" VALUES (\"39\",\"col6_display\",\"\")");
    query.exec("INSERT INTO \"cb92ee55f44577b584464c13f47fa3771_metadata\" VALUES (\"40\",\"col6_edit\",\"\")");
    query.exec("INSERT INTO \"cb92ee55f44577b584464c13f47fa3771_metadata\" VALUES (\"41\",\"col6_trigger\",\"\")");
    query.exec("INSERT INTO \"cb92ee55f44577b584464c13f47fa3771_metadata\" VALUES (\"42\",\"col6_name\",\"Photo\")");
    query.exec("INSERT INTO \"cb92ee55f44577b584464c13f47fa3771_metadata\" VALUES (\"43\",\"col6_type\",\"9\")");
    query.exec("INSERT INTO \"files\" VALUES (\"1\",\"symphytum.jpg\",\"25993661ea0bdede9699836f9ba0956b.jpg\",\"2012-12-01T16:00:00\", \"\")");
    query.exec("INSERT INTO \"files\" VALUES (\"2\",\"calendula.jpg\",\"81f87c38d8fd4cff00f35847e454e753.jpg\",\"2012-12-01T16:00:00\", \"\")");
    query.exec("INSERT INTO \"files\" VALUES (\"3\",\"coffea.jpg\",\"fd461a1f28d6682993422d65dafa2ddf.jpg\",\"2012-12-01T16:00:00\", \"\")");
    query.exec("INSERT INTO \"symphytum_info\" (\"key\",\"value\") VALUES (\"current_collection\",\"1\")");

    if (!database.commit()) {
        QString err = query.lastError().text();
        QMessageBox::critical(nullptr, QObject::tr("Database Error"),
                              QObject::tr("Failed to initialize "
                                          "the database: %1")
                              .arg(err));
    }

    //copy example files
    QString filesDir = FileManager().getFilesDirectory();
    QFile::copy(":/images/sample/symphytum.jpg", filesDir + "25993661ea0bdede9699836f9ba0956b.jpg");
    QFile::copy(":/images/sample/calendula.jpg", filesDir + "81f87c38d8fd4cff00f35847e454e753.jpg");
    QFile::copy(":/images/sample/coffea.jpg", filesDir + "fd461a1f28d6682993422d65dafa2ddf.jpg");

    //fix file permission because qrc files are read only
    QFile f;
    f.setFileName(filesDir + "25993661ea0bdede9699836f9ba0956b.jpg");
    f.setPermissions(QFile::ReadOwner | QFile::WriteOwner | QFile::ReadOther | QFile::WriteOther);
    f.setFileName(filesDir + "81f87c38d8fd4cff00f35847e454e753.jpg");
    f.setPermissions(QFile::ReadOwner | QFile::WriteOwner | QFile::ReadOther | QFile::WriteOther);
    f.setFileName(filesDir + "fd461a1f28d6682993422d65dafa2ddf.jpg");
    f.setPermissions(QFile::ReadOwner | QFile::WriteOwner | QFile::ReadOther | QFile::WriteOther);
}

void DatabaseManager::deleteDatabase()
{
    closeDatabase();

    QFile::remove(m_databasePath);
}

int DatabaseManager::getDatabaseVersion()
{
    int v = 0;

    QSqlQuery query(getDatabase());

    query.exec("SELECT value FROM symphytum_info WHERE key='db_version'");
    if (query.next()) {
        QString v_string = query.value(0).toString();
        v = v_string.toInt();
    }

    return v;
}

bool DatabaseManager::upgradeDatabase(const int oldVersion)
{
    //handle database version upgrades
    int currentUpgradeVersion = oldVersion;
    QSqlDatabase db = getDatabase();
    QSqlQuery query(db);

    //upgrade v1 -> v2
    if (currentUpgradeVersion == 1) {
        //no major change (only new field type URL and email)
        currentUpgradeVersion = 2;
    }
    //upgrade v2 -> v3
    if (currentUpgradeVersion == 2) {
        //file type has a new column for original import dir path
        if (!query.exec("ALTER TABLE \"files\" ADD \"original_dir_path\" TEXT;")) {
            QMessageBox::critical(nullptr, QObject::tr("Database Version Upgrade Failed"),
                                  QObject::tr("Failed to execute %1. Error: %2")
                                  .arg(query.lastQuery()).arg(query.lastError().text()));
            return false;
        }

        //add collection order info
        if (!query.exec("ALTER TABLE \"collections\" ADD \"c_order\" INTEGER;")) {
            QMessageBox::critical(nullptr, QObject::tr("Database Version Upgrade Failed"),
                                  QObject::tr("Failed to execute %1. Error: %2")
                                  .arg(query.lastQuery()).arg(query.lastError().text()));
            return false;
        }

        //get all collection IDs
        QList<int> collectionIDs;
        bool error = false;
        error = !query.exec("SELECT \"_id\" FROM \"collections\"");
        while (query.next()) {
            collectionIDs.append(query.value(0).toInt());
        }

        //set order based on position in list
        int currentOrder = 0;
        foreach (int id, collectionIDs) {
            currentOrder++;
            query.prepare(QString("UPDATE \"collections\" "
                                  "SET \"c_order\"=:newOrder WHERE \"_id\"=:collectionID"));
            query.bindValue(":newOrder", currentOrder);
            query.bindValue(":collectionID", id);
            error |= (!query.exec());
        }

        if (error)  {
            QMessageBox::critical(nullptr, QObject::tr("Database Version Upgrade Failed"),
                                  QObject::tr("Failed to execute %1. Error: %2")
                                  .arg(query.lastQuery()).arg(query.lastError().text()));
            return false;
        }
        currentUpgradeVersion = 3;
    }
    //upgrade v3 -> v4
    if (currentUpgradeVersion == 3) {
        bool error = false;
        QString errorMessage;

        //fix inconsistent data sets, see https://github.com/giowck/symphytum/issues/122
        //first, check if there are any
        //get all collections, including the metadata copy
        QStringList collectionsToCheck;
        error |= !query.exec("SELECT table_name FROM collections");
        while (query.next()) {
            QString c = query.value(0).toString();
            collectionsToCheck.append(c);
            collectionsToCheck.append(c + "_metadata");
        }

        //check for any _id columns that are not PRIMARY KEY
        QStringList collectionsToFix;
        foreach (QString tableName, collectionsToCheck) {
            error |= !query.exec(QString("SELECT sql FROM sqlite_master WHERE tbl_name=\"%1\" "
                                         "AND sql NOT LIKE \"%PRIMARY%\"").arg(tableName));
            if (query.next()) {
                collectionsToFix.append(tableName);
            }
        }

        //correct NULL _id records if any by adding PRIMARY KEY constraint
        if (collectionsToFix.size() > 0) {
            //start transaction to speed up writes
            db.transaction();

            foreach (QString tableName, collectionsToFix) {
                //get original CREATE TABLE statement
                error |= !query.exec(QString("SELECT sql FROM sqlite_master "
                                             "WHERE tbl_name=\"%1\"").arg(tableName));
                if (query.next()) {
                    QString create_sql = query.value(0).toString();
                    //add PRIMARY KEY constraint
                    create_sql.replace("_id INT", "_id INTEGER PRIMARY KEY");
                    //rename table
                    QString tableNameFixed = tableName + "_fixed";
                    create_sql.replace(tableName, tableNameFixed);
                    //create new fixed table
                    error |= !query.exec(create_sql);
                    //copy data over to new table
                    error |= !query.exec(QString("INSERT INTO %1 SELECT * FROM %2")
                                         .arg(tableNameFixed).arg(tableName));
                    //delete old table
                    error |= !query.exec(QString("DROP TABLE %1").arg(tableName));
                    //rename fixed table to original name
                    error |= !query.exec(QString("ALTER TABLE %1 RENAME TO %2")
                                         .arg(tableNameFixed).arg(tableName));
                }
            }

            //commit transaction
            db.commit();
        }

        if (error)  {
            QMessageBox::critical(nullptr, QObject::tr("Database Version Upgrade Failed"),
                                  QObject::tr("Error: failed in v3 -> v4"));
            return false;
        }

        currentUpgradeVersion = 4;
    }
    //add new blocks on new versions here

    //upgrade done
    //so upgrade version info
    query.prepare("UPDATE symphytum_info SET value=:version WHERE key='db_version'");
    query.bindValue(":version", DefinitionHolder::DATABASE_VERSION);
    return query.exec();
}
