/*
 *  Copyright (c) 2012 Giorgio Wicklein <giowckln@gmail.com>
 */

//-----------------------------------------------------------------------------
// Hearders
//-----------------------------------------------------------------------------

#include "droprectwidget.h"
#include "../../utils/platformcolorservice.h"

#include <QtWidgets/QLabel>
#include <QtWidgets/QVBoxLayout>


//-----------------------------------------------------------------------------
// Public
//-----------------------------------------------------------------------------

DropRectWidget::DropRectWidget(QWidget *parent) :
    QWidget(parent)
{
    label = new QLabel(this);
    layout = new QVBoxLayout(this);

    layout->addWidget(label);
    setLayout(layout);

    QColor c = PlatformColorService::getHighlightColor();

    QString style(
                "border: 2px solid rgb(%1, %2, %3);"
                "border-radius: 5px;"
                "background-color: rgba(%4, %5, %6, 20%);"
                );
    style = style.arg(c.red()).arg(c.green()).arg(c.blue())
            .arg(c.red()).arg(c.green()).arg(c.blue());

    setStyleSheet(style);
}
