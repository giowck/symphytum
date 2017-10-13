/**
  * \class ComboboxFormWidget
  * \brief A form widget representing drop-down list fields
  * \author Giorgio Wicklein - GIOWISYS Software
  * \date 15/10/2012
  */

#ifndef COMBOBOXFORMWIDGET_H
#define COMBOBOXFORMWIDGET_H


//-----------------------------------------------------------------------------
// Headers
//-----------------------------------------------------------------------------

#include "abstractformwidget.h"


//-----------------------------------------------------------------------------
// Forward declarations
//-----------------------------------------------------------------------------

class QLabel;
class QVBoxLayout;
class QComboBox;


//-----------------------------------------------------------------------------
// ComboboxFormWidget
//-----------------------------------------------------------------------------

class ComboboxFormWidget : public AbstractFormWidget
{
    Q_OBJECT

public:
    ComboboxFormWidget(QWidget *parent = 0);

    void setFieldName(const QString &name);
    QString getFieldName() const;
    void clearData();
    void setData(const QVariant &data);
    QVariant getData() const;

    /**
     * Supported display properties are:
     * - items: string list (separator ';')
     * - default: int
     * - markEmpty: 1, 0
     */
    void loadMetadataDisplayProperties(const QString &metadata);

protected slots:
    /**
     * Supported edit properties are:
     * (none for now)
     */
    void validateData();

private:
    /** This method updates the style sheet according to active display properties */
    void updateStyleSheet();

    /** Set the focus policy to accept focus and to redirect it to input line */
    void setupFocusPolicy();

    QLabel *m_fieldNameLabel;
    QComboBox *m_comboBox;
    QVBoxLayout *m_mainLayout;
    QStringList m_itemNameList;
    int m_default; /**< The default value */
    bool m_markEmpty; /**< If data is empty mark field */
    int m_lastValidIndex; /**< Last valid index */
};

#endif // COMBOBOXFORMWIDGET_H
