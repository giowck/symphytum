/**
  * \class AbstractFieldWizard
  * \brief This class represents an abstract base class for all
  *        wizards implementing field configuration and creation.
  *        Each subclassing wizard has to handle the field creation
  *        by implementing the pure abstract methods.
  * \author Giorgio Wicklein - GIOWISYS Software
  * \date 20/07/2012
  */

#ifndef ABSTRACTFIELDWIZARD_H
#define ABSTRACTFIELDWIZARD_H


//-----------------------------------------------------------------------------
// Headers
//-----------------------------------------------------------------------------

#include <QtWidgets/QWidget>

#include "../../components/metadataengine.h"


//-----------------------------------------------------------------------------
// Forward declarations
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// AbstractFieldWizard
//-----------------------------------------------------------------------------

class AbstractFieldWizard : public QWidget
{
    Q_OBJECT

public:

    /** This enum holds the editing modes */
    enum EditMode {
        NewEditMode, /**< A new field */
        ModifyEditMode, /**< An existing field is being modified */
        DuplicateEditMode /**< An existing field is being duplicated */
    };

    explicit AbstractFieldWizard(const QString &fieldName,
                                 QWidget *parent = nullptr,
                                 EditMode editMode = NewEditMode);
    virtual ~AbstractFieldWizard();

    /**
     * This method extracts the field properties from the wizard.
     * This means that based on the state of the subclassing wizard
     * all field properties strings are populated and valid metadata strings.
     * For example: if "Required field" checkbox is checked, editProperties
     * will contain "noEmpty:1;" and so on...
     * Each subclass must implement this method, so that field properties
     * are available for the createField() method.
     */
    virtual void getFieldProperties(QString &displayProperties,
                                    QString &editProperties,
                                    QString &triggerProperties) = 0;

    /**
     * This method is called to load properties of the specified field
     * into the wizard's widgets, so that the user can modify field
     * properties. Subclassing wizards should populate here their input fields.
     * @param fieldId - the field number. Note: 0 is _id column/field
     * @param collectionId - the id of the collection, if not specified
     *                       current collection will be used
     */
    virtual void loadField(const int fieldId, const int collectionId = 0);

    /**
     * Create field with the configuration (parameters) from the wizard.
     * Field properties from the wizard are obtained by calling
     * getFieldProperties() on the correct subclass to
     * create the field with apropriate metadata.
     * @param fieldType - the type of the field to create
     * @param mode - whether the field to create is new or is modified
     *               from an existing one.
     *               Note that the corresponding fieldId (column number)
     *               is passed in loadField() and stored in m_fieldId,
     *               same for collectionId.
     */
    void createField(MetadataEngine::FieldType fieldType, EditMode mode);

signals:
    /**
     * This signal is usually emitted when the back button
     * has been pressed on the wizard. This signal is catched
     * by AddFieldDialog to set the current page of the stacked widget
     * to the previous one.
     */
    void backSignal();

    /**
     * This signal is usually emitted when the finish button
     * has been pressed on the wizard. This signal is catched
     * by AddFieldDialog which in turn calls createField() on
     * the apropriate wizard implementation.
     */
    void finishSignal();

protected:
    int m_fieldId; /**< The field which is being edited/duplicated
                        if editing mode is modify */
    int m_collectionId; /**< The collection if editing mode is modify */
    QString m_fieldName; /**< Current field name */
    EditMode m_currentEditMode; /**< Current edit mode */
};

#endif // ABSTRACTFIELDWIZARD_H
