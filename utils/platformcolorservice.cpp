/*
 *  Copyright (c) 2012 Giorgio Wicklein <giowckln@gmail.com>
 */

//-----------------------------------------------------------------------------
// Hearders
//-----------------------------------------------------------------------------

#include "platformcolorservice.h"

#include <QtGui/QPalette>
#include <QtGui/QBrush>
#include <QtGui/QColor>


//-----------------------------------------------------------------------------
// Public
//-----------------------------------------------------------------------------

QColor PlatformColorService::getHighlightColor()
{
    QPalette p;
    QBrush b = p.highlight();
    QColor c = b.color();
    return c;
}
