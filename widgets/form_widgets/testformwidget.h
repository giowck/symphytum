/**
  * \class TestFormWidget
  * \brief This is a form widget for testing purpose.
  * \author Giorgio Wicklein - GIOWISYS Software
  * \date 11/04/2012
  */

#ifndef TESTFORMWIDGET_H
#define TESTFORMWIDGET_H


//-----------------------------------------------------------------------------
// Headers
//-----------------------------------------------------------------------------

#include "abstractformwidget.h"


//-----------------------------------------------------------------------------
// Forward declarations
//-----------------------------------------------------------------------------

class QLabel;
class QVBoxLayout;


//-----------------------------------------------------------------------------
// TestFormWidget
//-----------------------------------------------------------------------------

class TestFormWidget : public AbstractFormWidget
{
    Q_OBJECT

public:
    explicit TestFormWidget(QWidget *parent = nullptr);

    void setFieldName(const QString &name);
    QString getFieldName() const;
    void clearData();
    void setData(const QVariant &data);
    QVariant getData() const;

private:
    QLabel *testLabel;
    QVBoxLayout *mainLayout;
    QString data;
};

#endif // TESTFORMWIDGET_H
