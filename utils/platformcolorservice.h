/**
  * \class PlatformColorService
  * \brief This class provides easy static methods to get default colors
  *        of the current platform for different roles.
  * \author Giorgio Wicklein - GIOWISYS Software
  * \date 29/06/2012
  */

#ifndef PLATFORMCOLORSERVICE_H
#define PLATFORMCOLORSERVICE_H


//-----------------------------------------------------------------------------
// Forward declarations
//-----------------------------------------------------------------------------

class QColor;


//-----------------------------------------------------------------------------
// PlatformColorService
//-----------------------------------------------------------------------------

class PlatformColorService
{
public:
    static QColor getHighlightColor();

private:
    PlatformColorService() {} //static only
};

#endif // PLATFORMCOLORSERVICE_H
