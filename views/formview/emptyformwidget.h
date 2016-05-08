/**
  * \class EmptyFormWidget
  * \brief This widget represents a placeholder for an empty FormView
  * \author Giorgio Wicklein - GIOWISYS Software
  * \date 13/07/2012
  */

#ifndef EMPTYFORMWIDGET_H
#define EMPTYFORMWIDGET_H


//-----------------------------------------------------------------------------
// Headers
//-----------------------------------------------------------------------------

#include <QWidget>


//-----------------------------------------------------------------------------
// Forward declarations
//-----------------------------------------------------------------------------

namespace Ui {
class EmptyFormWidget;
}


//-----------------------------------------------------------------------------
// EmptyFormWidget
//-----------------------------------------------------------------------------

class EmptyFormWidget : public QWidget
{
    Q_OBJECT
    
public:

    /**
     * This enum represents the states in which this
     * help widget can be
     */
    enum State {
        AllMissing, /**< There are no collections, fields nor records */
        FieldMissing, /**< Collections: yes, Fields: no, Records: no */
        RecordMissing, /**< Collections: yes, Fields: yes, Records: no */
        MissingModel /**< Model is not set */
    };

    /**
     * Set the state of the help messages, according to the state,
     * some messages are shown or hidde
     */
    void setState(State s);

    explicit EmptyFormWidget(QWidget *parent = 0);
    ~EmptyFormWidget();
    
private:
    Ui::EmptyFormWidget *ui;
};

#endif // EMPTYFORMWIDGET_H
