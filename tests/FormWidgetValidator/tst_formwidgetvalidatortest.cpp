#include <QtCore/QString>
#include <QtTest/QtTest>

#include "../../utils/formwidgetvalidator.h"

class FormWidgetValidatorTest : public QObject
{
    Q_OBJECT
    
public:
    FormWidgetValidatorTest();
    
private Q_SLOTS:
    void testNumericType();
};

FormWidgetValidatorTest::FormWidgetValidatorTest()
{
}

void FormWidgetValidatorTest::testNumericType()
{
    QString metadata1 = "noEmpty:1;";
    QString metadata2 = "noEmpty:0;";
    QString metadata3 = "noEmpty:lol;";
    QString metadata4 = "asd:lol;";
    QString metadata5 = "";
    QString input1 = "99";
    QString input2 = "";
    QString error;
    bool v;

    FormWidgetValidator v1(metadata1, MetadataEngine::NumericType);
    v = v1.validate(input1, error);
    QVERIFY(v);

    v = v1.validate(input2, error);
    qDebug() << error;
    QVERIFY(!v);


    FormWidgetValidator v2(metadata2, MetadataEngine::NumericType);
    v = v2.validate(input1, error);
    QVERIFY(v);

    v = v2.validate(input2, error);
    QVERIFY(v);


    FormWidgetValidator v3(metadata3, MetadataEngine::NumericType);
    v = v3.validate(input1, error);
    QVERIFY(v);

    v = v3.validate(input2, error);
    QVERIFY(v);


    FormWidgetValidator v4(metadata4, MetadataEngine::NumericType);
    v = v4.validate(input1, error);
    QVERIFY(v);

    v = v4.validate(input2, error);
    QVERIFY(v);


    FormWidgetValidator v5(metadata2, MetadataEngine::NumericType);
    v = v5.validate(input1, error);
    QVERIFY(v);

    v = v5.validate(input2, error);
    QVERIFY(v);
}

QTEST_APPLESS_MAIN(FormWidgetValidatorTest)

#include "tst_formwidgetvalidatortest.moc"
