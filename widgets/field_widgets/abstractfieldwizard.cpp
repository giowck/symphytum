/*
 *  Copyright (c) 2012 Giorgio Wicklein <giowckln@gmail.com>
 */

//-----------------------------------------------------------------------------
// Hearders
//-----------------------------------------------------------------------------

#include "abstractfieldwizard.h"
#include "../../components/undocommands.h"
#include "../../components/sync_framework/syncsession.h"
#include "../mainwindow.h"

#include <QtWidgets/QUndoCommand>
#include <QtWidgets/QUndoStack>


//-----------------------------------------------------------------------------
// Public
//-----------------------------------------------------------------------------

AbstractFieldWizard::AbstractFieldWizard(const QString &fieldName,
                                         QWidget *parent,
                                         EditMode editMode) :
    QWidget(parent), m_fieldId(-1), m_collectionId(0),
    m_fieldName(fieldName), m_currentEditMode(editMode)
{
}

AbstractFieldWizard::~AbstractFieldWizard()
{

}

void AbstractFieldWizard::loadField(const int fieldId, const int collectionId)
{
    m_fieldId = fieldId;
    m_collectionId = collectionId;
}

void AbstractFieldWizard::createField(MetadataEngine::FieldType fieldType,
                                      EditMode mode)
{
    MetadataEngine *meta = &MetadataEngine::getInstance();
    int fieldId = m_fieldId;
    QString displayProperties;
    QString editProperties;
    QString triggerProperties;

    //if mode is modify field,
    //keep old properties for undo
    QString oldDisplayProperties;
    QString oldEditProperties;
    QString oldTriggerProperties;
    QString oldFieldName;

    //collection id must be valid, if not specified
    if (m_collectionId == 0)
        m_collectionId = MetadataEngine::getInstance().getCurrentCollectionId();

    //get field properties
    getFieldProperties(displayProperties,
                       editProperties,
                       triggerProperties);

    if (mode == AbstractFieldWizard::NewEditMode) {
        fieldId = meta->createField(m_fieldName, fieldType, displayProperties,
                                    editProperties, triggerProperties);
    } else if (mode == AbstractFieldWizard::ModifyEditMode) {
        //save old properties for undo
        oldDisplayProperties = meta->getFieldProperties(
                    MetadataEngine::DisplayProperty,
                    fieldId, m_collectionId);
        oldEditProperties = meta->getFieldProperties(
                    MetadataEngine::EditProperty,
                    fieldId, m_collectionId);
        oldTriggerProperties = meta->getFieldProperties(
                    MetadataEngine::TriggerProperty,
                    fieldId, m_collectionId);
        oldFieldName = meta->getFieldName(fieldId, m_collectionId);

        meta->modifyField(fieldId, m_fieldName, displayProperties,
                          editProperties, triggerProperties);
    } else if (mode == AbstractFieldWizard::DuplicateEditMode) {
        fieldId = meta->createField(m_fieldName, fieldType, displayProperties,
                                    editProperties, triggerProperties);
    }

    //set local data changed
    SyncSession::LOCAL_DATA_CHANGED = true;

    //create undo action
    QUndoStack *stack = MainWindow::getUndoStack();
    if (stack) {
        QUndoCommand *command;

        if (mode == ModifyEditMode) {
            command = new ModFieldCommand(m_collectionId, fieldId,
                                          oldFieldName,
                                          oldDisplayProperties,
                                          oldEditProperties,
                                          oldTriggerProperties,
                                          m_fieldName,
                                          displayProperties,
                                          editProperties,
                                          triggerProperties);
        } else { //new or duplicate
            AddFieldCommand::FieldOperation op;
            if (mode == NewEditMode)
                op = AddFieldCommand::AddField;
            else
                op = AddFieldCommand::DuplicateField;
            command = new AddFieldCommand(m_collectionId, fieldId,
                                          op, m_fieldName, fieldType,
                                          displayProperties,
                                          editProperties,
                                          triggerProperties);
        }
        stack->push(command);
    }
}

//-----------------------------------------------------------------------------
// Private
//-----------------------------------------------------------------------------
