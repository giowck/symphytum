/**
  * \class TextArea
  * \brief This widget is a reimplementation of QTextEdit
  *        to add support for the editingFinished() signal
  *        as in QLineEdit.
  * \author Giorgio Wicklein - GIOWISYS Software
  * \date 30/06/2012
  */

#ifndef TEXTAREA_H
#define TEXTAREA_H


//-----------------------------------------------------------------------------
// Headers
//-----------------------------------------------------------------------------

#include <QtWidgets/QTextEdit>


//-----------------------------------------------------------------------------
// TextArea
//-----------------------------------------------------------------------------

class TextArea : public QTextEdit
{
    Q_OBJECT

public:
    explicit TextArea(QWidget *parent = 0);

signals:
    void editingFinished();

protected:
    void focusOutEvent(QFocusEvent *e);
    void focusInEvent(QFocusEvent *e);
    void keyPressEvent(QKeyEvent *e);
};

#endif // TEXTAREA_H
