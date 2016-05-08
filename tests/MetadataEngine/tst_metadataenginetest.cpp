#include <QString>
#include <QtTest>
#include <QSqlQuery>

#include "../../components/metadataengine.h"
#include "../../components/databasemanager.h"

class MetadataEngineTest : public QObject
{
    Q_OBJECT
    
public:
    MetadataEngineTest();
    ~MetadataEngineTest();
    
private Q_SLOTS:
    void testCollectionId();
    void testCollectionName();
    void testFieldName();
    void testFieldCount();
    void testFieldType();
    void testFieldPos();
    void testFieldSize();
    void testFieldProperty();
    void testCreateModel();
    void testCreateCollection();
    void testDeleteCollection();
    void testCreateField();
    void testDeleteField();
    void testModifyField();
    void testFileMetadata();

private:
    MetadataEngine *m_metadataEngine;
    DatabaseManager *m_databaseManager;
};

MetadataEngineTest::MetadataEngineTest()
{
    m_metadataEngine = &MetadataEngine::getInstance();
    m_databaseManager = &DatabaseManager::getInstance();
}

MetadataEngineTest::~MetadataEngineTest()
{
    m_metadataEngine = 0;
    m_databaseManager = 0;
}

void MetadataEngineTest::testCollectionId()
{
    int originalId = -1;
    int newId = 99;

    originalId = m_metadataEngine->getCurrentCollectionId();
    QVERIFY2((originalId != -1),
             "Get collection ID not working");

    m_metadataEngine->setCurrentCollectionId(newId);
    QVERIFY2((newId == m_metadataEngine->getCurrentCollectionId()),
             "Set collection ID not working");

    //set original back
    m_metadataEngine->setCurrentCollectionId(originalId);
}

void MetadataEngineTest::testCollectionName()
{
    //this is the same string from database manager where the default
    //collection name is defined for example data
    QString defaultCollectionName = "Customers";
    QString currentCollectionName = m_metadataEngine->getCurrentCollectionName();

    QVERIFY2((defaultCollectionName == currentCollectionName),
             "Get collection name not working");

    QString getCollectionName = m_metadataEngine->getCollectionName(
                m_metadataEngine->getCurrentCollectionId());
    QVERIFY(defaultCollectionName == getCollectionName);
}

void MetadataEngineTest::testFieldName()
{
    QString originalName, newName, exampleName;
    int id, column;

    exampleName = "Age"; //from database manager example data
    newName = "Weight";
    column = 3; //age column from example data
    id = m_metadataEngine->getCurrentCollectionId();
    originalName = m_metadataEngine->getFieldName(column);

    QVERIFY(originalName == exampleName);
    QVERIFY(originalName == m_metadataEngine->getFieldName(column, id));
    QVERIFY("_invalid_column_name_" == m_metadataEngine->getFieldName(column, 9999));

    m_metadataEngine->setFieldName(column, newName);
    QVERIFY(newName == m_metadataEngine->getFieldName(column));

    m_metadataEngine->setFieldName(column, newName, id);
    QVERIFY(newName == m_metadataEngine->getFieldName(column, id));

    m_metadataEngine->setFieldName(column, "invalid", 9999);
    QVERIFY(newName == m_metadataEngine->getFieldName(column, id));

    //set back to original name
    m_metadataEngine->setFieldName(column, originalName);

}

void MetadataEngineTest::testFieldCount()
{
    int exampleFieldCount = 4; //field count from exmple data
    int id = m_metadataEngine->getCurrentCollectionId();

    QVERIFY(exampleFieldCount == m_metadataEngine->getFieldCount());
    QVERIFY(exampleFieldCount == m_metadataEngine->getFieldCount(id));
    QVERIFY(0 == m_metadataEngine->getFieldCount(9999));
}

void MetadataEngineTest::testFieldType()
{
    //from example data
    MetadataEngine::FieldType exampleFieldType = MetadataEngine::NumericType;
    int column = 3;
    int id = m_metadataEngine->getCurrentCollectionId();
    QString columnName = "Age";

    QVERIFY(exampleFieldType == m_metadataEngine->getFieldType(column, id));
    QVERIFY(exampleFieldType == m_metadataEngine->getFieldType(column));
    QVERIFY(exampleFieldType == m_metadataEngine->getFieldType(columnName, id));
    QVERIFY(exampleFieldType == m_metadataEngine->getFieldType(columnName));
}

void MetadataEngineTest::testFieldPos()
{
    //from example data
    int column = 3;
    int id = m_metadataEngine->getCurrentCollectionId();
    QString columnName = "Age";

    int x, y;
    bool ok = m_metadataEngine->getFieldCoordinate(column, x, y, id);
    QVERIFY(ok == false);

    m_metadataEngine->setFieldCoordinate(column, 1, 2);
    ok = m_metadataEngine->getFieldCoordinate(column, x, y, id);
    QVERIFY(ok == true);
    QVERIFY((x == 1) && (y == 2));

    m_metadataEngine->setFieldCoordinate(column, -1, -1);
    ok = m_metadataEngine->getFieldCoordinate(column, x, y, id);
    QVERIFY(ok == false);
}

void MetadataEngineTest::testFieldSize()
{
    //from example data
    int column = 3;
    int id = m_metadataEngine->getCurrentCollectionId();
    QString columnName = "Age";

    int w, h;
    m_metadataEngine->getFieldFormLayoutSize(column, w, h, 9999);
    QVERIFY((w == -1) && (h == -1));

    m_metadataEngine->setFieldFormLayoutSize(column, 1, 2);
    m_metadataEngine->getFieldFormLayoutSize(column, w, h, id);
    QVERIFY((w == 1) && (h == 2));

    m_metadataEngine->setFieldFormLayoutSize(column, 1, 1);
}

void MetadataEngineTest::testFieldProperty()
{
    //from example data
    int column = 3;
    int id = m_metadataEngine->getCurrentCollectionId();
    QString columnName = "Age";
    QString testProperty = "key:value;key2:value2a,value2b;";

    QVERIFY(
    m_metadataEngine->getFieldProperties(MetadataEngine::DisplayProperty, column)
            .isEmpty()); //FIXME: err

    m_metadataEngine->setFieldProperties(MetadataEngine::DisplayProperty, column,
                                         testProperty);
    QVERIFY(
    m_metadataEngine->getFieldProperties(MetadataEngine::DisplayProperty, column)
            == testProperty);

    //reset
    m_metadataEngine->setFieldProperties(MetadataEngine::DisplayProperty, column,
                                         "");
}

void MetadataEngineTest::testCreateModel()
{
    //model without init
    QAbstractItemModel *model = m_metadataEngine->createModel(
                                    MetadataEngine::StandardCollection);
    QVERIFY(model != 0);
    QVERIFY(model->columnCount() == 0);

    //model with init
    int id = m_metadataEngine->getCurrentCollectionId();
    int exampleFieldCount = m_metadataEngine->getFieldCount();
    QAbstractItemModel *model2 = m_metadataEngine->createModel(
                                    MetadataEngine::StandardCollection, id);
    QVERIFY(model2 != 0);
    QVERIFY(model2->columnCount() == exampleFieldCount);
}

void MetadataEngineTest::testCreateCollection()
{
    int id = -1;
    QString tableName;
    QString metadataTableName;

    //simulate new entry in collection list
    QSqlQuery query(m_databaseManager->getDatabase());
    query.exec("INSERT INTO \"collections\" (\"name\") VALUES (\"Test\")");

    //create tables and metadata
    int cid = m_metadataEngine->createNewCollection();

    //get id
    query.exec("SELECT _id FROM collections WHERE name='Test'");
    if (query.next()) {
        id = query.value(0).toInt();
    }
    QVERIFY(id > 0);
    QVERIFY(id == cid);

    //get table names
    query.exec("SELECT table_name FROM collections WHERE name='Test'");
    if (query.next()) {
        tableName = query.value(0).toString();
        metadataTableName = tableName + "_metadata";
    }
    QVERIFY(!tableName.isEmpty());

    //check if tables exist
    query.exec(QString("SELECT name FROM sqlite_master WHERE type='table'"
                       " AND name='%1'").arg(tableName));
    bool a = query.next();
    QVERIFY(a);
    QVERIFY(!query.value(0).toString().isEmpty());

    query.exec(QString("SELECT name FROM sqlite_master WHERE type='table'"
                       " AND name='%1'").arg(metadataTableName));
    bool b = query.next();
    QVERIFY(b);
    QVERIFY(!query.value(0).toString().isEmpty());
}

void MetadataEngineTest::testDeleteCollection()
{
    int id = -1;
    QString tableName;
    QString metadataTableName;
    QSqlQuery query(m_databaseManager->getDatabase());

    //get id
    query.exec("SELECT _id FROM collections WHERE name='Test'");
    if (query.next()) {
        id = query.value(0).toInt();
    }
    QVERIFY(id > 0);
    query.clear(); //release query otherwise delete will fail

    //create tables and metadata
    m_metadataEngine->deleteCollection(id);

    //get table names
    query.exec("SELECT table_name FROM collections WHERE name='Test'");
    if (query.next()) {
        tableName = query.value(0).toString();
        metadataTableName = tableName + "_metadata";
    }
    QVERIFY(!tableName.isEmpty());

    //check if tables exist
    query.exec(QString("SELECT name FROM sqlite_master WHERE type='table'"
                       " AND name='%1'").arg(tableName));
    QVERIFY(!query.next());

    query.exec(QString("SELECT name FROM sqlite_master WHERE type='table'"
                       " AND name='%1'").arg(metadataTableName));
    QVERIFY(!query.next());

    //remove collection from collections list
    query.exec(QString("DELETE FROM collections WHERE _id=%1").arg(id));
}

void MetadataEngineTest::testCreateField()
{
    QSqlQuery query(m_databaseManager->getDatabase());
    QString fieldName("Test Field");
    MetadataEngine::FieldType type = MetadataEngine::NumericType;
    QString displayProperties = "prop:1;";
    QString editProperties = "prop:1;";
    QString triggerProperties = "prop:1;";

    int fieldId = m_metadataEngine->createField(fieldName,
                                                type,
                                                displayProperties,
                                                editProperties,
                                                triggerProperties);

    QVERIFY(fieldId != 0); //0 should be _id column
    QVERIFY(fieldId == (m_metadataEngine->getFieldCount() - 1));
    QVERIFY(fieldName == m_metadataEngine->getFieldName(fieldId));
    QVERIFY(type == m_metadataEngine->getFieldType(fieldId));
    QVERIFY(displayProperties ==
            m_metadataEngine->getFieldProperties(MetadataEngine::DisplayProperty,
                                                 fieldId));
    QVERIFY(editProperties ==
            m_metadataEngine->getFieldProperties(MetadataEngine::EditProperty,
                                                 fieldId));
    QVERIFY(triggerProperties ==
            m_metadataEngine->getFieldProperties(MetadataEngine::TriggerProperty,
                                                 fieldId));
}

void MetadataEngineTest::testDeleteField()
{
    QString fieldName2 = "Test2";
    MetadataEngine::FieldType type2 = MetadataEngine::NumericType;
    int fieldId2 = m_metadataEngine->createField(fieldName2,
                                                 type2,
                                                 "",
                                                 "",
                                                 "");

    QString fieldName("Test Field");
    int fieldId = 4; //hard coded I know :P
    int fieldCount = m_metadataEngine->getFieldCount();

    //delete Test Field
    m_metadataEngine->deleteField(fieldId);

    QVERIFY(m_metadataEngine->getFieldCount() == (fieldCount - 1));
    QVERIFY(fieldName2 == m_metadataEngine->getFieldName(fieldId));
    QVERIFY(m_metadataEngine->getFieldProperties(MetadataEngine::DisplayProperty,
                                                 fieldId).isEmpty());
    QVERIFY(m_metadataEngine->getFieldProperties(MetadataEngine::EditProperty,
                                                 fieldId).isEmpty());
    QVERIFY(m_metadataEngine->getFieldProperties(MetadataEngine::TriggerProperty,
                                                 fieldId).isEmpty());

    //delete Test2
    m_metadataEngine->deleteField(fieldId);

    QVERIFY(m_metadataEngine->getFieldCount() == (fieldCount - 2));
    QVERIFY("_invalid_column_name_" == m_metadataEngine->getFieldName(fieldId));
    QVERIFY(m_metadataEngine->getFieldProperties(MetadataEngine::DisplayProperty,
                                                 fieldId).isEmpty());
    QVERIFY(m_metadataEngine->getFieldProperties(MetadataEngine::EditProperty,
                                                 fieldId).isEmpty());
    QVERIFY(m_metadataEngine->getFieldProperties(MetadataEngine::TriggerProperty,
                                                 fieldId).isEmpty());
}

void MetadataEngineTest::testModifyField()
{
    QString fieldName("Test Field");
    QString fieldName2("Test Mod Field");
    MetadataEngine::FieldType type = MetadataEngine::NumericType;
    QString displayProperties = "prop:1;";
    QString editProperties = "prop:1;";
    QString triggerProperties = "prop:1;";
    QString displayProperties2 = "prop:2;";
    QString editProperties2 = "prop:2;";
    QString triggerProperties2 = "prop:2;";

    int fieldId = m_metadataEngine->createField(fieldName,
                                                type,
                                                displayProperties,
                                                editProperties,
                                                triggerProperties);

    //modify
    m_metadataEngine->modifyField(fieldId, fieldName2,
                                  displayProperties2,
                                  editProperties2,
                                  triggerProperties2);

    //verify
    QVERIFY(fieldName2 == m_metadataEngine->getFieldName(fieldId));
    QVERIFY(displayProperties2 ==
            m_metadataEngine->getFieldProperties(MetadataEngine::DisplayProperty,
                                                 fieldId));
    QVERIFY(editProperties2 ==
            m_metadataEngine->getFieldProperties(MetadataEngine::EditProperty,
                                                 fieldId));
    QVERIFY(triggerProperties2 ==
            m_metadataEngine->getFieldProperties(MetadataEngine::TriggerProperty,
                                                 fieldId));

    //delete
    m_metadataEngine->deleteField(fieldId);
}

void MetadataEngineTest::testFileMetadata()
{
    QString fileName = "testFileName.txt";
    QString hashName = "md5HashName.txt";

    //add
    int id = m_metadataEngine->addContentFile(fileName, hashName);
    QVERIFY(id != 0);

    //get id
    QVERIFY(id == m_metadataEngine->getContentFileId(hashName));

    //get
    QString qFileName;
    QString qHashName;
    QDateTime qFileAddedDate;
    bool s = m_metadataEngine->getContentFile(id, qFileName,
                                              qHashName, qFileAddedDate);
    QVERIFY(s);
    QVERIFY(qFileName == fileName);
    QVERIFY(qHashName == hashName);
    QVERIFY(qFileAddedDate.isValid());

    //update
    QString a = "New name.txt";
    QString b = "new_hash_md5.txt";
    QDateTime c = QDateTime(QDate(1990,6,28));
    m_metadataEngine->updateContentFile(id, a, b, c);
    s = m_metadataEngine->getContentFile(id, qFileName,
                                              qHashName, qFileAddedDate);
    QVERIFY(s);
    QVERIFY(qFileName == a);
    QVERIFY(qHashName == b);
    QVERIFY(qFileAddedDate == c);

    //remove
    m_metadataEngine->removeContentFile(id);
    s = m_metadataEngine->getContentFile(id, qFileName,
                                         qHashName, qFileAddedDate);
    QVERIFY(!s);

    //get all files
    int x = m_metadataEngine->addContentFile(fileName, hashName);
    int y = m_metadataEngine->addContentFile(a, b);
    QHash<int,QString> map = m_metadataEngine->getAllContentFiles();
    m_metadataEngine->removeContentFile(x);
    m_metadataEngine->removeContentFile(y);
    QVERIFY(map.size() == 2);
    QVERIFY(map.value(x) == hashName);
    QVERIFY(map.value(y) == b);
}

QTEST_APPLESS_MAIN(MetadataEngineTest)

#include "tst_metadataenginetest.moc"
