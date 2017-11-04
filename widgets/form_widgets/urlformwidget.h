/**
  * \class URLFormWidget
  * \brief A form widget representing URL fields.
  *        This widget uses a QLineEdit with an inline button for browser action.
  * \author Giorgio Wicklein
  * \date 28/05/2016
  */

#ifndef URLFORMWIDGET_H
#define URLFORMWIDGET_H


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
class QAction;


//-----------------------------------------------------------------------------
// URLFormWidget
//-----------------------------------------------------------------------------

class URLFormWidget : public AbstractFormWidget
{
    Q_OBJECT

public:
    explicit URLFormWidget(QWidget *parent = 0);

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

protected slots:
    /**
     * Supported edit properties are:
     * - noEmpty: 1, 0
     */
    void validateData();

private slots:
    void openURLActionTriggered();

private:
    /** This method updates the style sheet according to active display properties */
    void updateStyleSheet();

    /** Update focus policy to accept focus and redirect it to the correct input field */
    void updateFocusPolicy();

    QLabel *m_fieldNameLabel;
    QLineEdit *m_lineEdit;
    QVBoxLayout *m_mainLayout;
    QAction *m_openURLAction;
    bool m_markEmpty; /**< If data is empty mark field */
};

#endif // URLFORMWIDGET_H
