/**
  * \class FormWidgetValidator
  * \brief This utility is used to validate input data of form widgets and delegates.
  *        Each form widget has its own metadata edit properties which
  *        delineate valid data from invalid one. This utility is designed
  *        to do input validation in one central module, in order to avoid
  *        code duplication, for example FormView and TableView uses this.
  * \author Giorgio Wicklein - GIOWISYS Software
  * \date 04/07/2012
  */

#ifndef FORMWIDGETVALIDATOR_H
#define FORMWIDGETVALIDATOR_H


//-----------------------------------------------------------------------------
// Headers
//-----------------------------------------------------------------------------

#include "../components/metadataengine.h"


//-----------------------------------------------------------------------------
// Forward declarations
//-----------------------------------------------------------------------------

class MetadataPropertiesParser;
class QString;
class QVariant;


//-----------------------------------------------------------------------------
// FormWidgetValidator
//-----------------------------------------------------------------------------

class FormWidgetValidator
{
public:
    /** Construct a validator with the specified metadata edit properties */
    FormWidgetValidator(const QString &metadataEditProperties,
                        MetadataEngine::FieldType type);

    ~FormWidgetValidator();

    /**
     * Validate data
     * @param inputData - the data to validate
     * @param errorMessage - a reference where to save error message
     *                       if data is not valid (ie. why not valid)
     * @return bool - whether the data is valid or not
     */
    bool validate(const QVariant &inputData, QString &errorMessage);

private:
    bool validateTextType(const QVariant &inputData, QString &errorMessage);
    bool validateNumericType(const QVariant &inputData, QString &errorMessage);

    MetadataPropertiesParser *m_parser;
    MetadataEngine::FieldType m_fieldType;
};

#endif // FORMWIDGETVALIDATOR_H
