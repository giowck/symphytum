/*
 *  Copyright (C) 2012 Giorgio Wicklein <giorgio.wicklein@giowisys.com>
 */

//-----------------------------------------------------------------------------
// Hearders
//-----------------------------------------------------------------------------

#include "maclionfullscreenprovider.h"

#include <QtGui/QMainWindow>


//-----------------------------------------------------------------------------
// Public
//-----------------------------------------------------------------------------

void MacLionFullscreenProvider::enableWindow(QMainWindow *window)
{
#if MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_7
    NSView *nsview = (NSView *) window->winId();
    NSWindow *nswindow = [nsview window];
    [nswindow setCollectionBehavior:NSWindowCollectionBehaviorFullScreenPrimary];
#endif
}

void MacLionFullscreenProvider::toggleFullscreen(QMainWindow *window)
{
#if MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_7
    NSView *nsview = (NSView *) window->winId();
    NSWindow *nswindow = [nsview window];
    [nswindow toggleFullScreen:nil];
#endif
}

bool MacLionFullscreenProvider::isFullScreen(QMainWindow *window)
{
    bool result;
#if MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_7
    NSView *nsview = (NSView *) window->winId();
    NSWindow *nswindow = [nsview window];
    NSUInteger masks = [nswindow styleMask];
    result = masks & NSFullScreenWindowMask;
#endif
    return result;
}
