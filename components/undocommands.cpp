/*
 *  Copyright (c) 2012 Giorgio Wicklein <giowckln@gmail.com>
 */

//-----------------------------------------------------------------------------
// Hearders
//-----------------------------------------------------------------------------

#include "undocommands.h"
#include "../widgets/mainwindow.h"
#include "../models/standardmodel.h"
#include "../utils/formviewlayoutstate.h"
#include "../views/formview/formview.h"
#include "../utils/metadatapropertiesparser.h"
#include "alarmmanager.h"

#include <QtCore/QAbstractItemModel>


//-----------------------------------------------------------------------------
// ChangeCollectionCommand
//-----------------------------------------------------------------------------

ChangeCollectionCommand::ChangeCollectionCommand(int oldCollectionId,
                                                 int newCollectionId) :
    m_currentCollectionId(newCollectionId),
    m_previousCollectionId(oldCollectionId),
    m_avoidConstructorRedo(true)
{
    setText(QObject::tr("collection change"));
}

ChangeCollectionCommand::~ChangeCollectionCommand()
{

}

void ChangeCollectionCommand::undo()
{
    MetadataEngine::getInstance().setCurrentCollectionId(m_previousCollectionId);
}

void ChangeCollectionCommand::redo()
{
    //since redo() is called automatically from constructor
    //use this to prevent the call
    if (m_avoidConstructorRedo) {
        m_avoidConstructorRedo = false;
        return;
    }

    MetadataEngine::getInstance().setCurrentCollectionId(m_currentCollectionId);
}


//-----------------------------------------------------------------------------
// RenameCollectionCommand
//-----------------------------------------------------------------------------

RenameCollectionCommand::RenameCollectionCommand(int row,
                                                 QAbstractItemModel *model,
                                                 const QVariant &oldName,
                                                 const QVariant &newName) :
    m_row(row),
    m_avoidConstructorRedo(true),
    m_model(model),
    m_oldData(oldName),
    m_newData(newName)
{
    setText(QObject::tr("collection edit"));
}

RenameCollectionCommand::~RenameCollectionCommand()
{

}

void RenameCollectionCommand::undo()
{
    QModelIndex index = m_model->index(m_row, 1);
    if (index.isValid())
        m_model->setData(index, m_oldData);
}

void RenameCollectionCommand::redo()
{
    //since redo() is called automatically from constructor
    //use this to prevent the call
    if (m_avoidConstructorRedo) {
        m_avoidConstructorRedo = false;
        return;
    }

    QModelIndex index = m_model->index(m_row, 1);
    if (index.isValid())
        m_model->setData(index, m_newData);
}

//-----------------------------------------------------------------------------
// AddFieldCommand
//-----------------------------------------------------------------------------

AddFieldCommand::AddFieldCommand(int collectionId, int fieldId,
                                 FieldOperation op,
                                 const QString &fieldName,
                                 MetadataEngine::FieldType type,
                                 const QString &displayProperties,
                                 const QString &editProperties,
                                 const QString &triggerProperties) :
    m_collectionId(collectionId),
    m_fieldId(fieldId),
    m_fieldOperation(op),
    m_avoidConstructorRedo(true),
    m_fieldName(fieldName),
    m_fieldType(type),
    m_displayProperties(displayProperties),
    m_editProperties(editProperties),
    m_triggerProperties(triggerProperties)
{
    switch (m_fieldOperation) {
    case AddField:
        setText(QObject::tr("field creation"));
        break;
    case DuplicateField:
        setText(QObject::tr("field duplication"));
        break;
    }
}

AddFieldCommand::~AddFieldCommand()
{

}

void AddFieldCommand::undo()
{
    MetadataEngine::getInstance().deleteField(m_fieldId, m_collectionId);
}

void AddFieldCommand::redo()
{
    //since redo() is called automatically from constructor
    //use this to prevent the call
    if (m_avoidConstructorRedo) {
        m_avoidConstructorRedo = false;
        return;
    }

    MetadataEngine::getInstance().createField(
                m_fieldName,
                m_fieldType,
                m_displayProperties,
                m_editProperties,
                m_triggerProperties,
                m_collectionId);
}


//-----------------------------------------------------------------------------
// ModFieldCommand
//-----------------------------------------------------------------------------

ModFieldCommand::ModFieldCommand(int collectionId, int fieldId,
                                 const QString &oldFieldName,
                                 const QString &oldDisplayProperties,
                                 const QString &oldEditProperties,
                                 const QString &oldTriggerProperties,
                                 const QString &newFieldName,
                                 const QString &newDisplayProperties,
                                 const QString &newEditProperties,
                                 const QString &newTriggerProperties) :
    m_collectionId(collectionId),
    m_fieldId(fieldId),
    m_avoidConstructorRedo(true),
    m_oldFieldName(oldFieldName),
    m_oldDisplayProperties(oldDisplayProperties),
    m_oldEditProperties(oldEditProperties),
    m_oldTriggerProperties(oldTriggerProperties),
    m_newFieldName(newFieldName),
    m_newDisplayProperties(newDisplayProperties),
    m_newEditProperties(newEditProperties),
    m_newTriggerProperties(newTriggerProperties)
{
    setText(QObject::tr("field modification"));
}

ModFieldCommand::~ModFieldCommand()
{

}

void ModFieldCommand::undo()
{
    MetadataEngine::getInstance().modifyField(m_fieldId,
                                              m_oldFieldName,
                                              m_oldDisplayProperties,
                                              m_oldEditProperties,
                                              m_oldTriggerProperties,
                                              m_collectionId);
    handleEditTriggers();
}

void ModFieldCommand::redo()
{
    //since redo() is called automatically from constructor
    //use this to prevent the call
    if (m_avoidConstructorRedo) {
        m_avoidConstructorRedo = false;
        return;
    }

    MetadataEngine::getInstance().modifyField(m_fieldId,
                                              m_newFieldName,
                                              m_newDisplayProperties,
                                              m_newEditProperties,
                                              m_newTriggerProperties,
                                              m_collectionId);
    handleEditTriggers();
}

void ModFieldCommand::handleEditTriggers()
{
    MetadataEngine *meta = &MetadataEngine::getInstance();

    switch (meta->getFieldType(m_fieldId, m_collectionId)) {
    case MetadataEngine::DateType:
    {
        //update alarms property if needed
        MetadataPropertiesParser parser(meta->getFieldProperties(
                                            MetadataEngine::TriggerProperty,
                                            m_fieldId, m_collectionId));
        if (parser.size() > 0) {
            //update alarms
            AlarmManager a;
            if (parser.getValue("alarmOnDate") == "1") {
                //add all alarms
                a.addAlarmsForExistingRecords(m_collectionId, m_fieldId);
            } else {
                //remove all alarms
                a.removeAllAlarms(m_collectionId, m_fieldId);
            }
        }
    }
        break;
    default:
        //no special triggers
        break;
    }
}


//-----------------------------------------------------------------------------
// ModRecordCommand
//-----------------------------------------------------------------------------

ModRecordCommand::ModRecordCommand(int collectionId,
                                   int row, int column,
                                   const QVariant &oldRecordData,
                                   const QVariant &newRecordData) :
    m_collectionId(collectionId),
    m_avoidConstructorRedo(true),
    m_row(row),
    m_column(column),
    m_oldRecordData(oldRecordData),
    m_newRecordData(newRecordData)
{
    setText(QObject::tr("record edit"));
}

ModRecordCommand::~ModRecordCommand()
{

}

void ModRecordCommand::undo()
{
    QAbstractItemModel *model = MainWindow::getCurrentModel();
    if (!model) return;

    QModelIndex index = model->index(m_row, m_column);
    if (index.isValid()) {
        model->setData(index, m_oldRecordData);
        handleEditTriggers(m_oldRecordData);
    }
}

void ModRecordCommand::redo()
{
    //since redo() is called automatically from constructor
    //use this to prevent the call
    if (m_avoidConstructorRedo) {
        m_avoidConstructorRedo = false;
        return;
    }

    QAbstractItemModel *model = MainWindow::getCurrentModel();
    if (!model) return;

    QModelIndex index = model->index(m_row, m_column);
    if (index.isValid()) {
        model->setData(index, m_newRecordData);
        handleEditTriggers(m_newRecordData);
    }
}

void ModRecordCommand::handleEditTriggers(const QVariant &data)
{
    MetadataEngine *meta = &MetadataEngine::getInstance();

    switch (meta->getFieldType(m_column, m_collectionId)) {
    case MetadataEngine::DateType:
    {
        //update alarm property if needed
        MetadataPropertiesParser parser(meta->getFieldProperties(
                                            MetadataEngine::TriggerProperty,
                                            m_column, m_collectionId));
        if (parser.size() > 0) {
            //update alarm
            if (parser.getValue("alarmOnDate") == "1") {
                int id = -1;
                QAbstractItemModel *model = MainWindow::getCurrentModel();
                if (model) {
                    QModelIndex index = model->index(m_row, 0);
                    if (index.isValid())
                        id = index.data().toInt();
                }
                QDateTime dateTime(data.toDateTime());
                AlarmManager a;
                a.addOrUpdateAlarm(m_collectionId,
                                   m_column, id,
                                   dateTime);
            }
        }
    }
        break;
    default:
        //no special triggers
        break;
    }
}


//-----------------------------------------------------------------------------
// NewRecordCommand
//-----------------------------------------------------------------------------

NewRecordCommand::NewRecordCommand() : m_avoidConstructorRedo(true)
{
    setText(QObject::tr("record creation"));
}

NewRecordCommand::~NewRecordCommand()
{

}

void NewRecordCommand::undo()
{
    QAbstractItemModel *model = MainWindow::getCurrentModel();
    if (!model) return;

    QModelIndex index;
    while (model->canFetchMore(index))
        model->fetchMore(index);

    int rowCount = model->rowCount();

    //delete last record
    model->removeRow(rowCount - 1);
}

void NewRecordCommand::redo()
{
    //since redo() is called automatically from constructor
    //use this to prevent the call
    if (m_avoidConstructorRedo) {
        m_avoidConstructorRedo = false;
        return;
    }

    QAbstractItemModel *model = MainWindow::getCurrentModel();
    if (!model) return;

    //add new record
    //assuming StandardModel only
    StandardModel *sModel = qobject_cast<StandardModel*>(model);
    if (sModel) {
        sModel->addRecord();
    }
}


//-----------------------------------------------------------------------------
// DeleteRecordCommand
//-----------------------------------------------------------------------------

DeleteRecordCommand::DeleteRecordCommand(int row, QUndoCommand *parent) :
    QUndoCommand(parent), m_avoidConstructorRedo(true),
    m_rowToDelete(row)
{
    setText(QObject::tr("record deletion"));

    //save old data
    QAbstractItemModel *model = MainWindow::getCurrentModel();
    if (model) {
        QModelIndex index;
        while (model->canFetchMore(index))
            model->fetchMore(index);

        for (int i = 1; i < model->columnCount(); i++) { //1 cause 0 is _id
            index = model->index(m_rowToDelete, i);
            m_dataList.append(index.data());
        }
    }
}

DeleteRecordCommand::~DeleteRecordCommand()
{

}

void DeleteRecordCommand::undo()
{
    QAbstractItemModel *model = MainWindow::getCurrentModel();
    if (!model) return;

    QModelIndex index;
    while (model->canFetchMore(index))
        model->fetchMore(index);
    int rowCount = model->rowCount();

    //insert row again at the end
    model->insertRow(rowCount++);
    index = model->index(rowCount - 1, 0);

    //set old data
    for (int i = 1; i < model->columnCount(); i++) {
        index = model->index(index.row(), i);
        model->setData(index, m_dataList.at(i-1)); //-1 cause list starts at 0
                                                   //but column at 1 (0 is _id)
    }

    model->submit();
}

void DeleteRecordCommand::redo()
{
    //since redo() is called automatically from constructor
    //use this to prevent the call
    if (m_avoidConstructorRedo) {
        m_avoidConstructorRedo = false;
        return;
    }

    QAbstractItemModel *model = MainWindow::getCurrentModel();
    if (!model) return;

    QModelIndex index;
    while (model->canFetchMore(index))
        model->fetchMore(index);
    int rowCount = model->rowCount();

    model->removeRow(rowCount - 1); //delete last record
}


//-----------------------------------------------------------------------------
// DuplicateRecordCommand
//-----------------------------------------------------------------------------

DuplicateRecordCommand::DuplicateRecordCommand(int row, QUndoCommand *parent) :
    QUndoCommand(parent), m_avoidConstructorRedo(true),
    m_rowToDuplicate(row)
{
    setText(QObject::tr("record duplication"));
}

DuplicateRecordCommand::~DuplicateRecordCommand()
{

}

void DuplicateRecordCommand::undo()
{
    QAbstractItemModel *model = MainWindow::getCurrentModel();
    if (!model) return;

    QModelIndex index;
    while (model->canFetchMore(index))
        model->fetchMore(index);
    int rowCount = model->rowCount();

    model->removeRow(rowCount - 1); //delete last record
}

void DuplicateRecordCommand::redo()
{
    //since redo() is called automatically from constructor
    //use this to prevent the call
    if (m_avoidConstructorRedo) {
        m_avoidConstructorRedo = false;
        return;
    }

    QAbstractItemModel *model = MainWindow::getCurrentModel();
    if (!model) return;

    //duplicate record
    //assuming StandardModel only
    StandardModel *sModel = qobject_cast<StandardModel*>(model);
    if (sModel) {
        sModel->duplicateRecord(m_rowToDuplicate);
    }
}


//-----------------------------------------------------------------------------
// FormLayoutChangeCommand
//-----------------------------------------------------------------------------

FormLayoutChangeCommand::FormLayoutChangeCommand(FormViewLayoutState &oldLayout,
                                                 FormViewLayoutState &newLayout,
                                                 FormView *formView) :
    m_avoidConstructorRedo(true), m_oldLayout(0), m_newLayout(0),
    m_formView(formView)
{
    setText(QObject::tr("field layout change"));

    //create copy
    m_oldLayout = new FormViewLayoutState(oldLayout);
    m_newLayout = new FormViewLayoutState(newLayout);
}

FormLayoutChangeCommand::~FormLayoutChangeCommand()
{
    if (m_oldLayout)
        delete m_oldLayout;
    if (m_newLayout)
        delete m_newLayout;
}

void FormLayoutChangeCommand::undo()
{
    m_formView->setFormLayoutState(*m_oldLayout);
}

void FormLayoutChangeCommand::redo()
{
    //since redo() is called automatically from constructor
    //use this to prevent the call
    if (m_avoidConstructorRedo) {
        m_avoidConstructorRedo = false;
        return;
    }

    m_formView->setFormLayoutState(*m_newLayout);
}
