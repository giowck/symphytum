/**
  * \class NumberFormWidget
  * \brief A form widget representing numeric fields
  * \author Giorgio Wicklein - GIOWISYS Software
  * \date 29/06/2012
  */

#ifndef NUMBERFORMWIDGET_H
#define NUMBERFORMWIDGET_H


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


//-----------------------------------------------------------------------------
// NumberFormWidget
//-----------------------------------------------------------------------------

class NumberFormWidget : public AbstractFormWidget
{
    Q_OBJECT

public:
    NumberFormWidget(QWidget *parent = 0);

    void setFieldName(const QString &name);
    QString getFieldName() const;
    void clearData();
    void setData(const QVariant &data);
    QVariant getData() const;
    bool showHighlightSearchResults(const QString &searchString);

    /**
     * Supported display properties are:
     * - displayMode: auto, decimal, scientific
     * - precision: int
     * - markNegative: 1, 0
     * - markEmpty: 1, 0
     */
    void loadMetadataDisplayProperties(const QString &metadata);

    /** This enum describes the supported display modes of numeric values */
    enum DisplayMode {
        AutoDetect, /**< Decimal dot is shown if needed so its precision.
                         Very large numbers are shown in scientific notation */
        DecimalNormal, /**< The decimal dot is always shown with the precision
                            specified by m_precision */
        ScientificNotation /**< All values are shown in scientific notation
                                with the precision specified by m_precision */
    };

protected slots:
    /**
     * Supported edit properties are:
     * - noEmpty: 1, 0
     */
    void validateData();

private:
    /** This method updates the style sheet according to active display properties */
    void updateStyleSheet();

    /** Set the focus policy to accept focus and to redirect it to input line */
    void setupFocusPolicy();

    QLabel *m_fieldNameLabel;
    QLineEdit *m_lineEdit;
    QVBoxLayout *m_mainLayout;
    int m_precision; /**< The after decimal dot digit count */
    bool m_markNegative; /**< If true, negative values are marked red */
    bool m_markEmpty; /**< If data is empty mark field */
    DisplayMode m_displayMode; /**< The preferred display mode */
};

#endif // NUMBERFORMWIDGET_H
