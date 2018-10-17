/**
  * \file  undocommands.h
  * \brief This contains all undo commands grouped together.
  * \author Giorgio Wicklein - GIOWISYS Software
  * \date 02/08/2012
  */

#ifndef UNDOCOMMANDS_H
#define UNDOCOMMANDS_H


//-----------------------------------------------------------------------------
// Headers
//-----------------------------------------------------------------------------

#include <QtWidgets/QUndoCommand>
#include <QtCore/QVariant>
#include <QtCore/QList>

#include "metadataengine.h"


//-----------------------------------------------------------------------------
// Forward declarations
//-----------------------------------------------------------------------------

class FormViewLayoutState;
class FormView;


//-----------------------------------------------------------------------------
// ChangeCollectionCommand
//-----------------------------------------------------------------------------

/**
  * \class ChangeCollectionCommand
  * \brief Command to change current active collection.
  * \author Giorgio Wicklein - GIOWISYS Software
  * \date 02/08/2012
  */
class ChangeCollectionCommand : public QUndoCommand
{
public:
    ChangeCollectionCommand(int oldCollectionId, int newCollectionId);
    ~ChangeCollectionCommand();

    void undo();
    void redo();

private:
    int m_currentCollectionId;
    int m_previousCollectionId;
    bool m_avoidConstructorRedo;
};


//-----------------------------------------------------------------------------
// RenameCollectionCommand
//-----------------------------------------------------------------------------

/**
  * \class RenameCollectionCommand
  * \brief Command to change name of a collection.
  * \author Giorgio Wicklein - GIOWISYS Software
  * \date 03/08/2012
  */
class RenameCollectionCommand : public QUndoCommand
{
public:
    RenameCollectionCommand(int row,
                            QAbstractItemModel *model,
                            const QVariant& oldName,
                            const QVariant& newName);
    ~RenameCollectionCommand();

    void undo();
    void redo();

private:
    int m_row;
    bool m_avoidConstructorRedo;
    QAbstractItemModel *m_model;

    //data
    QVariant m_oldData;
    QVariant m_newData;
};


//-----------------------------------------------------------------------------
// AddFieldCommand
//-----------------------------------------------------------------------------

/**
  * \class AddFieldCommand
  * \brief Command to undo/redo field add/duplication.
  * \author Giorgio Wicklein - GIOWISYS Software
  * \date 02/08/2012
  */
class AddFieldCommand : public QUndoCommand
{
public:
    /** Supported operations on fields */
    enum FieldOperation {
        AddField,
        DuplicateField
    };

    AddFieldCommand(int collectionId, int fieldId, FieldOperation op,
                    const QString &fieldName, MetadataEngine::FieldType type,
                    const QString &displayProperties,
                    const QString &editProperties,
                    const QString &triggerProperties);
    ~AddFieldCommand();

    void undo();
    void redo();

private:
    int m_collectionId;
    int m_fieldId;
    FieldOperation m_fieldOperation;
    bool m_avoidConstructorRedo;

    //field attributes
    QString m_fieldName;
    MetadataEngine::FieldType m_fieldType;
    QString m_displayProperties;
    QString m_editProperties;
    QString m_triggerProperties;
};


//-----------------------------------------------------------------------------
// ModFieldCommand
//-----------------------------------------------------------------------------

/**
  * \class ModFieldCommand
  * \brief Command to undo/redo field modification.
  * \author Giorgio Wicklein - GIOWISYS Software
  * \date 03/08/2012
  */
class ModFieldCommand : public QUndoCommand
{
public:
    ModFieldCommand(int collectionId, int fieldId,
                    const QString &oldFieldName,
                    const QString &oldDisplayProperties,
                    const QString &oldEditProperties,
                    const QString &oldTriggerProperties,
                    const QString &newFieldName,
                    const QString &newDisplayProperties,
                    const QString &newEditProperties,
                    const QString &newTriggerProperties);
    ~ModFieldCommand();

    void undo();
    void redo();

private:
    void handleEditTriggers();

    int m_collectionId;
    int m_fieldId;
    bool m_avoidConstructorRedo;

    //field attributes
    QString m_oldFieldName;
    QString m_oldDisplayProperties;
    QString m_oldEditProperties;
    QString m_oldTriggerProperties;
    QString m_newFieldName;
    QString m_newDisplayProperties;
    QString m_newEditProperties;
    QString m_newTriggerProperties;
};


//-----------------------------------------------------------------------------
// ModRecordCommand
//-----------------------------------------------------------------------------

/**
  * \class ModRecordCommand
  * \brief Command to undo/redo record modification.
  * \author Giorgio Wicklein - GIOWISYS Software
  * \date 03/08/2012
  */
class ModRecordCommand : public QUndoCommand
{
public:
    ModRecordCommand(int collectionId,
                     int row, int column,
                     const QVariant &oldRecordData,
                     const QVariant &newRecordData);
    ~ModRecordCommand();

    void undo();
    void redo();

private:
    void handleEditTriggers(const QVariant &data);

    int m_collectionId;
    bool m_avoidConstructorRedo;

    //data
    int m_row;
    int m_column;
    QVariant m_oldRecordData;
    QVariant m_newRecordData;
};


//-----------------------------------------------------------------------------
// NewRecordCommand
//-----------------------------------------------------------------------------

/**
  * \class NewRecordCommand
  * \brief Command to undo/redo record creation.
  * \author Giorgio Wicklein - GIOWISYS Software
  * \date 03/08/2012
  */
class NewRecordCommand : public QUndoCommand
{
public:
    NewRecordCommand();
    ~NewRecordCommand();

    void undo();
    void redo();

private:
    bool m_avoidConstructorRedo;
};


//-----------------------------------------------------------------------------
// DeleteRecordCommand
//-----------------------------------------------------------------------------

/**
  * \class DeleteRecordCommand
  * \brief Command to undo/redo record deletion.
  * \author Giorgio Wicklein - GIOWISYS Software
  * \date 03/08/2012
  */
class DeleteRecordCommand : public QUndoCommand
{
public:
    DeleteRecordCommand(int row, QUndoCommand *parent = nullptr);
    ~DeleteRecordCommand();

    void undo();
    void redo();

private:
    bool m_avoidConstructorRedo;
    int m_rowToDelete;
    QList<QVariant> m_dataList;
};


//-----------------------------------------------------------------------------
// DuplicateRecordCommand
//-----------------------------------------------------------------------------

/**
  * \class DuplicateRecordCommand
  * \brief Command to undo/redo record duplication.
  * \author Giorgio Wicklein - GIOWISYS Software
  * \date 03/08/2012
  */
class DuplicateRecordCommand : public QUndoCommand
{
public:
    DuplicateRecordCommand(int row, QUndoCommand *parent = nullptr);
    ~DuplicateRecordCommand();

    void undo();
    void redo();

private:
    bool m_avoidConstructorRedo;
    int m_rowToDuplicate;
};

#endif // UNDOCOMMANDS_H


//-----------------------------------------------------------------------------
// FormLayoutChangeCommand
//-----------------------------------------------------------------------------

/**
  * \class FormLayoutChangeCommand
  * \brief Command to undo/redo a layout change on FormView.
  * \author Giorgio Wicklein - GIOWISYS Software
  * \date 03/08/2012
  */
class FormLayoutChangeCommand : public QUndoCommand
{
public:
    FormLayoutChangeCommand(FormViewLayoutState &oldLayout,
                            FormViewLayoutState &newLayout,
                            FormView *formView);
    ~FormLayoutChangeCommand();

    void undo();
    void redo();

private:
    bool m_avoidConstructorRedo;
    FormViewLayoutState *m_oldLayout;
    FormViewLayoutState *m_newLayout;
    FormView *m_formView;
};
