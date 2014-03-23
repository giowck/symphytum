#include <QtCore/QString>
#include <QtTest/QtTest>

#include "../../utils/metadatapropertiesparser.h"

class MetadataPropertiesParserTest : public QObject
{
    Q_OBJECT
    
public:
    MetadataPropertiesParserTest();
    
private Q_SLOTS:
    void testGetValue();
};

MetadataPropertiesParserTest::MetadataPropertiesParserTest()
{
}

void MetadataPropertiesParserTest::testGetValue()
{
    QString metadataString =
            "keyOne:valueOne;keyTwo:valueTwo;keyMulti:value1,value2,value3";
    QString invalid= "sdgs-gd sj,h sdf.ds";
    QString validInvalidMix = "asd;lol:rofl;wtf;l33t:r0x:p0wn;;:;;:";

    MetadataPropertiesParser parser1(metadataString);
    QVERIFY(parser1.size() == 3);
    QVERIFY(parser1.getValue("invalidKey") == QString(""));
    QVERIFY(parser1.getValue("keyOne") == "valueOne");
    QVERIFY(parser1.getValue("keyTwo") == "valueTwo");
    QVERIFY(parser1.getValue("keyMulti") == "value1,value2,value3");

    MetadataPropertiesParser parser2(invalid);
    QVERIFY(parser2.size() == 0);

    MetadataPropertiesParser parser3(validInvalidMix);
    QVERIFY(parser3.size() == 2);
    QVERIFY(parser3.getValue("asd") == QString(""));
    QVERIFY(parser3.getValue("lol") == QString("rofl"));
    QVERIFY(parser3.getValue("l33t") == QString(""));
}

QTEST_APPLESS_MAIN(MetadataPropertiesParserTest)

#include "tst_metadatapropertiesparsertest.moc"
