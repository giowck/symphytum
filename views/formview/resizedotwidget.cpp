/*
 *  Copyright (c) 2012 Giorgio Wicklein <giowckln@gmail.com>
 */

//-----------------------------------------------------------------------------
// Hearders
//-----------------------------------------------------------------------------

#include "resizedotwidget.h"
#include "../../utils/platformcolorservice.h"

#include <QtWidgets/QLabel>
#include <QtWidgets/QVBoxLayout>


//-----------------------------------------------------------------------------
// Public
//-----------------------------------------------------------------------------

ResizeDotWidget::ResizeDotWidget(QWidget *parent) :
    QWidget(parent), isResizing(false)
{
    setContentsMargins(0, 0, 0, 0);
    label = new QLabel(this);
    layout = new QVBoxLayout(this);
    layout->setMargin(0);

    layout->addWidget(label);
    setLayout(layout);

    QColor c = PlatformColorService::getHighlightColor();

    QString style(
                "border: 1px solid black;"
                "border-radius: 2px;"
                "background-color: rgba(%4, %5, %6, 50%);"
                );
    style = style.arg(c.red()).arg(c.green()).arg(c.blue());

    setStyleSheet(style);
}
