/**
  * \class SyncStatusWidget
  * \brief This is widget is added to the dock under the collection
  *        list view and displays status info about the sync service.
  * \author Giorgio Wicklein - GIOWISYS Software
  * \date 28/08/2012
  */

#ifndef SYNCSTATUSWIDGET_H
#define SYNCSTATUSWIDGET_H


//-----------------------------------------------------------------------------
// Headers
//-----------------------------------------------------------------------------

#include <QtWidgets/QWidget>


//-----------------------------------------------------------------------------
// Forward declarations
//-----------------------------------------------------------------------------

class QLabel;
class QVBoxLayout;
class QHBoxLayout;
class QProgressBar;


//-----------------------------------------------------------------------------
// SyncStatusWidget
//-----------------------------------------------------------------------------

class SyncStatusWidget : public QWidget
{
    Q_OBJECT

public:
    explicit SyncStatusWidget(QWidget *parent = nullptr);
    ~SyncStatusWidget();

public slots:
    /** Update display to show current status based on SyncSession */
    void updateStatus();
    
private:
    void init();
    void createConnections();

    QVBoxLayout *m_mainLayout;
    QLabel *m_syncServiceLabel;
    QLabel *m_syncStatusLabel;
    QHBoxLayout *m_syncStatusLayout;
    QLabel *m_syncOperation;
    QProgressBar *m_progressBar;
};

#endif // SYNCSTATUSWIDGET_H
