/**
  * \class ProgressFormWidget
  * \brief A form widget representing progress fields
  * \author Giorgio Wicklein - GIOWISYS Software
  * \date 18/10/2012
  */

#ifndef PROGRESSFORMWIDGET_H
#define PROGRESSFORMWIDGET_H


//-----------------------------------------------------------------------------
// Headers
//-----------------------------------------------------------------------------

#include "abstractformwidget.h"


//-----------------------------------------------------------------------------
// Forward declarations
//-----------------------------------------------------------------------------

class QLabel;
class QHBoxLayout;
class QVBoxLayout;
class QProgressBar;
class QSpinBox;


//-----------------------------------------------------------------------------
// ProgressFormWidget
//-----------------------------------------------------------------------------

class ProgressFormWidget : public AbstractFormWidget
{
    Q_OBJECT

public:
    ProgressFormWidget(QWidget *parent = 0);

    void setFieldName(const QString &name);
    QString getFieldName() const;
    void clearData();
    void setData(const QVariant &data);
    QVariant getData() const;

    /**
     * Supported display properties are:
     * - max: int
     */
    void loadMetadataDisplayProperties(const QString &metadata);

protected:
    void focusOutEvent(QFocusEvent *event);

protected slots:
    /**
     * Supported edit properties are:
     * (none for now)
     */
    void validateData();

private:
    /** Set the focus policy to accept focus and to redirect it to input line */
    void setupFocusPolicy();

    QLabel *m_fieldNameLabel;
    QProgressBar *m_progressBar;
    QSpinBox *m_spinBox;
    QVBoxLayout *m_mainLayout;
    QHBoxLayout *m_progressLayout;
    int m_maxValue; /**< The max progress value */
    int m_lastValidProgress; /**< last valiud progress value */
};

#endif // PROGRESSFORMWIDGET_H
