#include <QtCore/QString>
#include <QtTest/QtTest>

#include "../../components/formlayoutmatrix.h"
#include "../../widgets/form_widgets/testformwidget.h"

class FormLayoutMatrixTest : public QObject
{
    Q_OBJECT
    
public:
    FormLayoutMatrixTest();
    
private Q_SLOTS:
    void testConstructor();
    void testOutOfRangeAccess1();
    void testOutOfRangeAccess2();
    void testSetAndGet1();
    void testSetAndGet2();
    void testSetAndGet3();
    void testSetAndGet4();
    void testSetAndGet5();
    void testSetAndGet6();
    void testSetAndGet7();
    void testSetAndGet8();
    void testGetByExt1();
    void testGetByExt2();
    void testAddWidget1();
    void testAddWidget2();
    void testAddWidget3();
    void testRemoveWidget1();
    void testRemoveWidget2();
    void testFindIndex();
    void testSimplifyMatrix1();
    void testSimplifyMatrix2();
    void testFormWidgetMovement1();
    void testFormWidgetMovement2();
    void testFormWidgetMovement3();
    void testFormWidgetMovement4();
    void testFormWidgetMovement5();
    void testFormWidgetMovement6();
    void testFormWidgetMovement7();
    void testFormWidgetMovement8();
    void testFormWidgetResize1();
    void testFormWidgetResize2();
    void testFormWidgetResize3();
    void testCopyConstructor();
    void testComparisonOperator();
};

FormLayoutMatrixTest::FormLayoutMatrixTest()
{
}

void FormLayoutMatrixTest::testConstructor()
{
    FormLayoutMatrix f;

    QVERIFY(f.columnCount() == 0);
    QVERIFY(f.rowCount() == 0);
}

void FormLayoutMatrixTest::testOutOfRangeAccess1()
{
    FormLayoutMatrix f;

    FormWidget* x = f.getFormWidget(10, 10);
    QVERIFY(x == NULL);
}

void FormLayoutMatrixTest::testOutOfRangeAccess2()
{
    FormLayoutMatrix f;

    TestFormWidget a(0);
    f.setFormWidget(&a, 0, 0);

    FormWidget* x = f.getFormWidget(0, 10);
    QVERIFY(x == NULL);
}

void FormLayoutMatrixTest::testSetAndGet1()
{
    //test simple set at (0,0)
    FormLayoutMatrix f;

    TestFormWidget a(0);
    f.setFormWidget(&a, 0, 0);
    QVERIFY(f.columnCount() == 1);
    QVERIFY(f.rowCount() == 1);
    QVERIFY(*(f.getFormWidget(0, 0)) == a);
    QVERIFY(f.getFormWidget(0, 0) == &a);
    QVERIFY(f[0][0] == &a);
    qDebug() << "Matrix with only one item at (0,0)" << f.toString();
}

void FormLayoutMatrixTest::testSetAndGet2()
{
    //test simple set at (0,1)
    FormLayoutMatrix f;

    TestFormWidget a(0);
    f.setFormWidget(&a, 0, 1);
    QVERIFY(f.columnCount() == 2);
    QVERIFY(f.rowCount() == 1);
    QVERIFY(*(f.getFormWidget(0, 1)) == a);
    QVERIFY(f.getFormWidget(0, 1) == &a);
    QVERIFY(f[0][1] == &a);
    qDebug() << "Matrix with only one item at (0,1)" << f.toString();
}

void FormLayoutMatrixTest::testSetAndGet3()
{
    //test set at a new row that needs creation
    FormLayoutMatrix f;

    TestFormWidget a(0);
    f.setFormWidget(&a, 1, 0);
    QVERIFY(f.columnCount() == 1);
    QVERIFY(f.rowCount() == 2);
    QVERIFY(*(f.getFormWidget(1, 0)) == a);
    QVERIFY(f.getFormWidget(1, 0) == &a);
    QVERIFY(f[1][0] == &a);
    qDebug() << "Matrix with only one item at (1,0)" << f.toString();
}

void FormLayoutMatrixTest::testSetAndGet4()
{
    //test set at a new row and column that needs creation
    FormLayoutMatrix f;

    TestFormWidget a(0);
    f.setFormWidget(&a, 5, 5);
    QVERIFY(f.columnCount() == 6);
    QVERIFY(f.rowCount() == 6);
    QVERIFY(*(f.getFormWidget(5, 5)) == a);
    QVERIFY(f.getFormWidget(5, 5) == &a);
    QVERIFY(f[5][5] == &a);
    qDebug() << "Matrix with only one item at (5,5)" << f.toString();
}

void FormLayoutMatrixTest::testSetAndGet5()
{
    //test set with more widgets
    FormLayoutMatrix f;

    TestFormWidget a(0);
    TestFormWidget b(0);
    TestFormWidget c(0);

    f.setFormWidget(&a, 0, 0);
    f.setFormWidget(&b, 1, 0);
    f.setFormWidget(&c, 3, 5);

    QVERIFY(f.columnCount() == 6);
    QVERIFY(f.rowCount() == 4);
    QVERIFY(f[0][0] == &a);
    QVERIFY(f[1][0] == &b);
    QVERIFY(f[3][5] == &c);
    qDebug() << "Matrix with 3 items at (0,0) (1,0) (3,5)" << f.toString();
}

void FormLayoutMatrixTest::testSetAndGet6()
{
    //test set with a widget with units bigger 1
    FormLayoutMatrix f;

    TestFormWidget a(0);
    a.setWidthUnits(2);
    a.setHeightUnits(3);
    f.setFormWidget(&a, 3, 3);

    QVERIFY(f.columnCount() == 5);
    QVERIFY(f.rowCount() == 6);
    QVERIFY(f[3][3] == &a);
    QVERIFY(f[3][4] == ((FormWidget*)FormLayoutMatrix::EXTENDED_FORM_WIDGET));
    QVERIFY(f[4][3] == ((FormWidget*)FormLayoutMatrix::EXTENDED_FORM_WIDGET));
    QVERIFY(f[4][4] == ((FormWidget*)FormLayoutMatrix::EXTENDED_FORM_WIDGET));
    QVERIFY(f[5][3] == ((FormWidget*)FormLayoutMatrix::EXTENDED_FORM_WIDGET));
    QVERIFY(f[5][4] == ((FormWidget*)FormLayoutMatrix::EXTENDED_FORM_WIDGET));
    qDebug() << "Matrix with 1 item(2x3) at (3,3)" << f.toString();
}

void FormLayoutMatrixTest::testSetAndGet7()
{
    //test set with more widgets that have units bigger 1
    FormLayoutMatrix f;

    TestFormWidget a(0);
    a.setWidthUnits(2);
    a.setHeightUnits(3);
    f.setFormWidget(&a, 3, 3);

    TestFormWidget b(0);
    b.setWidthUnits(2);
    b.setHeightUnits(1);
    f.setFormWidget(&b, 0, 0);

    TestFormWidget c(0);
    c.setWidthUnits(1);
    c.setHeightUnits(2);
    f.setFormWidget(&c, 0, 2);

    QVERIFY(f.columnCount() == 5);
    QVERIFY(f.rowCount() == 6);

    QVERIFY(f[3][3] == &a);
    QVERIFY(f[3][4] == ((FormWidget*)FormLayoutMatrix::EXTENDED_FORM_WIDGET));
    QVERIFY(f[4][3] == ((FormWidget*)FormLayoutMatrix::EXTENDED_FORM_WIDGET));
    QVERIFY(f[4][4] == ((FormWidget*)FormLayoutMatrix::EXTENDED_FORM_WIDGET));
    QVERIFY(f[5][3] == ((FormWidget*)FormLayoutMatrix::EXTENDED_FORM_WIDGET));
    QVERIFY(f[5][4] == ((FormWidget*)FormLayoutMatrix::EXTENDED_FORM_WIDGET));

    QVERIFY(f[0][0] == &b);
    QVERIFY(f[0][1] == ((FormWidget*)FormLayoutMatrix::EXTENDED_FORM_WIDGET));

    QVERIFY(f[0][2] == &c);
    QVERIFY(f[1][2] == ((FormWidget*)FormLayoutMatrix::EXTENDED_FORM_WIDGET));

    qDebug() << "Matrix with item(2x3)(3,3) item(2x1)(0,0) item(1x2)(0,2)" << f.toString();
}

void FormLayoutMatrixTest::testSetAndGet8()
{
    //test set with a widget that has width > 1
    //where only 1 unit with is free and a new column
    //needs to be created
    FormLayoutMatrix f;

    TestFormWidget a(0);
    TestFormWidget b(0);
    b.setWidthUnits(2);
    f.setFormWidget(&a, 3, 3);
    f.setFormWidget(&b, 2, 3);

    QVERIFY(f.columnCount() == 5);
    QVERIFY(f.rowCount() == 4);
    QVERIFY(f[3][3] == &a);
    QVERIFY(f[2][3] == &b);
    QVERIFY(f[2][4] == ((FormWidget*)FormLayoutMatrix::EXTENDED_FORM_WIDGET));
    qDebug() << "Matrix with item(1x1)(3,3) item(2x1)(2,3)" << f.toString();
}

void FormLayoutMatrixTest::testGetByExt1()
{
    FormLayoutMatrix f;
    FormWidget *fw;

    TestFormWidget a(0);
    a.setWidthUnits(2);
    a.setHeightUnits(3);
    f.setFormWidget(&a, 3, 3);

    fw = f.getFormWidgetByExtended(3, 3);
    QVERIFY(fw == NULL); //FAILS

    fw = f.getFormWidgetByExtended(3, 4);
    QVERIFY(fw == &a);

    fw = f.getFormWidgetByExtended(4, 3);
    QVERIFY(fw == &a);

    fw = f.getFormWidgetByExtended(4, 4);
    QVERIFY(fw == &a);

    fw = f.getFormWidgetByExtended(5, 3);
    QVERIFY(fw == &a);

    fw = f.getFormWidgetByExtended(5, 4);
    QVERIFY(fw == &a);

    qDebug() << "Matrix with 1 item(2x3) at (3,3)" << f.toString();
}

void FormLayoutMatrixTest::testGetByExt2()
{
    //test set with more widgets that have units bigger 1
    FormLayoutMatrix f;
    FormWidget *fw;

    TestFormWidget a(0);
    a.setWidthUnits(2);
    a.setHeightUnits(3);
    f.setFormWidget(&a, 3, 3);

    TestFormWidget b(0);
    b.setWidthUnits(2);
    b.setHeightUnits(1);
    f.setFormWidget(&b, 0, 0);

    TestFormWidget c(0);
    c.setWidthUnits(1);
    c.setHeightUnits(2);
    f.setFormWidget(&c, 0, 2);

    fw = f.getFormWidgetByExtended(3, 4);
    QVERIFY(fw == &a);

    fw = f.getFormWidgetByExtended(0, 1);
    QVERIFY(fw == &b);

    fw = f.getFormWidgetByExtended(1, 2);
    QVERIFY(fw == &c);
}

void FormLayoutMatrixTest::testAddWidget1()
{
    FormLayoutMatrix f;

    TestFormWidget a(0);

    f.addFormWidget(&a);

    QVERIFY(f.rowCount() == 1);
    QVERIFY(f.columnCount() == 1);
    QVERIFY(f[0][0] == &a);
    qDebug() << "Matrix with only one item at (0,0)" << f.toString();
}

void FormLayoutMatrixTest::testAddWidget2()
{
    FormLayoutMatrix f;

    TestFormWidget a(0);
    TestFormWidget b(0);
    TestFormWidget c(0);

    f.addFormWidget(&a);
    f.addFormWidget(&b);
    f.addFormWidget(&c);

    QVERIFY(f.rowCount() == 3);
    QVERIFY(f.columnCount() == 1);
    QVERIFY(f[0][0] == &a);
    QVERIFY(f[1][0] == &b);
    QVERIFY(f[2][0] == &c);
    qDebug() << "Matrix with item(1x1)(0,0) item(1x1)(1,0) item(1x1)(2,0)" << f.toString();
}

void FormLayoutMatrixTest::testAddWidget3()
{
    //test add with units bigger 1
    FormLayoutMatrix f;

    TestFormWidget a(0);
    TestFormWidget b(0);
    TestFormWidget c(0);

    a.setWidthUnits(2);
    a.setHeightUnits(2);
    b.setWidthUnits(3);
    b.setHeightUnits(1);
    c.setWidthUnits(1);
    c.setHeightUnits(1);

    f.addFormWidget(&a);
    f.addFormWidget(&b);
    f.addFormWidget(&c);

    QVERIFY(f.rowCount() == 3);
    QVERIFY(f.columnCount() == 3);
    QVERIFY(f[0][0] == &a);
    QVERIFY(f[2][0] == &b);
    QVERIFY(f[0][2] == &c);
    qDebug() << "Matrix with item(2x2)(0,0) item(3x1)(2,0) item(1x1)(0,2)" << f.toString();
}

void FormLayoutMatrixTest::testRemoveWidget1()
{
    FormLayoutMatrix f;

    TestFormWidget a(0);
    f.setFormWidget(&a, 2, 2);

    qDebug() << "Matrix before remove with only one item at (2,2)" << f.toString();
    f.removeFormWidget(2, 2);

    QVERIFY(f.rowCount() == 3);
    QVERIFY(f.columnCount() == 3);
    QVERIFY(f[2][2] == ((FormWidget*)FormLayoutMatrix::NO_FORM_WIDGET));
    qDebug() << "Matrix after remove with no item at (2,2)" << f.toString();
}

void FormLayoutMatrixTest::testRemoveWidget2()
{
    //test add with units bigger 1
    FormLayoutMatrix f;

    TestFormWidget a(0);
    TestFormWidget b(0);
    TestFormWidget c(0);

    a.setWidthUnits(2);
    a.setHeightUnits(2);
    b.setWidthUnits(3);
    b.setHeightUnits(1);
    c.setWidthUnits(1);
    c.setHeightUnits(1);

    f.addFormWidget(&a);
    f.addFormWidget(&b);
    f.addFormWidget(&c);

    qDebug() << "Matrix before remove with item(2x2)(0,0) item(3x1)(2,0) item(1x1)(0,2)" << f.toString();

    f.removeFormWidget(0, 0);

    QVERIFY(f.rowCount() == 3);
    QVERIFY(f.columnCount() == 3);
    QVERIFY(f[0][0] == ((FormWidget*)FormLayoutMatrix::NO_FORM_WIDGET));
    QVERIFY(f[0][1] == ((FormWidget*)FormLayoutMatrix::NO_FORM_WIDGET));
    QVERIFY(f[1][0] == ((FormWidget*)FormLayoutMatrix::NO_FORM_WIDGET));
    QVERIFY(f[1][1] == ((FormWidget*)FormLayoutMatrix::NO_FORM_WIDGET));
    QVERIFY(f[2][0] == &b);
    QVERIFY(f[0][2] == &c);
    qDebug() << "Matrix after remove with item(3x1)(2,0) item(1x1)(0,2)" << f.toString();
}

void FormLayoutMatrixTest::testFindIndex()
{
    FormLayoutMatrix f;

    TestFormWidget a(0);
    TestFormWidget b(0);
    TestFormWidget c(0);

    a.setWidthUnits(2);
    a.setHeightUnits(2);
    b.setWidthUnits(3);
    b.setHeightUnits(1);
    c.setWidthUnits(1);
    c.setHeightUnits(1);

    f.addFormWidget(&a);
    f.addFormWidget(&b);
    f.addFormWidget(&c);

    int row, column;
    f.findFormWidgetIndex(&c, row, column);
    QVERIFY(row == 0);
    QVERIFY(column == 2);

    QVERIFY(!f.findFormWidgetIndex(NULL, row, column));
}

void FormLayoutMatrixTest::testSimplifyMatrix1()
{
    //simple test with many empty rows columns before a FW
    FormLayoutMatrix f;

    TestFormWidget a(0);
    f.setFormWidget(&a, 5, 5);

    qDebug() << "Matrix before simplify()" << f.toString();
    f.simplifyMatrix();

    QVERIFY(f.columnCount() == 1);
    QVERIFY(f.rowCount() == 1);
    QVERIFY(f[0][0] == &a);
    qDebug() << "Matrix after simplify()" << f.toString();
}

void FormLayoutMatrixTest::testSimplifyMatrix2()
{
    //complex matrix with FWs and empty rows/columns between
    FormLayoutMatrix f;

    TestFormWidget a(0);
    TestFormWidget b(0);
    TestFormWidget c(0);

    a.setWidthUnits(2);
    a.setHeightUnits(1);
    b.setWidthUnits(1);
    b.setHeightUnits(2);
    c.setWidthUnits(3);
    c.setHeightUnits(3);

    f.setFormWidget(&a, 8, 8);
    f.setFormWidget(&b, 0, 0);
    f.setFormWidget(&c, 5, 5);

    qDebug() << "Matrix before simplify()" << f.toString();
    f.simplifyMatrix();

    QVERIFY(f.columnCount() == 6);
    QVERIFY(f.rowCount() == 6);
    QVERIFY(f[0][0] == &b);
    QVERIFY(f[5][4] == &a);
    QVERIFY(f[2][1] == &c);
    qDebug() << "Matrix after simplify()" << f.toString();
}

void FormLayoutMatrixTest::testFormWidgetMovement1()
{
    //test 1x1 widget moves to an empty cell
    FormLayoutMatrix f;

    TestFormWidget a(0);
    TestFormWidget b(0);
    TestFormWidget c(0);

    f.setFormWidget(&a, 0, 0);
    f.setFormWidget(&b, 0, 1);
    f.setFormWidget(&c, 1, 0);

    qDebug() << "Matrix before movement" << f.toString();
    f.formWidgetMovement(&a, 1, 1);
    qDebug() << "Matrix after movement" << f.toString();

    QVERIFY(f.columnCount() == 2);
    QVERIFY(f.rowCount() == 2);
    QVERIFY(f[0][0] == (FormWidget*)FormLayoutMatrix::NO_FORM_WIDGET);
    QVERIFY(f[0][1] == &b);
    QVERIFY(f[1][0] == &c);
    QVERIFY(f[1][1] == &a);
}

void FormLayoutMatrixTest::testFormWidgetMovement2()
{
    //test 1x1 widget moves to a cell which contains a 1x1 widget
    FormLayoutMatrix f;

    TestFormWidget a(0);
    TestFormWidget b(0);
    TestFormWidget c(0);
    TestFormWidget d(0);

    f.setFormWidget(&a, 0, 0);
    f.setFormWidget(&b, 0, 1);
    f.setFormWidget(&c, 1, 0);
    f.setFormWidget(&d, 1, 1);

    f.formWidgetMovement(&a, 1, 1);

    QVERIFY(f.columnCount() == 2);
    QVERIFY(f.rowCount() == 2);
    QVERIFY(f[0][0] == &d);
    QVERIFY(f[0][1] == &b);
    QVERIFY(f[1][0] == &c);
    QVERIFY(f[1][1] == &a);
}

void FormLayoutMatrixTest::testFormWidgetMovement3()
{
    //test 1x1 widget moves to a cell which contains a 2x1 widget
    FormLayoutMatrix f;

    TestFormWidget a(0);
    TestFormWidget b(0);
    TestFormWidget c(0);

    c.setWidthUnits(2);

    f.setFormWidget(&a, 0, 0);
    f.setFormWidget(&b, 0, 1);
    f.setFormWidget(&c, 1, 0);

    qDebug() << "Matrix before movement" << f.toString();
    f.formWidgetMovement(&a, 1, 0);
    qDebug() << "Matrix after movement" << f.toString();

    QVERIFY(f.columnCount() == 2);
    QVERIFY(f.rowCount() == 3);
    QVERIFY(f[0][0] == (FormWidget*)FormLayoutMatrix::NO_FORM_WIDGET);
    QVERIFY(f[1][1] == (FormWidget*)FormLayoutMatrix::NO_FORM_WIDGET);
    QVERIFY(f[0][1] == &b);
    QVERIFY(f[1][0] == &a);
    QVERIFY(f[2][0] == &c);
    QVERIFY(f[2][1] == (FormWidget*)FormLayoutMatrix::EXTENDED_FORM_WIDGET);
}

void FormLayoutMatrixTest::testFormWidgetMovement4()
{
    //test 1x1 widget moves to a cell which contains a 1x2 widget
    FormLayoutMatrix f;

    TestFormWidget a(0);
    TestFormWidget b(0);
    TestFormWidget c(0);

    c.setHeightUnits(2);

    f.setFormWidget(&a, 0, 0);
    f.setFormWidget(&b, 1, 0);
    f.setFormWidget(&c, 0, 1);

    qDebug() << "Matrix before movement" << f.toString();
    f.formWidgetMovement(&a, 0, 1);
    qDebug() << "Matrix after movement" << f.toString();

    QVERIFY(f.columnCount() == 2);
    QVERIFY(f.rowCount() == 4);
    QVERIFY(f[0][0] == (FormWidget*)FormLayoutMatrix::NO_FORM_WIDGET);
    QVERIFY(f[1][1] == (FormWidget*)FormLayoutMatrix::NO_FORM_WIDGET);
    QVERIFY(f[2][1] == (FormWidget*)FormLayoutMatrix::NO_FORM_WIDGET);
    QVERIFY(f[3][1] == (FormWidget*)FormLayoutMatrix::NO_FORM_WIDGET);
    QVERIFY(f[0][1] == &a);
    QVERIFY(f[1][0] == &b);
    QVERIFY(f[2][0] == &c);
    QVERIFY(f[3][0] == (FormWidget*)FormLayoutMatrix::EXTENDED_FORM_WIDGET);

}

void FormLayoutMatrixTest::testFormWidgetMovement5()
{
    //test 2x1 widget moves to a cell which contains a 1x1 widget
    FormLayoutMatrix f;

    TestFormWidget a(0);
    TestFormWidget b(0);
    TestFormWidget c(0);

    c.setWidthUnits(2);

    f.setFormWidget(&a, 1, 0);
    f.setFormWidget(&b, 1, 1);
    f.setFormWidget(&c, 0, 0);

    qDebug() << "Matrix before movement" << f.toString();
    f.formWidgetMovement(&c, 1, 0);
    qDebug() << "Matrix after movement" << f.toString();

    QVERIFY(f.columnCount() == 2);
    QVERIFY(f.rowCount() == 2);
    QVERIFY(f[0][0] == &a);
    QVERIFY(f[0][1] == &b);
    QVERIFY(f[1][0] == &c);
    QVERIFY(f[1][1] == (FormWidget*)FormLayoutMatrix::EXTENDED_FORM_WIDGET);
}

void FormLayoutMatrixTest::testFormWidgetMovement6()
{
    //test 2x1 widget moves to a cell which contains a 1x1 widget
    //and where 1 column needs to be created by setFormWidget
    FormLayoutMatrix f;

    TestFormWidget a(0);
    TestFormWidget b(0);
    TestFormWidget c(0);

    c.setWidthUnits(2);

    f.setFormWidget(&a, 1, 0);
    f.setFormWidget(&b, 1, 1);
    f.setFormWidget(&c, 0, 0);

    qDebug() << "Matrix before movement" << f.toString();
    f.formWidgetMovement(&c, 1, 1);
    qDebug() << "Matrix after movement" << f.toString();

    QVERIFY(f.columnCount() == 3);
    QVERIFY(f.rowCount() == 2);
    QVERIFY(f[0][0] == &b);
    QVERIFY(f[0][1] == (FormWidget*)FormLayoutMatrix::NO_FORM_WIDGET);
    QVERIFY(f[0][2] == (FormWidget*)FormLayoutMatrix::NO_FORM_WIDGET);
    QVERIFY(f[1][0] == &a);
    QVERIFY(f[1][1] == &c);
    QVERIFY(f[1][2] == (FormWidget*)FormLayoutMatrix::EXTENDED_FORM_WIDGET);
}

void FormLayoutMatrixTest::testFormWidgetMovement7()
{
    //test 1x2 widget moves to a cell which contains a 2x1 widget
    FormLayoutMatrix f;

    TestFormWidget a(0);
    TestFormWidget b(0);
    TestFormWidget c(0);
    TestFormWidget d(0);

    c.setHeightUnits(2);
    d.setWidthUnits(2);

    f.setFormWidget(&a, 1, 1);
    f.setFormWidget(&b, 1, 2);
    f.setFormWidget(&c, 0, 0);
    f.setFormWidget(&d, 0, 1);

    qDebug() << "Matrix before movement" << f.toString();
    f.formWidgetMovement(&c, 0, 1);
    qDebug() << "Matrix after movement" << f.toString();

    QVERIFY(f.columnCount() == 3);
    QVERIFY(f.rowCount() == 3);
    QVERIFY(f[0][0] == &a);
    QVERIFY(f[0][1] == &c);
    QVERIFY(f[0][2] == (FormWidget*)FormLayoutMatrix::NO_FORM_WIDGET);
    QVERIFY(f[1][0] == (FormWidget*)FormLayoutMatrix::NO_FORM_WIDGET);
    QVERIFY(f[1][1] == (FormWidget*)FormLayoutMatrix::EXTENDED_FORM_WIDGET);
    QVERIFY(f[1][2] == &b);
    QVERIFY(f[2][0] == &d);
    QVERIFY(f[2][1] == (FormWidget*)FormLayoutMatrix::EXTENDED_FORM_WIDGET);
    QVERIFY(f[2][2] == (FormWidget*)FormLayoutMatrix::NO_FORM_WIDGET);
}

void FormLayoutMatrixTest::testFormWidgetMovement8()
{
    //test 2x1 widget moves to a cell which is beyond indey and needs to be created
    FormLayoutMatrix f;

    TestFormWidget a(0);
    TestFormWidget b(0);
    TestFormWidget c(0);

    a.setWidthUnits(2);

    f.setFormWidget(&a, 0, 0);
    f.setFormWidget(&b, 1, 0);
    f.setFormWidget(&c, 1, 1);

    qDebug() << "Matrix before movement" << f.toString();
    f.formWidgetMovement(&a, 0, 2);
    qDebug() << "Matrix after movement" << f.toString();

    QVERIFY(f.columnCount() == 4);
    QVERIFY(f.rowCount() == 2);
    QVERIFY(f[0][0] == (FormWidget*)FormLayoutMatrix::NO_FORM_WIDGET);
    QVERIFY(f[0][1] == (FormWidget*)FormLayoutMatrix::NO_FORM_WIDGET);
    QVERIFY(f[0][2] == &a);
    QVERIFY(f[0][3] == (FormWidget*)FormLayoutMatrix::EXTENDED_FORM_WIDGET);
    QVERIFY(f[1][0] == &b);
    QVERIFY(f[1][1] == &c);
    QVERIFY(f[1][2] == (FormWidget*)FormLayoutMatrix::NO_FORM_WIDGET);
    QVERIFY(f[1][3] == (FormWidget*)FormLayoutMatrix::NO_FORM_WIDGET);
}

void FormLayoutMatrixTest::testFormWidgetResize1()
{
    FormLayoutMatrix f;

    TestFormWidget a(0);

    f.setFormWidget(&a, 0, 0);

    qDebug() << "Matrix before resize" << f.toString();
    f.formWidgetResize(&a, 2, 1);
    qDebug() << "Matrix after resize" << f.toString();

    QVERIFY(f.columnCount() == 2);
    QVERIFY(f.rowCount() == 1);
    QVERIFY(f[0][0] == &a);
    QVERIFY(f[0][1] == (FormWidget*)FormLayoutMatrix::EXTENDED_FORM_WIDGET);
    QVERIFY(a.getWidthUnits() == 2);
    QVERIFY(a.getHeightUnits() == 1);
}

void FormLayoutMatrixTest::testFormWidgetResize2()
{
    FormLayoutMatrix f;

    TestFormWidget a(0);
    TestFormWidget b(0);
    TestFormWidget c(0);

    f.setFormWidget(&a, 0, 0);
    f.setFormWidget(&b, 0, 1);
    f.setFormWidget(&c, 0, 2);

    qDebug() << "Matrix before resize" << f.toString();
    f.formWidgetResize(&a, 3, 1);
    qDebug() << "Matrix after resize" << f.toString();

    QVERIFY(f.columnCount() == 3);
    QVERIFY(f.rowCount() == 2);
    QVERIFY(f[0][0] == &a);
    QVERIFY(f[0][1] == (FormWidget*)FormLayoutMatrix::EXTENDED_FORM_WIDGET);
    QVERIFY(f[0][2] == (FormWidget*)FormLayoutMatrix::EXTENDED_FORM_WIDGET);
    QVERIFY(f[1][0] == &b);
    QVERIFY(f[1][1] == &c);
    QVERIFY(f[1][2] == (FormWidget*)FormLayoutMatrix::NO_FORM_WIDGET);
    QVERIFY(a.getWidthUnits() == 3);
    QVERIFY(a.getHeightUnits() == 1);
}

void FormLayoutMatrixTest::testFormWidgetResize3()
{
    FormLayoutMatrix f;

    TestFormWidget a(0);
    TestFormWidget b(0);

    a.setHeightUnits(2);
    b.setWidthUnits(2);

    f.setFormWidget(&a, 0, 0);
    f.setFormWidget(&b, 0, 1);

    qDebug() << "Matrix before resize" << f.toString();
    f.formWidgetResize(&a, 2, 2);
    qDebug() << "Matrix after resize" << f.toString();

    QVERIFY(f.columnCount() == 2);
    QVERIFY(f.rowCount() == 3);
    QVERIFY(f[0][0] == &a);
    QVERIFY(f[0][1] == (FormWidget*)FormLayoutMatrix::EXTENDED_FORM_WIDGET);
    QVERIFY(f[1][0] == (FormWidget*)FormLayoutMatrix::EXTENDED_FORM_WIDGET);
    QVERIFY(f[1][1] == (FormWidget*)FormLayoutMatrix::EXTENDED_FORM_WIDGET);
    QVERIFY(f[2][0] == &b);
    QVERIFY(f[2][1] == (FormWidget*)FormLayoutMatrix::EXTENDED_FORM_WIDGET);
    QVERIFY(a.getWidthUnits() == 2);
    QVERIFY(a.getHeightUnits() == 2);
}

void FormLayoutMatrixTest::testCopyConstructor()
{
    TestFormWidget a(0);
    TestFormWidget b(0);

    FormLayoutMatrix f1;
    f1.setFormWidget(&a, 0, 0);
    f1.setFormWidget(&b, 0, 1);

    FormLayoutMatrix f2(f1);

    QVERIFY(f1.columnCount() == f2.columnCount());
    QVERIFY(f2.rowCount() == f2.rowCount());
    QVERIFY(f2[0][0] == &a);
    QVERIFY(f2[0][1] == &b);
    QVERIFY(f1.toString() == f2.toString());
}

void FormLayoutMatrixTest::testComparisonOperator()
{
    TestFormWidget a(0);
    TestFormWidget b(0);

    FormLayoutMatrix f1;
    f1.setFormWidget(&a, 0, 0);
    f1.setFormWidget(&b, 0, 1);

    FormLayoutMatrix f2(f1);
    QVERIFY(f1 == f2);
    QVERIFY(!(f1 != f2));

    FormLayoutMatrix f3;
    QVERIFY(f1 != f3);
    QVERIFY(!(f1 == f3));
}

QTEST_MAIN(FormLayoutMatrixTest)

#include "tst_formlayoutmatrixtest.moc"
