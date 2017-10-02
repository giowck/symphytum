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

//-----------------------------------------------------------------------------
// Hearders
//-----------------------------------------------------------------------------

#include "searchlineedit.h"
#include "../utils/platformcolorservice.h"

#include <QtWidgets/QToolButton>
#include <QtWidgets/QStyle>


//-----------------------------------------------------------------------------
// Public
//-----------------------------------------------------------------------------

SearchLineEdit::SearchLineEdit(QWidget *parent)
    : QLineEdit(parent)
{
    //set up search button
    m_searchButton = new QToolButton(this);
    QPixmap p1(":/images/icons/searchline_icon.png");
    m_searchButton->setIcon(QIcon(p1));
    m_searchButton->setIconSize(p1.size());
    m_searchButton->setCursor(Qt::ArrowCursor);
    m_searchButton->setStyleSheet("QToolButton { border: none; padding: 0px; }");
    m_searchButton->setToolTip(tr("Search"));

    //if a search option menu is needed:
    //add a down arrow to the search icon, to indicate a menu
    //uncomment and set this up
    //m_searchButton->setPopupMode(QToolButton::InstantPopup);
    //m_searchButton->setMenu();

    //set up clear button
    m_clearButton = new QToolButton(this);
    QPixmap p2(":/images/icons/searchline_clear.png");
    m_clearButton->setIcon(QIcon(p2));
    m_clearButton->setIconSize(p2.size());
    m_clearButton->setCursor(Qt::ArrowCursor);
    m_clearButton->setStyleSheet("QToolButton { border: none; padding: 0px; }");
    m_clearButton->setToolTip(tr("Clears the text"));
    m_clearButton->hide();

    //style sheet
    QColor c = PlatformColorService::getHighlightColor();
    int frameWidth = style()->pixelMetric(QStyle::PM_DefaultFrameWidth);

    int borderRadius;
#if defined(Q_OS_LINUX)
    borderRadius = 11; //ideal everywhere is 12px, only kde's oxygen theme requires 11px
#elif defined(Q_OS_WIN)
    borderRadius = 11;
#elif defined(Q_OS_OSX)
    borderRadius = 11;
#endif

    setStyleSheet(QString(
                      "QLineEdit { padding-right: %1px; "
                      "padding-left: %2px;"
                      "border-radius: %3px;"
                      "border: 1px solid gray; }"
                      "QLineEdit:focus { "
                      "border: 2px solid rgba(%4, %5, %6, 80%); }"
                      ).arg(m_clearButton->sizeHint().width() + frameWidth + 1)
                       .arg(m_searchButton->sizeHint().width() + frameWidth + 1)
                       .arg(borderRadius)
                       .arg(c.red()).arg(c.green()).arg(c.blue()));

    //size setup
    QSize msz = minimumSizeHint();
    setMinimumSize(qMax(msz.width(), m_clearButton->sizeHint().height() + frameWidth * 2 + 2),
                   qMax(msz.height(), m_clearButton->sizeHint().height() + frameWidth * 2 + 2));
    setMaximumWidth(250);

    //placeholder
    setPlaceholderText(tr("Search"));

    //on mac disable focus rect around rounded borders
    setAttribute(Qt::WA_MacShowFocusRect, 0);

    //connections
    connect(m_clearButton, SIGNAL(clicked()), this, SLOT(clear()));
    connect(this, SIGNAL(textChanged(const QString&)),
            this, SLOT(updateCloseButton(const QString&)));
}


//-----------------------------------------------------------------------------
// Protected
//-----------------------------------------------------------------------------

void SearchLineEdit::resizeEvent(QResizeEvent *)
{
    int frameWidth = style()->pixelMetric(QStyle::PM_DefaultFrameWidth);

    QSize szSearchButton = m_searchButton->sizeHint();
    m_searchButton->move(rect().left() + frameWidth + 2,
                      (rect().bottom() + 2 - szSearchButton.height())/2);

    QSize szClearButton = m_clearButton->sizeHint();
    m_clearButton->move(rect().right() - frameWidth - szClearButton.width(),
                      (rect().bottom() + 2 - szClearButton.height())/2);
}


//-----------------------------------------------------------------------------
// Private slots
//-----------------------------------------------------------------------------

void SearchLineEdit::updateCloseButton(const QString& text)
{
    m_clearButton->setVisible(!text.isEmpty());
}

