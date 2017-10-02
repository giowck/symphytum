/*
 *  Copyright (c) 2012 Giorgio Wicklein <giowckln@gmail.com>
 */

//-----------------------------------------------------------------------------
// Hearders
//-----------------------------------------------------------------------------

#include "syncstatuswidget.h"
#include "../components/sync_framework/syncsession.h"

#include <QtWidgets/QLabel>
#include <QtWidgets/QProgressBar>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QHBoxLayout>


//-----------------------------------------------------------------------------
// Public
//-----------------------------------------------------------------------------

SyncStatusWidget::SyncStatusWidget(QWidget *parent) :
    QWidget(parent)
{
    init();
    createConnections();
    updateStatus();
}

SyncStatusWidget::~SyncStatusWidget()
{
}


//-----------------------------------------------------------------------------
// Public slots
//-----------------------------------------------------------------------------

void SyncStatusWidget::updateStatus()
{
    if (SyncSession::IS_READ_ONLY) {
        m_syncStatusLabel->setText(tr("read-only"));
        m_syncStatusLabel->setStyleSheet("color: #F88017;");
    } else if (SyncSession::IS_ONLINE) {
        m_syncStatusLabel->setText(tr("online"));
        m_syncStatusLabel->setStyleSheet("color: #41A317;");
    } else if (!SyncSession::IS_ONLINE) {
        m_syncStatusLabel->setText(tr("offline"));
        m_syncStatusLabel->setStyleSheet("color: #C11B17;");
    } else {
        m_syncStatusLabel->setText(tr("undefined"));
        m_syncStatusLabel->setStyleSheet("");
    }

    switch (SyncSession::CURRENT_STATE) {
    case SyncSession::NoOperation:
        m_syncOperation->hide();
        m_progressBar->hide();
        break;
    case SyncSession::Authenticating:
        m_syncOperation->show();
        m_progressBar->show();
        m_syncOperation->setText(tr("Authenticating..."));
        break;
    case SyncSession::OpeningSession:
        m_syncOperation->show();
        m_progressBar->show();
        m_syncOperation->setText(tr("Opening session..."));
        break;
    case SyncSession::ClosingSession:
        m_syncOperation->show();
        m_progressBar->show();
        m_syncOperation->setText(tr("Closing session..."));
        break;
    case SyncSession::Accessing:
        m_syncOperation->show();
        m_progressBar->show();
        m_syncOperation->setText(tr("Accessing..."));
        break;
    case SyncSession::Downloading:
        m_syncOperation->show();
        m_progressBar->show();
        m_syncOperation->setText(tr("Downloading..."));
        break;
    case SyncSession::Uploading:
        m_syncOperation->show();
        m_progressBar->show();
        m_syncOperation->setText(tr("Uploading..."));
        break;
    default:
        m_syncOperation->hide();
        m_progressBar->hide();
        break;
    }
}


//-----------------------------------------------------------------------------
// Private
//-----------------------------------------------------------------------------

void SyncStatusWidget::init()
{
    //horizontal line
    QFrame *line = new QFrame(this);
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);

    m_syncServiceLabel = new QLabel(tr("Cloud:"), this);
    m_syncStatusLabel = new QLabel(tr("undefined"), this);

    m_syncStatusLayout = new QHBoxLayout; //becomes child of m_mainLayout
    m_syncStatusLayout->addWidget(m_syncServiceLabel);
    m_syncStatusLayout->addWidget(m_syncStatusLabel);

    m_syncOperation = new QLabel(tr(""), this);

    m_progressBar = new QProgressBar(this);
    m_progressBar->setRange(0, 0);
    m_progressBar->hide();

    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->addWidget(line);
    m_mainLayout->addLayout(m_syncStatusLayout);
    m_mainLayout->addWidget(m_syncOperation);
    m_mainLayout->addWidget(m_progressBar);
    setLayout(m_mainLayout);
}

void SyncStatusWidget::createConnections()
{

}
