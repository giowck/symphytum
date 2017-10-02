/*
 *  Copyright (c) 2012 Giorgio Wicklein <giowckln@gmail.com>
 */

//-----------------------------------------------------------------------------
// Hearders
//-----------------------------------------------------------------------------

#include "syncsession.h"


//-----------------------------------------------------------------------------
// Static variables initialization
//-----------------------------------------------------------------------------

bool SyncSession::IS_ENABLED = false;
bool SyncSession::IS_ONLINE = false;
bool SyncSession::IS_READ_ONLY = false;
bool SyncSession::LOCAL_DATA_CHANGED = false;
SyncSession::SessionState SyncSession::CURRENT_STATE = SyncSession::NoOperation;
