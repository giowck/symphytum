/**
  * \class SelectRectWidget
  * \brief This widget represents a rectangle which is used in FormView as
  *        the selection rect to mark current selection.
  * \author Giorgio Wicklein - GIOWISYS Software
  * \date 03/05/2012
  */

#ifndef SELECTRECTWIDGET_H
#define SELECTRECTWIDGET_H


//-----------------------------------------------------------------------------
// Headers
//-----------------------------------------------------------------------------

#include <QWidget>


//-----------------------------------------------------------------------------
// Forward declarations
//-----------------------------------------------------------------------------

class QLabel;
class QVBoxLayout;


//-----------------------------------------------------------------------------
// SelectRectWidget
//-----------------------------------------------------------------------------

class SelectRectWidget : public QWidget
{
    Q_OBJECT
public:
    explicit SelectRectWidget(QWidget *parent = 0);
    
private:
    QLabel *label;
    QVBoxLayout *layout;
};

#endif // SELECTRECTWIDGET_H
