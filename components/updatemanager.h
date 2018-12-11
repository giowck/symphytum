/**
  * \class UpdateManager
  * \brief This class handles software updates.
  * \author Giorgio Wicklein - GIOWISYS Software
  * \date 13/10/2012
  */

#ifndef UPDATEMANAGER_H
#define UPDATEMANAGER_H

//-----------------------------------------------------------------------------
// Headers
//-----------------------------------------------------------------------------

#include <QtCore/QObject>


//-----------------------------------------------------------------------------
// Forward declarations
//-----------------------------------------------------------------------------

class QNetworkAccessManager;
class QNetworkReply;


//-----------------------------------------------------------------------------
// UpdateManager
//-----------------------------------------------------------------------------

class UpdateManager : public QObject
{
    Q_OBJECT

public:
    UpdateManager(QObject *parent = nullptr);
    ~UpdateManager();

    /** Start checking for updates */
    void checkForUpdates();

signals:
    void noUpdateSignal();
    void updateErrorSignal();
    void updatesAccepted();

private slots:
    void updateResponseSlot(QNetworkReply*);

private:
    QNetworkAccessManager *m_accessManager;
};

#endif // UPDATEMANAGER_H
