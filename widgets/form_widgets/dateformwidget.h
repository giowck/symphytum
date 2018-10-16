/**
  * \class DateFormWidget
  * \brief A form widget representing date/time fields
  * \author Giorgio Wicklein - GIOWISYS Software
  * \date 29/10/2012
  */

#ifndef DATEFORMWIDGET_H
#define DATEFORMWIDGET_H


//-----------------------------------------------------------------------------
// Headers
//-----------------------------------------------------------------------------

#include "abstractformwidget.h"


//-----------------------------------------------------------------------------
// Forward declarations
//-----------------------------------------------------------------------------

class QLabel;
class QDateTimeEdit;
class QVBoxLayout;
class QDateTime;


//-----------------------------------------------------------------------------
// DateFormWidget
//-----------------------------------------------------------------------------

class DateFormWidget : public AbstractFormWidget
{
    Q_OBJECT

public:
    DateFormWidget(QWidget *parent = nullptr);
    ~DateFormWidget();

    void setFieldName(const QString &name);
    QString getFieldName() const;
    void clearData();
    void setData(const QVariant &data);
    QVariant getData() const;

    /**
     * Supported display properties are:
     * - dateFormat: 1, 2, 3, 4, 5, 6
     * - alarmOnDate: 0, 1
     */
    void loadMetadataDisplayProperties(const QString &metadata);

protected slots:
    /**
     * Supported edit properties are:
     * (none for now)
     */
    void validateData();

    void editingFinishedSlot();

private:
    /** Set the focus policy to accept focus and to redirect it to input line */
    void setupFocusPolicy();

    QLabel *m_fieldNameLabel;
    QDateTimeEdit *m_dateTimeEdit;
    QVBoxLayout *m_mainLayout;
    QDateTime *m_lastValidDateTime;
};

#endif // DATEFORMWIDGET_H
