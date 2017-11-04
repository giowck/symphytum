/**
  * \class AbstractFormWidget
  * \brief This is the abstract base widget for all form widgets.
  *        When subclassing this base widget, the derived widgets must
  *        initialize the heightUnits and widthUnits variable, those are used
  *        by FormLayoutMatrix to determine how much space is used by the widget on the
  *        layout, for ex. widthUnits=1 means that the widget uses one horizontal
  *        unit of the matrix grid (column).
  * \author Giorgio Wicklein - GIOWISYS Software
  * \date 10/04/2012
  */

#ifndef ABSTRACTFORMWIDGET_H
#define ABSTRACTFORMWIDGET_H


//-----------------------------------------------------------------------------
// Headers
//-----------------------------------------------------------------------------

#include <QtWidgets/QWidget>


//-----------------------------------------------------------------------------
// Forward declarations
//-----------------------------------------------------------------------------

class QString;
class QVariant;


//-----------------------------------------------------------------------------
// AbstractFormWidget
//-----------------------------------------------------------------------------

class AbstractFormWidget : public QWidget
{
    Q_OBJECT

public:
    explicit AbstractFormWidget(QWidget *parent = 0);
    virtual ~AbstractFormWidget();

    /** Get the height unit need for FormLayoutMatrix */
    int getHeightUnits() const;

    /** Get the width unit need for FormLayoutMatrix */
    int getWidthUnits() const;

    /** Set the height unit need for FormLayoutMatrix */
    virtual void setHeightUnits(int heightUnits);

    /** Set the width unit need for FormLayoutMatrix */
    virtual void setWidthUnits(int widthUnits);

    /**
     * Set the field/column name of the widget. Each widget needs to implement
     * this method, because each widget displays the name of its own way.
     */
    virtual void setFieldName(const QString &name) = 0;

    /** Return the field/column name of the widget */
    virtual QString getFieldName() const = 0;

    /** Set the fieldId/columnId which this widget is representing */
    void setFieldId(int columnId);

    /** Return the fieldId/columnId which this widget is representing */
    int getFieldId();

    /** Clear/Reset the content of the form widget */
    virtual void clearData() = 0;

    /**
     * Set the content data of the form widget.
     * @param data - the data as QVariant, since every
     *               form widget has its own data type
     */
    virtual void setData(const QVariant& data) = 0;

    /**
     * Get the content data of the form widget.
     * @return data - the data as QVariant, since every
     *                form widget has its own data type
     */
    virtual QVariant getData() const = 0;

    /**
     * This method initializes metadata properties which affect
     * the display properties of the data only. In other words,
     * some field types (form widget) may have some specific metadata
     * tags which describe how the field displays the data.
     * See /stuff/doc/Standard Database Design.
     * Form widget subclasses should reimplement this method
     * only if they need to support metadata display properties.
     * @param metadata - a string containing the metadata in form
     *                   "key:value;key2:value2;key3:value1,value2;"
     */
    virtual void loadMetadataDisplayProperties(const QString &metadata) {Q_UNUSED(metadata)}

    /** Overload operator == for comparison,
      * two widgets are equal if they point to the same address
      */
    bool operator== (const AbstractFormWidget& other) const;

    /** Overload operator != for comparison,
      * two widgets are equal if they point to the same address
      */
    bool operator!= (const AbstractFormWidget& other) const;

    /** Highlight the form widget or its contents if the searched string is matched.
     * Default implementation returns false, implement if applicable.
     * If no match is found, make sure to clear any highlighted state.
     * @param searchString - the string to match against the data
     * @return bool - if the data was found (matched) or not
     */
    virtual bool showHighlightSearchResults(const QString& searchString);

signals:
    /** Emitted when content data has been edited */
    void dataEdited();

    /**
     * Emitted if the form widget asks for attention
     * @param message - QString with a message to user
     */
    void requiresAttention(QString &message);

protected slots:
    /**
     * This slot should be called to validate input data.
     * Reimplement this slot only if the form widget subclass
     * needs to validate input data as defined by its edit
     * metadata properties, if any.
     * When implementing this slot, make sure to emit the
     * dataEdited() signal if the data is valid: according to
     * edit metadata properties. So that the data
     * is passed to the model.
     */
    virtual void validateData() {}

protected:
    int widthUnits;  /**< The needed width units used in FormLayoutMatrix  */
    int heightUnits; /**< The needed height units used in FormLayoutMatrix */

private:
    int fieldId; /**< The id of the field/column represented */
};

#endif // ABSTRACTFORMWIDGET_H
