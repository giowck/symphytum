#include <QtCore/QString>
#include <QtTest/QtTest>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>

#include "../../components/databasemanager.h"

class DatabaseManagerTest : public QObject
{
    Q_OBJECT
    
public:
    DatabaseManagerTest();
    
private Q_SLOTS:
    void testGetDatabase();
    void testTransaction();
    void testOptimizeDatabaseSize();
    void testTruncateTable();
    void testGetDatabaseFileSize();

private:
    DatabaseManager *m_database;
};

DatabaseManagerTest::DatabaseManagerTest()
{
    m_database = &DatabaseManager::getInstance();
}

void DatabaseManagerTest::testGetDatabase()
{
    QSqlDatabase db = m_database->getDatabase();

    QVERIFY(db.isValid());
    QVERIFY(db.isOpen());
}

void DatabaseManagerTest::testTransaction()
{
    QSqlQuery query(m_database->getDatabase());

    m_database->beginTransaction();
    query.exec("CREATE TABLE test ('_id' INTEGER PRIMARY KEY, 'value' INTEGER)");
    query.exec("INSERT INTO test('value') VALUES (99)");
    m_database->endTransaction();

    int r = 0;
    query.exec("SELECT value FROM test");
    if (query.next())
        r = query.value(0).toInt();

    QVERIFY(r == 99);
}

void DatabaseManagerTest::testOptimizeDatabaseSize()
{
    m_database->optimizeDatabaseSize();

    QVERIFY(m_database->getDatabase().isValid());
    QVERIFY(m_database->getDatabase().isOpen());
}

void DatabaseManagerTest::testTruncateTable()
{
    m_database->truncateTable("test");

    QSqlQuery query(m_database->getDatabase());
    query.exec("SELECT value FROM test");

    QVERIFY(!query.next());
}

void DatabaseManagerTest::testGetDatabaseFileSize()
{
    int size = m_database->getDatabaseFileSize();

    QVERIFY(size > 0);
}

QTEST_APPLESS_MAIN(DatabaseManagerTest)

#include "tst_databasemanagertest.moc"
