/*
 *  Copyright (c) 2012 Giorgio Wicklein <giowckln@gmail.com>
 */

//-----------------------------------------------------------------------------
// Hearders
//-----------------------------------------------------------------------------

#include "collectionviewdelegate.h"
#include "../../widgets/mainwindow.h"
#include "../../components/undocommands.h"

#include <QtWidgets/QUndoStack>
#include <QtWidgets/QUndoCommand>
#include <QtWidgets/QLineEdit>


//-----------------------------------------------------------------------------
// Public
//-----------------------------------------------------------------------------

CollectionViewDelegate::CollectionViewDelegate(QObject *parent) :
    QStyledItemDelegate(parent)
{
}

void CollectionViewDelegate::setModelData(QWidget *editor,
                                          QAbstractItemModel *model,
                                          const QModelIndex &index) const
{
    QLineEdit *l = qobject_cast<QLineEdit*>(editor);
    if (l) {
        //create undo action
        QVariant oldData = index.data();
        QVariant newData = l->text();
        if ((oldData != newData) && (!l->text().isEmpty())) {
            QUndoStack *stack = MainWindow::getUndoStack();
            if (stack) {
                RenameCollectionCommand *cmd = new RenameCollectionCommand(
                            index.row(), model, oldData, newData);
                stack->push(cmd);
            }
        }
    }

    QStyledItemDelegate::setModelData(editor, model, index);
}
