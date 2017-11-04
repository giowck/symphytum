/**
  * \class TextFormWidget
  * \brief A form widget representing text fields.
  *        This widget uses a QLineEdit if the height is 1 unit. When the height
  *        changes above 1 unit, the line edit is replaced by a QTextEdit in
  *        order to display more than one text line.
  * \author Giorgio Wicklein - GIOWISYS Software
  * \date 29/06/2012
  */

#ifndef TEXTFORMWIDGET_H
#define TEXTFORMWIDGET_H


//-----------------------------------------------------------------------------
// Headers
//-----------------------------------------------------------------------------

#include "abstractformwidget.h"


//-----------------------------------------------------------------------------
// Forward declarations
//-----------------------------------------------------------------------------

class QLabel;
class QVBoxLayout;
class QLineEdit;
class TextArea;


//-----------------------------------------------------------------------------
// TextFormWidget
//-----------------------------------------------------------------------------

class TextFormWidget : public AbstractFormWidget
{
    Q_OBJECT

public:
    explicit TextFormWidget(QWidget *parent = 0);

    void setFieldName(const QString &name);
    QString getFieldName() const;
    void clearData();
    void setData(const QVariant &data);
    QVariant getData() const;
    bool showHighlightSearchResults(const QString &searchString);

    /**
     * Supported display properties are:
     * - markEmpty: 1, 0
     */
    void loadMetadataDisplayProperties(const QString &metadata);

    /** Reimplementing this method to detect height changes */
    void setHeightUnits(int heightUnits);

protected slots:
    /**
     * Supported edit properties are:
     * - noEmpty: 1, 0
     */
    void validateData();
    
private:
    /** This method updates the style sheet according to active display properties */
    void updateStyleSheet();

    /** Update focus policy to accept focus and redirect it to the correct input field */
    void updateFocusPolicy();

    QLabel *m_fieldNameLabel;
    QLineEdit *m_lineEdit;
    TextArea *m_textArea;
    QVBoxLayout *m_mainLayout;
    bool m_multiLine; /**< Indicates if the widget is displaying multiple lines */
    bool m_markEmpty; /**< If data is empty mark field */
};

#endif // TEXTFORMWIDGET_H
