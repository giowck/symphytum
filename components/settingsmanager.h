/**
  * \class SettingsManager
  * \brief The settings manager is used to handle
  *        saving and restoring of local application settings,
  *        such as widget geometries, app preferences and more.
  * \author Giorgio Wicklein - GIOWISYS Software
  * \date 09/06/2012
  */

#ifndef SETTINGSMANAGER_H
#define SETTINGSMANAGER_H


//-----------------------------------------------------------------------------
// Headers
//-----------------------------------------------------------------------------

#include <QtCore/QtGlobal>
#include <QtCore/QHash>


//-----------------------------------------------------------------------------
// Forward declarations
//-----------------------------------------------------------------------------

class QString;
class QStringList;
class QByteArray;
class QSettings;
class QVariant;
class QDate;
class QDateTime;


//-----------------------------------------------------------------------------
// SettingsManager
//-----------------------------------------------------------------------------

class SettingsManager
{
public:
    SettingsManager();
    ~SettingsManager();

    /** Save the geometry property for the given object name */
    void saveGeometry(const QString& objectName, const QByteArray& geometry);

    /** Restore the geometry property for the specified object */
    QByteArray restoreGeometry(const QString& objectName) const;

    /**
      * Save the state property (dock, toolbars, etc...)
      * for the given object name
      */
    void saveState(const QString& objectName, const QByteArray& state);

    /** Restore the state property for the specified object */
    QByteArray restoreState(const QString& objectName) const;

    /** Save the state of a property
      * @param propertyName - the key name
      * @param objectName - the object that holds the property
      * @param value - the value to store
      */
    void saveProperty(const QString& propertyName, const QString& objectName,
                      const QVariant& value);

    /** Restore the state of a property
      * @param propertyName - the key name
      * @param objectName - the object that holds the property
      * @return the stored value
      */
    QVariant restoreProperty(const QString& propertyName,
                             const QString& objectName) const;

    /** Delete all saved properties */
    void removeAllSettings();

    /** Delete group/object including all keys/properties */
    void deleteObjectProperties(const QString &objectName);

    /** Store the current software version (build) */
    void saveSoftwareBuild();

    /** Retrive the last saved software build (version) */
    int restoreSoftwareBuild() const;

    /** Save the specified int as MainWindow's view mode */
    void saveViewMode(int mode);

    /** Restore MainWindow's view mode */
    int restoreViewMode() const;

    /** Save last used record (row) of current collection */
    void saveLastUsedRecord(int row);

    /** Restore last used record (row) of current collection */
    int restoreLastUsedRecord();

    /** Return whether a cloud sync service has been configured and is active */
    bool isCloudSyncActive();

    /** Activate/deactivate current configured cloud sync service */
    void setCloudSyncActive(bool a);

    /** Save access token (in encoded form) for sync service */
    void saveEncodedAccessToken(const QString &token);

    /** Restore access token (in encoded form) for sync service */
    QString restoreEncodedAccessToken();

    /** Save the id of the current configured cloud sync service */
    void saveCurrentCloudSyncService(int id);

    /** Restore the id of the current configured cloud sync service */
    int restoreCurrentCloudSyncService();

    /** Set boolean for first time sync, true means that no sync ever happened */
    void setCloudSyncFirstTime(bool b);

    /** Return whether the first sync was already done (false) or not (true) */
    bool isCloudSyncFirstTime();

    /** Set boolean if cloud has already been initialized from this client */
    void setCloudSyncInitialized(bool b);

    /** Return whether the cloud has already been initialized from this client */
    bool isCloudSyncInitialized();

    /** Save the state of local data change since last sync */
    void saveCloudLocalDataChanged(bool b);

    /** Restore local modification state since last sync */
    bool restoreCloudLocalDataChanged();

    /** Save cloud sync revision */
    void saveCloudSyncRevision(quint64 revision);

    /** Restore cloud sync revision */
    quint64 restoreCloudSyncRevision();

    /** Save the specified cloud sync session key */
    void saveCloudSessionKey(const QString &sessionKey);

    /** Restore the cloud sync session key */
    QString restoreCloudSessionKey();

    /** Save state on check updates at startup */
    void saveCheckUpdates(bool b);

    /** Check updates automatically at startup */
    bool restoreCheckUpdates();

    /** Save list of files to upload */
    void saveToUploadList(const QStringList &list);

    /** Restore list of files to upload */
    QStringList restoreToUploadList();

    /** Save list of files to delete */
    void saveToDeleteList(const QStringList &list);

    /** Restore list of files to delete */
    QStringList restoreToDeleteList();

    /** Save list of files to watch/check for changes */
    void saveToWatchList(const QHash<QString,QDateTime> &map);

    /** Restore list of files to watch/check for changes */
    QHash<QString,QDateTime> restoreToWatchList();

private:
    QSettings *m_settings;
};

#endif // SETTINGSMANAGER_H
