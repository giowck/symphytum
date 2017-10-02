/**
  * \class SearchLineEdit
  * \brief This widget represents a search line. It is used in MainWindow
  *        to allow search operations. This widget contains a clear button
  *        inside the input field to allow text reset.
  * \author Giorgio Wicklein - GIOWISYS Software
  * \author Trolltech ASA <info@trolltech.com>
  * \date 18/05/2012
  */

/*
 * Original code from
 * Copyright (c) 2007 Trolltech ASA <info@trolltech.com>
 *
 * Use, modification and distribution is allowed without limitation,
 * warranty, liability or support of any kind.
 *
 * Modified by
 * Copyright (c) 2012 Giorgio Wicklein <giowckln@gmail.com>
 */

#ifndef SEARCHLINEEDIT_H
#define SEARCHLINEEDIT_H


//-----------------------------------------------------------------------------
// Headers
//-----------------------------------------------------------------------------

#include <QtWidgets/QLineEdit>


//-----------------------------------------------------------------------------
// Forward declarations
//-----------------------------------------------------------------------------

class QToolButton;


//-----------------------------------------------------------------------------
// SearchLineEdit
//-----------------------------------------------------------------------------

class SearchLineEdit : public QLineEdit
{
    Q_OBJECT

public:
    SearchLineEdit(QWidget *parent = 0);

protected:
    void resizeEvent(QResizeEvent *);

private slots:
    void updateCloseButton(const QString &text);

private:
    QToolButton *m_clearButton;
    QToolButton *m_searchButton;
};

#endif // SEARCHLIENEDIT_H
