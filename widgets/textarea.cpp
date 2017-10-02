/*
 *  Copyright (c) 2012 Giorgio Wicklein <giowckln@gmail.com>
 */

//-----------------------------------------------------------------------------
// Hearders
//-----------------------------------------------------------------------------

#include "textarea.h"

#include <QtGui/QKeyEvent>
#include <QtWidgets/QApplication>


//-----------------------------------------------------------------------------
// Public
//-----------------------------------------------------------------------------

TextArea::TextArea(QWidget *parent) :
    QTextEdit(parent)
{
    setAcceptRichText(false);
    setTabChangesFocus(true);
}


//-----------------------------------------------------------------------------
// Protected
//-----------------------------------------------------------------------------

void TextArea::focusOutEvent(QFocusEvent *e)
{
    QTextEdit::focusOutEvent(e);

    //handle editingFinished signal
    bool leaveEditing = true;

    //if focus is passed to a child widget or context menu (popup widget)
    //dont emit editing finished signal
    if (e->reason() == Qt::PopupFocusReason) {
        QWidget *p = QApplication::activePopupWidget();
        if (p && isAncestorOf(p->parentWidget()))
            leaveEditing = false;
    } else {
        QWidget *f = QApplication::focusWidget();
        if (isAncestorOf(f)) {
            leaveEditing = false;
        }
    }

    if (leaveEditing)
        emit editingFinished();
}

void TextArea::focusInEvent(QFocusEvent *e)
{
    QTextEdit::focusInEvent(e);

    //simulate tab focus like QLineEdit
    if (e->reason() == Qt::TabFocusReason)
        selectAll();
}

void TextArea::keyPressEvent(QKeyEvent *e)
{
    //commented out because support for new lines in text area
    //is not bad at all :P
//    if (e->key() == Qt::Key_Return) {
//        emit editingFinished();
//    } else {
//        QTextEdit::keyPressEvent(e);
//    }

        QTextEdit::keyPressEvent(e);
}
