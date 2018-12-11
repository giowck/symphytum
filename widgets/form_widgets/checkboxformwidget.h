/**
  * \class CheckboxFormWidget
  * \brief A form widget representing checkbox fields
  * \author Giorgio Wicklein - GIOWISYS Software
  * \date 06/10/2012
  */

#ifndef CHECKBOXFORMWIDGET_H
#define CHECKBOXFORMWIDGET_H


//-----------------------------------------------------------------------------
// Headers
//-----------------------------------------------------------------------------

#include "abstractformwidget.h"


//-----------------------------------------------------------------------------
// Forward declarations
//-----------------------------------------------------------------------------

class QLabel;
class QVBoxLayout;
class QCheckBox;


//-----------------------------------------------------------------------------
// CheckboxFormWidget
//-----------------------------------------------------------------------------

class CheckboxFormWidget : public AbstractFormWidget
{
    Q_OBJECT

public:
    CheckboxFormWidget(QWidget *parent = nullptr);

    void setFieldName(const QString &name);
    QString getFieldName() const;
    void clearData();
    void setData(const QVariant &data);
    QVariant getData() const;

    /**
     * Supported display properties are:
     * (none for now)
     */
    void loadMetadataDisplayProperties(const QString &metadata);

protected slots:
    /**
     * Supported edit properties are:
     * (none for now)
     */
    void validateData();

private:
    /** Set the focus policy to accept focus and to redirect it to input line */
    void setupFocusPolicy();

    QCheckBox *m_checkbox;
    QVBoxLayout *m_mainLayout;
};

#endif // CHECKBOXFORMWIDGET_H
