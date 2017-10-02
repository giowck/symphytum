/*
 *  Copyright (c) 2012 Giorgio Wicklein <giowckln@gmail.com>
 */

//-----------------------------------------------------------------------------
// Hearders
//-----------------------------------------------------------------------------

#include "collectionlistmodel.h"
#include "../components/databasemanager.h"
#include "../components/sync_framework/syncsession.h"

#include <QtSql/QSqlRecord>
#include <QtGui/QIcon>


//-----------------------------------------------------------------------------
// Public
//-----------------------------------------------------------------------------

CollectionListModel::CollectionListModel(QObject *parent) :
    QSqlTableModel(parent, DatabaseManager::getInstance().getDatabase())
{
    //save data to db immediately after change
    setEditStrategy(QSqlTableModel::OnFieldChange);

    setTable("collections");
    select();
}

QVariant CollectionListModel::data(const QModelIndex &idx, int role) const
{
    if (role == Qt::DecorationRole) {
        QIcon i(":/images/icons/collectionlistitem.png");
        return i.pixmap(16, 16);
    } else {
        return QSqlTableModel::data(idx, role);
    }
}

bool CollectionListModel::setData(const QModelIndex &index,
                                  const QVariant &value, int role)
{
    //avoid empty collection names and check read-only mode
    if ((!value.toString().trimmed().isEmpty()) && (!SyncSession::IS_READ_ONLY)) {
        //set local data changed
        SyncSession::LOCAL_DATA_CHANGED = true;

        return QSqlTableModel::setData(index, value, role);
    } else {
        return false;
    }
}

void CollectionListModel::addCollection()
{
    QSqlRecord r = record();
    r.setValue(1, tr("New Collection"));
    insertRecord(-1, r);

    //changes are not applied even with on field change strategy
    //so call manually submit
    submitAll();
}


//-----------------------------------------------------------------------------
// Private
//-----------------------------------------------------------------------------
