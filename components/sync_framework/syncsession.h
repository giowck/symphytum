/**
  * \class SyncSession
  * \brief This static class represents the sync session with its state.
  * \author Giorgio Wicklein - GIOWISYS Software
  * \date 28/08/2012
  */

#ifndef SYNCSESSION_H
#define SYNCSESSION_H


//-----------------------------------------------------------------------------
// Forward declarations
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// SyncSession
//-----------------------------------------------------------------------------

class SyncSession
{
public:
    /** Enum of session states */
    enum SessionState {
        NoOperation,    /**< No operation is going on */
        Authenticating, /**< Authenticating on the cloud service */
        OpeningSession, /**< Opening a sync session to cloud */
        ClosingSession, /**< Closing a cloud sync session */
        Accessing,      /**< Accessing metafile on cloud service */
        Uploading,      /**< Uploading files */
        Downloading     /**< Downloading files */
    };

    static bool IS_ENABLED; /**< Whether sync is enabled */
    static bool IS_ONLINE; /**< Whether user is online or nor */
    static bool IS_READ_ONLY; /**< Whether read-only mode is active or not */
    static bool LOCAL_DATA_CHANGED; /**< Whether local data has been modified */
    static SessionState CURRENT_STATE; /** Current state of the session */

private:
    SyncSession() {} //static only
};

#endif // SYNCSESSION_H
