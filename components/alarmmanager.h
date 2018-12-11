/**
  * \class AlarmManager
  * \brief This class handles all operation related to alarms/reminders
  *        of date field types such as checking, showing and more.
  *        Reminders can be added/removed or updated.
  * \author Giorgio Wicklein - GIOWISYS Software
  * \date 05/11/2012
  */

#ifndef ALARMMANAGER_H
#define ALARMMANAGER_H


//-----------------------------------------------------------------------------
// Headers
//-----------------------------------------------------------------------------

#include <QtCore/QObject>
#include <QtCore/QDateTime>


//-----------------------------------------------------------------------------
// Forward declarations
//-----------------------------------------------------------------------------

class MetadataEngine;


//-----------------------------------------------------------------------------
// AlarmManager
//-----------------------------------------------------------------------------

class AlarmManager : public QObject
{
    Q_OBJECT

public:
    /** Alarm definition */
    typedef struct {
        int alarmId;
        int alarmCollectionId;
        int alarmFieldId;
        int alarmRecordId;
        QDateTime alarmDateTime;
    } Alarm;

    explicit AlarmManager(QObject *parent = nullptr);

    /**
     * @brief Insert or update (if already registered)
     *        an alarm/reminder entry.
     *        An invalid (old) alarm is removed
     *        if the alarm was already present in table.
     * @param collectionId - the collection id
     * @param fieldId - the id of the date type field
     * @param recordId - the id of the record (_id)
     * @param dateTime - the new trigger date and time
     */
    void addOrUpdateAlarm(int collectionId,
                          int fieldId,
                          int recordId,
                          QDateTime &dateTime);

    /**
     * @brief Add all existing future dates of the specified field id
     * @param collectionId - the collection id
     * @param fieldId - the field id
     */
    void addAlarmsForExistingRecords(int collectionId, int fieldId);

    /**
     * @brief Remove all alarm triggers for the specified field id
     * @param collectionId - the id of the collection
     * @param fieldId - the id of the date type field
     */
    void removeAllAlarms(int collectionId, int fieldId);

    /**
     * @brief Remove the specified alarm
     * @param alarmId - the id of the alarm (_id)
     */
    void removeAlarm(int alarmId);

    /**
     * @brief Check alarms for trigger.
     *        Removes invalid alarms.
     * @return whether there are alarms that are ready to trigger
     */
    bool checkAlarms();

    /**
     * @brief Get all alarms that are ready to trigger
     * @return a list of alarms
     */
    QList<Alarm> getAllTriggeredAlarms();

    /** Get specified alarm */
    Alarm getAlarm(const int alarmId) const;

    /**
     * @brief Get all alarms
     * @param collectionId - optional filter for colelction id, if -1 all alarms are returned
     * @return list of all alarms, optinally restricted by collection id
     */
    QList<Alarm> getAllAlarms(const int collectionId = -1);

private:

    MetadataEngine *m_metadatEngine;
};

#endif // ALARMMANAGER_H
