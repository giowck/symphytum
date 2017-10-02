/*
 *  Copyright (c) 2012 Giorgio Wicklein <giowckln@gmail.com>
 */

//-----------------------------------------------------------------------------
// Hearders
//-----------------------------------------------------------------------------

#include "filestypeeditor.h"
#include "../../../components/filemanager.h"
#include "../../../components/metadataengine.h"

#include <QtWidgets/QPushButton>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QFileDialog>
#include <QtCore/QDir>
#include <QtCore/QEventLoop>
#include <QtCore/QFile>
#include <QtWidgets/QProgressDialog>
#include <QtWidgets/QFrame>
#include <QtWidgets/QApplication>


//-----------------------------------------------------------------------------
// Public
//-----------------------------------------------------------------------------

FilesTypeEditor::FilesTypeEditor(QWidget *parent) :
    QWidget(parent)
{
    //init
    m_browseButton = new QPushButton(tr("Add files"), this);
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

void FilesTypeEditor::setFiles(const QString &files)
{
    m_fileList.clear();
    QStringList list = files.split(",", QString::SkipEmptyParts);
    foreach(QString s, list) {
        bool ok;
        int id = s.toInt(&ok);
        if (ok)
            m_fileList.append(id);
    }
}

QString FilesTypeEditor::getFiles()
{
    QString filesAsString;

    foreach(int i, m_fileList) {
        filesAsString.append(QString::number(i) + ",");
    }

    return filesAsString;
}


//-----------------------------------------------------------------------------
// Private slots
//-----------------------------------------------------------------------------

void FilesTypeEditor::browseButtonClicked()
{
    QStringList fileList = QFileDialog::getOpenFileNames(this,
                                                      tr("Import Files"),
                                                      QDir::homePath()
                                                      );
    if (fileList.isEmpty())
        return;

    FileManager fm(this);
    QEventLoop waitLoop(this);
    connect(&fm, SIGNAL(addFileCompletedSignal(QString)),
            this, SLOT(addHashNameToFileList(QString)));
    connect(&fm, SIGNAL(addFileCompletedSignal(QString)),
            &waitLoop, SLOT(quit()));
    connect(&fm, SIGNAL(fileOpFailed()),
            &waitLoop, SLOT(quit()));

    //check for directories, if found add recusively all files
    foreach (QString path, fileList) {
        QFileInfo info(path);
        if (info.isDir()) {
            //add recursively
            fileList.append(getAllFilesFromDir(path));

            //remove dir from list
            fileList.removeAll(path);
        }
    }

    int count = fileList.size();

    //init progress dialog
    QProgressDialog progressDialog(tr("Importing file 0 of %1")
                                   .arg(count),
                                   tr("Cancel"), 1,
                                   count, this);
    progressDialog.setWindowModality(Qt::WindowModal);
    progressDialog.setWindowTitle(tr("Progress"));
    progressDialog.setMinimumDuration(0);
    progressDialog.show();
    int progress = 0;

    for (int i = 0; i < count; i++) {
#ifdef Q_OS_WIN
        //workaroung for windows bug where dialog remains hidden sometimes
        qApp->processEvents();
#endif // Q_OS_WIN
        if (progressDialog.wasCanceled())
            break;
        progressDialog.setValue(++progress);
        progressDialog.setLabelText(tr("Importing file %1 of %2")
                                    .arg(progress)
                                    .arg(count));

        //import file
        fm.startAddFile(fileList.at(i));
        waitLoop.exec(); //wait until fm completes
    }

    //close editor
    emit editingFinished();
}

void FilesTypeEditor::addHashNameToFileList(const QString &hashName)
{
    int id = MetadataEngine::getInstance().getContentFileId(hashName);
    m_fileList.append(id);
}


//-----------------------------------------------------------------------------
// Private
//-----------------------------------------------------------------------------

QStringList FilesTypeEditor::getAllFilesFromDir(const QString &dirPath)
{
    QStringList files;

    QDir dir(dirPath);
    QStringList entries = dir.entryList(QDir::AllEntries |
                                        QDir::NoDotAndDotDot);

    foreach (QString entry, entries) {
        QString entryPath = dir.absoluteFilePath(entry);
        QFileInfo info(entryPath);
        if (info.isDir())
            files.append(getAllFilesFromDir(entryPath));
        else
            files.append(entryPath);
    }

    return files;
}
