/*
 *  Copyright (c) 2012 Giorgio Wicklein <giowckln@gmail.com>
 */

//-----------------------------------------------------------------------------
// Hearders
//-----------------------------------------------------------------------------

#include "imagetypeeditor.h"
#include "../../../components/filemanager.h"
#include "../../../components/metadataengine.h"

#include <QtWidgets/QPushButton>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QFileDialog>
#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtWidgets/QProgressDialog>
#include <QtWidgets/QFrame>


//-----------------------------------------------------------------------------
// Public
//-----------------------------------------------------------------------------

ImageTypeEditor::ImageTypeEditor(QWidget *parent) :
    QWidget(parent),
    m_currentFileId(0)
{
    //init
    m_browseButton = new QPushButton(tr("Select"), this);
    QFrame *backgroundFrame = new QFrame(this);
    QVBoxLayout *frameLayout = new QVBoxLayout(backgroundFrame);
    frameLayout->addWidget(m_browseButton);
    frameLayout->setContentsMargins(0, 0, 0, 0);
    frameLayout->setSpacing(0);
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(backgroundFrame);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    //style
    backgroundFrame->setStyleSheet("QFrame { background-color: palette(base); }");

    //focus proxy
    setFocusProxy(m_browseButton);

    //connections
    connect(m_browseButton, SIGNAL(clicked()),
            this, SLOT(browseButtonClicked()));
}

void ImageTypeEditor::setImage(int fileId)
{
    FileManager fm(this);
    QString fileName, hashName;
    QDateTime dateAdded;
    MetadataEngine::getInstance().getContentFile(fileId,
                                                 fileName,
                                                 hashName,
                                                 dateAdded);

    m_browseButton->setIcon(QPixmap(fm.getFilesDirectory() + hashName));
    m_currentFileId = fileId;
}

int ImageTypeEditor::getImage()
{
    return m_currentFileId;
}


//-----------------------------------------------------------------------------
// Private slots
//-----------------------------------------------------------------------------

void ImageTypeEditor::browseButtonClicked()
{
    QString file = QFileDialog::getOpenFileName(this,
                                                tr("Import Image"),
                                                QDir::homePath(),
                                                tr("Images (*.png *.jpeg "
                                                   "*.jpg *.tiff *.gif *.bmp *.svg)")
                                                );

    if (file.isEmpty())
        return;

    MetadataEngine *meta = &MetadataEngine::getInstance();
    FileManager fm(this);

    QProgressDialog pd(this);
    pd.setWindowModality(Qt::WindowModal);
    pd.setWindowTitle(tr("Importing file"));
    pd.setLabelText(tr("Copying image file... Please wait!"));
    pd.setRange(0, 0);
    pd.setValue(-1);

    connect(&fm, SIGNAL(addFileCompletedSignal(QString)),
            this, SLOT(setLastFileHashResult(QString)));
    connect(&fm, SIGNAL(addFileCompletedSignal(QString)),
            &pd, SLOT(close()));
    connect(&fm, SIGNAL(fileOpFailed()),
            &pd, SLOT(close()));

    //remove exiting file
    if (m_currentFileId) {
        QString fileName, hashName;
        QDateTime dateAdded;
        meta->getContentFile(m_currentFileId,
                             fileName,
                             hashName,
                             dateAdded);

        //if user dragged the image from and to the image label (same image)
        if (file.contains(hashName)) return;

        fm.startRemoveFile(hashName);
    }

    //add file
    fm.startAddFile(file);

    //wait until file has been copied
    pd.exec();

    //set data
    m_currentFileId = meta->getContentFileId(m_lastFileHashResult);
    m_browseButton->setIcon(QPixmap(fm.getFilesDirectory() + m_lastFileHashResult));

    //close editor
    emit editingFinished();
}


//-----------------------------------------------------------------------------
// Private slots
//-----------------------------------------------------------------------------

void ImageTypeEditor::setLastFileHashResult(const QString &hashName)
{
    m_lastFileHashResult = hashName;
}
