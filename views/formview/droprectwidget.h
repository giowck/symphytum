/**
  * \class DropRectWidget
  * \brief This widget represents a rectangle which is used in FormView as
  *        the drop shadow/rect during drag/move operation to mark target index.
  * \author Giorgio Wicklein - GIOWISYS Software
  * \date 03/05/2012
  */

#ifndef DROPRECTWIDGET_H
#define DROPRECTWIDGET_H


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
// DropRectWidget
//-----------------------------------------------------------------------------

class DropRectWidget : public QWidget
{
    Q_OBJECT
public:
    explicit DropRectWidget(QWidget *parent = 0);

private:
    QLabel *label;
    QVBoxLayout *layout;
};

#endif // DROPRECTWIDGET_H
