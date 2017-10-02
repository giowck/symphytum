/*
 *  Copyright (c) 2012 Giorgio Wicklein <giowckln@gmail.com>
 */

//-----------------------------------------------------------------------------
// Hearders
//-----------------------------------------------------------------------------

#include "metadatapropertiesparser.h"
#include "formwidgetvalidator.h"
#include "../components/sync_framework/syncsession.h"

#include <QtCore/QVariant>
#include <QtCore/QString>
#include <QtCore/QObject>


//-----------------------------------------------------------------------------
// Public
//-----------------------------------------------------------------------------

FormWidgetValidator::FormWidgetValidator(const QString &metadataEditProperties,
                                         MetadataEngine::FieldType type) :
    m_parser(0), m_fieldType(MetadataEngine::TextType)
{
    m_fieldType = type;
    m_parser = new MetadataPropertiesParser(metadataEditProperties);
}

FormWidgetValidator::~FormWidgetValidator()
{
    if (m_parser)
        delete m_parser;
}

bool FormWidgetValidator::validate(const QVariant &inputData,
                                   QString &errorMessage)
{
    bool valid = true;

    //if read-only mode, reject
    if (SyncSession::IS_READ_ONLY) {
        errorMessage.append(QObject::tr("Read-only mode: "
                                        "Editing is not allowed."));
        return false;
    }

    switch (m_fieldType) {
    case MetadataEngine::TextType:
    case MetadataEngine::URLTextType:
    case MetadataEngine::EmailTextType:
        valid = validateTextType(inputData, errorMessage);
        break;
    case MetadataEngine::NumericType:
        valid = validateNumericType(inputData, errorMessage);
        break;
    case MetadataEngine::DateType:
    case MetadataEngine::CreationDateType:
    case MetadataEngine::ModDateType:
    case MetadataEngine::CheckboxType:
    case MetadataEngine::ComboboxType:
    case MetadataEngine::ProgressType:
    case MetadataEngine::ImageType:
    case MetadataEngine::FilesType:
        valid = true; //always valid
        break;
    default:
        valid = true;
    }

    return valid;
}


//-----------------------------------------------------------------------------
// Private
//-----------------------------------------------------------------------------

bool FormWidgetValidator::validateTextType(const QVariant &inputData,
                                           QString &errorMessage)
{
    bool valid = true;
    QString v;

    //check no empty property
    v = m_parser->getValue("noEmpty");
    if (v == "1" && inputData.toString().isEmpty()) {
        valid = false;
        errorMessage.append(QObject::tr("Required field: empty values not allowed\n"));
    }

    return valid;
}

bool FormWidgetValidator::validateNumericType(const QVariant &inputData,
                                              QString &errorMessage)
{
    bool valid = true;
    QString v;
    QString inputString = inputData.toString();

    //check if numeric
    if (!inputString.isEmpty()) { //allow empty values
        bool ok;
        inputString.toDouble(&ok);
        if (!ok)
            valid = false;
    }

    //check no empty property
    v = m_parser->getValue("noEmpty");
    if (v == "1" && inputString.isEmpty()) {
        valid = false;
        errorMessage.append(QObject::tr("Required field: empty values not allowed\n"));
    }

    return valid;
}
