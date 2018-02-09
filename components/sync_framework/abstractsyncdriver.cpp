/*
 *  Copyright (c) 2012 Giorgio Wicklein <giowckln@gmail.com>
 */

//-----------------------------------------------------------------------------
// Hearders
//-----------------------------------------------------------------------------

#include "abstractsyncdriver.h"


//-----------------------------------------------------------------------------
// Public
//-----------------------------------------------------------------------------

AbstractSyncDriver::AbstractSyncDriver(QObject *parent) :
    QObject(parent)
{
}

void AbstractSyncDriver::startAuthenticationRequest()
{
    QStringList empty;
    startAuthenticationRequest(empty);
}

AbstractSyncDriver::~AbstractSyncDriver()
{

}
