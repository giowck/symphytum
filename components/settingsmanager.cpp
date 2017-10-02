/*
 *  Copyright (c) 2012 Giorgio Wicklein <giowckln@gmail.com>
 */

//-----------------------------------------------------------------------------
// Hearders
//-----------------------------------------------------------------------------

#include "settingsmanager.h"
#include "../utils/definitionholder.h"

#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QByteArray>
#include <QtCore/QSettings>
#include <QtCore/QVariant>
#include <QtCore/QDate>


//-----------------------------------------------------------------------------
// Public
//-----------------------------------------------------------------------------

SettingsManager::SettingsManager()
{
    m_settings = new QSettings();
}

SettingsManager::~SettingsManager()
{
    delete m_settings;
}

void SettingsManager::saveGeometry(const QString &objectName, const QByteArray &geometry)
{
    m_settings->beginGroup(objectName);
    m_settings->setValue("geometry", geometry);
    m_settings->endGroup();
}

QByteArray SettingsManager::restoreGeometry(const QString &objectName) const
{
    QByteArray g;

    m_settings->beginGroup(objectName);
    g = m_settings->value("geometry").toByteArray();
    m_settings->endGroup();

    return g;
}

void SettingsManager::saveState(const QString &objectName, const QByteArray &state)
{
    m_settings->beginGroup(objectName);
    m_settings->setValue("state", state);
    m_settings->endGroup();
}

QByteArray SettingsManager::restoreState(const QString &objectName) const
{
    QByteArray s;

    m_settings->beginGroup(objectName);
    s = m_settings->value("state").toByteArray();
    m_settings->endGroup();

    return s;
}

void SettingsManager::saveProperty(const QString &propertyName, const QString &objectName,
                                   const QVariant &value)
{
    m_settings->beginGroup(objectName);
    m_settings->setValue(propertyName, value);
    m_settings->endGroup();
}


QVariant SettingsManager::restoreProperty(const QString &propertyName, const QString &objectName) const
{
    QVariant v;

    m_settings->beginGroup(objectName);
    v = m_settings->value(propertyName);
    m_settings->endGroup();

    return v;
}

void SettingsManager::removeAllSettings()
{
    m_settings->clear();
}

void SettingsManager::deleteObjectProperties(const QString &objectName)
{
    m_settings->remove(objectName);
}

void SettingsManager::saveSoftwareBuild()
{
    m_settings->beginGroup(DefinitionHolder::NAME.toLower());
    m_settings->setValue("build", DefinitionHolder::SOFTWARE_BUILD);
    m_settings->endGroup();
}

int SettingsManager::restoreSoftwareBuild() const
{
    int i;

    m_settings->beginGroup(DefinitionHolder::NAME.toLower());
    i = m_settings->value("build", 0).toInt(); //0 means no previous version installed
    m_settings->endGroup();

    return i;
}

void SettingsManager::saveViewMode(int mode)
{
    m_settings->beginGroup("mainWindow");
    m_settings->setValue("viewMode", mode);
    m_settings->endGroup();
}

int SettingsManager::restoreViewMode() const
{
    int i;

    m_settings->beginGroup("mainWindow");
    i = m_settings->value("viewMode", 0).toInt(); //0 is form view
    m_settings->endGroup();

    return i;
}

void SettingsManager::saveLastUsedRecord(int row)
{
    m_settings->beginGroup("mainWindow");
    m_settings->setValue("lastUsedRecord", row);
    m_settings->endGroup();
}

int SettingsManager::restoreLastUsedRecord()
{
    int i;

    m_settings->beginGroup("mainWindow");
    i = m_settings->value("lastUsedRecord", -1).toInt(); //-1 means invalid
    m_settings->endGroup();

    return i;
}

bool SettingsManager::isCloudSyncActive()
{
    bool a;

    m_settings->beginGroup("cloudSync");
    a = m_settings->value("cloudSyncActive", false).toBool();
    m_settings->endGroup();

    return a;
}

void SettingsManager::setCloudSyncActive(bool a)
{
    m_settings->beginGroup("cloudSync");
    m_settings->setValue("cloudSyncActive", a);
    m_settings->endGroup();
}

void SettingsManager::saveEncodedAccessToken(const QString &token)
{
    m_settings->beginGroup("cloudSync");
    m_settings->setValue("axTk", token);
    m_settings->endGroup();
}

QString SettingsManager::restoreEncodedAccessToken()
{
    QString t;

    m_settings->beginGroup("cloudSync");
    t = m_settings->value("axTk", "").toString();
    m_settings->endGroup();

    return t;
}

void SettingsManager::saveCurrentCloudSyncService(int id)
{
    m_settings->beginGroup("cloudSync");
    m_settings->setValue("service", id);
    m_settings->endGroup();
}

int SettingsManager::restoreCurrentCloudSyncService()
{
    int id;

    m_settings->beginGroup("cloudSync");
    id = m_settings->value("service", -1).toInt();
    m_settings->endGroup();

    return id;
}

void SettingsManager::setCloudSyncFirstTime(bool b)
{
    m_settings->beginGroup("cloudSync");
    m_settings->setValue("cloudSyncFirstTime", b);
    m_settings->endGroup();
}

bool SettingsManager::isCloudSyncFirstTime()
{
    bool b;

    m_settings->beginGroup("cloudSync");
    b = m_settings->value("cloudSyncFirstTime", true).toBool();
    m_settings->endGroup();

    return b;
}

void SettingsManager::setCloudSyncInitialized(bool b)
{
    m_settings->beginGroup("cloudSync");
    m_settings->setValue("cloudSyncInit", b);
    m_settings->endGroup();
}

bool SettingsManager::isCloudSyncInitialized()
{
    bool b;

    m_settings->beginGroup("cloudSync");
    b = m_settings->value("cloudSyncInit", false).toBool();
    m_settings->endGroup();

    return b;
}

void SettingsManager::saveCloudLocalDataChanged(bool b)
{
    m_settings->beginGroup("cloudSync");
    m_settings->setValue("localDataChanged", b);
    m_settings->endGroup();
}

bool SettingsManager::restoreCloudLocalDataChanged()
{
    bool b;

    m_settings->beginGroup("cloudSync");
    b = m_settings->value("localDataChanged", false).toBool();
    m_settings->endGroup();

    return b;
}

void SettingsManager::saveCloudSyncRevision(quint64 revision)
{
    m_settings->beginGroup("cloudSync");
    m_settings->setValue("revision", revision);
    m_settings->endGroup();
}

quint64 SettingsManager::restoreCloudSyncRevision()
{
    quint64 r;

    m_settings->beginGroup("cloudSync");
    r = m_settings->value("revision", 0).toULongLong();
    m_settings->endGroup();

    return r;
}

void SettingsManager::saveCloudSessionKey(const QString &sessionKey)
{
    //SM has that private method for ding it, on sync deactivation
    m_settings->beginGroup("cloudSync");
    m_settings->setValue("sessionKey", sessionKey);
    m_settings->endGroup();
}

QString SettingsManager::restoreCloudSessionKey()
{
    QString k;

    m_settings->beginGroup("cloudSync");
    k = m_settings->value("sessionKey", "invalid").toString();
    m_settings->endGroup();

    return k;
}

void SettingsManager::saveCheckUpdates(bool b)
{
    m_settings->beginGroup(DefinitionHolder::NAME.toLower());
    m_settings->setValue("checkUpdates", b);
    m_settings->endGroup();
}

bool SettingsManager::restoreCheckUpdates()
{
    bool b;

    m_settings->beginGroup(DefinitionHolder::NAME.toLower());
    b = m_settings->value("checkUpdates", true).toBool();
    m_settings->endGroup();

    return b;
}

void SettingsManager::saveToUploadList(const QStringList &list)
{
    m_settings->beginGroup("cloudSync");
    m_settings->setValue("toUpload", list);
    m_settings->endGroup();
}

QStringList SettingsManager::restoreToUploadList()
{
    QStringList s;

    m_settings->beginGroup("cloudSync");
    s = m_settings->value("toUpload").toStringList();
    m_settings->endGroup();

    return s;
}

void SettingsManager::saveToDeleteList(const QStringList &list)
{
    m_settings->beginGroup("cloudSync");
    m_settings->setValue("toDelete", list);
    m_settings->endGroup();
}

QStringList SettingsManager::restoreToDeleteList()
{
    QStringList s;

    m_settings->beginGroup("cloudSync");
    s = m_settings->value("toDelete").toStringList();
    m_settings->endGroup();

    return s;
}

void SettingsManager::saveToWatchList(const QHash<QString, QDateTime> &map)
{
    QList<QVariant> keys;
    QList<QVariant> values;

    QList<QString> t_keys = map.keys();
    for (int i = 0; i < map.size(); i++) {
        keys.append(t_keys.at(i));
        values.append(map.value(t_keys.at(i)));
    }

    m_settings->beginGroup("cloudSync");
    m_settings->setValue("toWatchKeys", keys);
    m_settings->setValue("toWatchValues", values);
    m_settings->endGroup();
}

QHash<QString, QDateTime> SettingsManager::restoreToWatchList()
{
    QHash<QString,QDateTime> map;
    QList<QVariant> keys;
    QList<QVariant> values;

    m_settings->beginGroup("cloudSync");
    keys = m_settings->value("toWatchKeys").toList();
    values = m_settings->value("toWatchValues").toList();
    m_settings->endGroup();

    for (int i = 0; i < keys.size(); i++) {
        map.insert(keys.at(i).toString(), values.at(i).toDateTime());
    }

    return map;
}


//-----------------------------------------------------------------------------
// Private
//-----------------------------------------------------------------------------
