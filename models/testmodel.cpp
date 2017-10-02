/*
 *  Copyright (c) 2012 Giorgio Wicklein <giowckln@gmail.com>
 */

//-----------------------------------------------------------------------------
// Hearders
//-----------------------------------------------------------------------------

#include "testmodel.h"


//-----------------------------------------------------------------------------
// Public
//-----------------------------------------------------------------------------

TestModel::TestModel(QObject *parent) :
    QStandardItemModel(parent)
{
    int rows = 4000;
    int columns = 6;

    setRowCount(rows);
    setColumnCount(columns);

    for (int column = 0; column < columns; ++column) {
        setHeaderData(column, Qt::Horizontal, QString("Column %1").arg(column));
    }

    for (int row = 0; row < rows; ++row) {
        for (int column = 0; column < 6; ++column) {
            QStandardItem *item = new QStandardItem(QString("row %0, column %1").arg(row).arg(column));
            setItem(row, column, item);
        }
    }
}


//-----------------------------------------------------------------------------
// Private
//-----------------------------------------------------------------------------
