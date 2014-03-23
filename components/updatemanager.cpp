/*
 *  Copyright (c) 2011  Giorgio Wicklein <g.wicklein@giowisys.com>
 */

//-----------------------------------------------------------------------------
// Hearders
//-----------------------------------------------------------------------------

#include "updatemanager.h"
#include "../utils/definitionholder.h"

#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkReply>
#include <QtCore/QStringList>
#include <QtGui/QMessageBox>
#include <QtGui/QApplication>
#include <QtGui/QDesktopServices>


//-----------------------------------------------------------------------------
// Public
//-----------------------------------------------------------------------------

UpdateManager::UpdateManager(QObject *parent) :
    QObject(parent)
{
    m_accessManager = new QNetworkAccessManager(this);

    connect(m_accessManager, SIGNAL(finished(QNetworkReply*)),
            this, SLOT(updateResponseSlot(QNetworkReply*)));
}

UpdateManager::~UpdateManager()
{
}

void UpdateManager::checkForUpdates()
{
    m_accessManager->get(QNetworkRequest(QUrl(DefinitionHolder::UPDATE_URL)));
}


//-----------------------------------------------------------------------------
// Private slots
//-----------------------------------------------------------------------------

void UpdateManager::updateResponseSlot(QNetworkReply *reply)
{
    QString tmp(reply->readAll());

    if (tmp.isEmpty()) {
        emit updateErrorSignal();
        return;
    }

    QStringList s = tmp.split(";", QString::SkipEmptyParts);

    if (s.size() < 1) { //software_build;
        emit updateErrorSignal();
        return;
    }

    bool softwareUpdate;
    bool a;
    int softwareBuild = s.at(0).toInt(&a);

    if (!a) {
        emit updateErrorSignal();
        return;
    }

    softwareUpdate = softwareBuild > DefinitionHolder::SOFTWARE_BUILD;

    if (softwareUpdate) {
        int r = QMessageBox::question(0, tr("New Software Version"),
                                      tr("A new version of %1 is available!<br>"
                                         "Do you want to download the new version?")
                                      .arg(DefinitionHolder::NAME),
                                      QMessageBox::Yes | QMessageBox::No,
                                      QMessageBox::Yes);

        if (r == QMessageBox::Yes) {
            QUrl downloadUrl(DefinitionHolder::DOWNLOAD_URL);
            QDesktopServices::openUrl(downloadUrl);
            emit updatesAccepted();
        }

    } else {
        //no updates available
        emit noUpdateSignal();
    }

    //delete reply
    reply->deleteLater();
}


//-----------------------------------------------------------------------------
// Private
//-----------------------------------------------------------------------------
