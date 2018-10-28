/**
  * \class DatabaseManager
  * \brief This class manages the main database and connections
  *        used in the application. This includes methods for db
  *        creation/deletion/backup.
  *        See /stuff/doc/Standard Database Design.
  * \author Giorgio Wicklein - GIOWISYS Software
  * \date 08/06/2012
  */

#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H


//-----------------------------------------------------------------------------
// Headers
//-----------------------------------------------------------------------------

#include <QtCore/QString>


//-----------------------------------------------------------------------------
// Forward declarations
//-----------------------------------------------------------------------------

class QSqlDatabase;


//-----------------------------------------------------------------------------
// DatabaseManager
//-----------------------------------------------------------------------------

class DatabaseManager
{
public:
    static DatabaseManager& getInstance();
    static void destroy();

    /** Return the database connection */
    QSqlDatabase getDatabase() const;

    /** Begin transaction mode */
    void beginTransaction();

    /** End transaction by committing to db */
    void endTransaction();

    /** Free unused space from database file */
    void optimizeDatabaseSize();

    /** Remove all entries from the specified table */
    void truncateTable(const QString &tableName);

    /** Get the size of the database file in bytes */
    int getDatabaseFileSize();

    /** Get the name of the database file */
    QString getDatabaseName();

    /** Get the path of the database file */
    QString getDatabasePath();

private:
    DatabaseManager();
    DatabaseManager(const DatabaseManager&) {}
    ~DatabaseManager();

    /** This method creates a new database if needed and opens it */
    void openDatabase();

    /** Close database */
    void closeDatabase();

    /** Whether the db file exists or not */
    bool databaseExists() const;

    /** Create the db file and its initial structure */
    void initDatabase(QSqlDatabase &database);

    /** Delete permanently the db file */
    void deleteDatabase();

    /** Query current database for database version */
    int getDatabaseVersion();

    /** Upgrade the database to the new version */
    bool upgradeDatabase(const int oldVersion);

    static DatabaseManager *m_instance;
    QString m_databasePath; /**< The full path, including db name
                              *  to the main db file
                              */
    QString m_databaseName; /**< The name of main database file */
};

#endif // DATABASEMANAGER_H
