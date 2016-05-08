#include <QtCore/QString>
#include <QtTest/QtTest>
#include <QtCore/QList>

#include "../../components/formlayoutmatrix.h"
#include "../../utils/formviewlayoutstate.h"
#include "../../widgets/form_widgets/abstractformwidget.h"
#include "../../widgets/form_widgets/testformwidget.h"


class FormLayoutStateTest : public QObject
{
    Q_OBJECT
    
public:
    FormLayoutStateTest();
    
private Q_SLOTS:
    void testCase();
};

FormLayoutStateTest::FormLayoutStateTest()
{
}

void FormLayoutStateTest::testCase()
{
    FormLayoutMatrix a;
    FormLayoutMatrix *b;
    QList<AbstractFormWidget*> list;

    TestFormWidget f1(0);
    TestFormWidget f2(0);

    f1.setWidthUnits(2);
    list.append(&f1);
    list.append(&f2);

    a.setFormWidget(&f1, 0, 0);
    a.setFormWidget(&f2, 1, 0);

    FormViewLayoutState state(&a, list);
    b = state.toFormLayoutMatrix(list);

    qDebug() << a.toString();
    qDebug() << b->toString();

    QVERIFY(a == *b);
}

QTEST_MAIN(FormLayoutStateTest)

#include "tst_formlayoutstatetest.moc"
