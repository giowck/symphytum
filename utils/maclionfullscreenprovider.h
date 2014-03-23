/**
  * \class MacLionFullscreenProvider
  * \brief This static class handles MacOS X Lion specific tweaks
  *        in order to enable fullscreen through the arrow button
  *        in the upper right corner of a window. This class is needed
  *        until Qt 5 is released, where the native fullscreen API will be
  *        implemented.
  * \author Giorgio Wicklein - GIOWISYS Software
  * \date 13/06/2012
  */

#ifndef MACLIONFULLSCREENPROVIDER_H
#define MACLIONFULLSCREENPROVIDER_H


//-----------------------------------------------------------------------------
// Headers
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Forward declarations
//-----------------------------------------------------------------------------

class QMainWindow;


//-----------------------------------------------------------------------------
// MacLionFullscreenProvider
//-----------------------------------------------------------------------------

class MacLionFullscreenProvider
{
public:
    /** Enable fullscreen button in the right upper corner for the specified window */
    static void enableWindow(QMainWindow *window);

    /** Toggle fullscreen mode */
    static void toggleFullscreen(QMainWindow *window);

    /** Return whether fullscreen is active or not */
    static bool isFullScreen(QMainWindow *window);

private:
    MacLionFullscreenProvider() {} //static only
};

#endif // MACLIONFULLSCREENPROVIDER_H
