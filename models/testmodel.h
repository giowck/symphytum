/**
  * \class TestModel
  * \brief This model is used to test views and to
  *        follow model calls. The data is stored only
  *        in memory, so it will be discarded.
  *        This class is based on QStandardItemModel
  *        where reimplemented methods serve debugging.
  * \author Giorgio Wicklein - GIOWISYS Software
  * \date 07/06/2012
  */

#ifndef TESTMODEL_H
#define TESTMODEL_H


//-----------------------------------------------------------------------------
// Headers
//-----------------------------------------------------------------------------

#include <QtGui/QStandardItemModel>


//-----------------------------------------------------------------------------
// TestModel
//-----------------------------------------------------------------------------

class TestModel : public QStandardItemModel
{
    Q_OBJECT

public:
    explicit TestModel(QObject *parent = 0);

private:

};

#endif // TESTMODEL_H
