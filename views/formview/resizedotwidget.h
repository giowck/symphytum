/**
  * \class ResizeDotWidget
  * \brief This widget represents a shape which is used in FormView as
  *        the resizing grip of a FW during resize operation.
  * \author Giorgio Wicklein - GIOWISYS Software
  * \date 04/05/2012
  */

#ifndef RESIZEDOTWIDGET_H
#define RESIZEDOTWIDGET_H


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
// ResizeDotWidget
//-----------------------------------------------------------------------------

class ResizeDotWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ResizeDotWidget(QWidget *parent = nullptr);
    bool isResizing; /**< A boolean indicating if the grip is
                          currently in a drag resize operation */

private:
    QLabel *label;
    QVBoxLayout *layout;
};

#endif // RESIZEDOTWIDGET_H
