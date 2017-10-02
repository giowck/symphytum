/*
 *  Copyright (c) 2012 Giorgio Wicklein <giowckln@gmail.com>
 */

//-----------------------------------------------------------------------------
// Hearders
//-----------------------------------------------------------------------------

#include "filesformwidget.h"
#include "../../components/metadataengine.h"
#include "../../components/filemanager.h"
#include "../../utils/metadatapropertiesparser.h"
#include "../mainwindow.h"

#include <QtWidgets/QLabel>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtCore/QVariant>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QFrame>
#include <QtWidgets/QFileDialog>
#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QDateTime>
#include <QtWidgets/QProgressDialog>
#include <QtWidgets/QMenu>
#include <QtWidgets/QAction>
#include <QtGui/QContextMenuEvent>
#include <QtGui/QMouseEvent>
#include <QtWidgets/QMessageBox>
#include <QtGui/QDesktopServices>
#include <QtCore/QUrl>
#include <QtCore/QMimeData>
#include <QtCore/QList>
#include <QtWidgets/QFileIconProvider>
#include <QtWidgets/QApplication>
#include <QtGui/QDragEnterEvent>
#include <QtGui/QDropEvent>
#include <QtWidgets/QToolButton>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QTableWidgetItem>
#include <QtCore/QEventLoop>
#include <QtWidgets/QApplication>
#include <QtWidgets/QUndoStack>
#include <QtGui/QDrag>


//-----------------------------------------------------------------------------
// FilesTableWidget
//-----------------------------------------------------------------------------

FilesTableWidget::FilesTableWidget(QWidget *parent) :
    QTableWidget(parent)
{

}

FilesTableWidget::~FilesTableWidget()
{
}

void FilesTableWidget::startDrag(Qt::DropActions supportedActions)
{
    Q_UNUSED(supportedActions);

    QModelIndexList rowIndexes = this->selectionModel()->selectedRows();
    QList<int> rowList;
    foreach(QModelIndex index, rowIndexes) {
        rowList.append(index.row());
    }

    //get files
    FileManager fm(this);
    QStringList fileList;
    foreach (int row, rowList) {
        bool ok;
        int id = this->item(row, 0)->text().toInt(&ok);
        if (!ok) continue;

        QString fileName, hashName;
        QDateTime dateAdded;
        MetadataEngine::getInstance().getContentFile(id, fileName, hashName, dateAdded);

        QString filePath = fm.getFilesDirectory() + hashName;
        if (filePath.isEmpty()) continue;
        fileList.append(filePath);
    }

    //set drag pixmap
    QPixmap dragPixmap;
    if (fileList.size()) {
        dragPixmap = QFileIconProvider().
                icon(QFileInfo(fileList.at(0))).
                pixmap(64, 64);
    }
    QMimeData *mimeData = new QMimeData;
    QList<QUrl> urls;
    foreach (QString path, fileList) {
        urls.append(QUrl::fromLocalFile(path));
    }
    mimeData->setUrls(urls);

    QDrag *drag = new QDrag(this);
    drag->setMimeData(mimeData);
    drag->setPixmap(dragPixmap);
    drag->setHotSpot(QPoint(dragPixmap.width()/2, dragPixmap.height()/2));
    drag->exec(Qt::CopyAction, Qt::CopyAction);
}


//-----------------------------------------------------------------------------
// Public
//-----------------------------------------------------------------------------

FilesFormWidget::FilesFormWidget(QWidget *parent) :
    AbstractFormWidget(parent),
    m_showType(false), m_showDate(false)
{
    m_fieldNameLabel = new QLabel("Invalid Name", this);

    //static styling
    m_fieldNameLabel->setStyleSheet("QLabel {color: gray;}");

    //no files label
    m_noFilesLabel = new QLabel(this);
    m_noFilesLabel->setWordWrap(true);
    m_noFilesLabel->setText(tr("<i>No files present<br>Drag files here to import "
                               " them or click the add button</i>"));
    m_noFilesLabel->setStyleSheet("QLabel {color: gray;}");
    m_noFilesLabel->setAlignment(Qt::AlignCenter);
    m_noFilesLabel->setSizePolicy(QSizePolicy::MinimumExpanding,
                                  QSizePolicy::MinimumExpanding);

    //toolbar frame
    QFrame *toolbarFrame = new QFrame(this);
    QHBoxLayout *toolbarLayout = new QHBoxLayout(toolbarFrame);
    m_addButton = new QToolButton(toolbarFrame);
    m_addButton->setIcon(QIcon(":/images/icons/add.png"));
    m_addButton->setToolTip(tr("Add file"));
    m_removeButton = new QToolButton(toolbarFrame);
    m_removeButton->setIcon(QIcon(":/images/icons/remove.png"));
    m_removeButton->setToolTip(tr("Remove file"));
    m_exportButton = new QToolButton(toolbarFrame);
    m_exportButton->setIcon(QIcon(":/images/icons/export.png"));
    m_exportButton->setToolTip(tr("Export file"));
    toolbarLayout->addWidget(m_addButton);
    toolbarLayout->addWidget(m_removeButton);
    toolbarLayout->addWidget(m_exportButton);
    toolbarLayout->addStretch();
    toolbarLayout->setSpacing(0);
    toolbarLayout->setContentsMargins(0, 0, 0, 0);

    //files table
    m_filesTable = new FilesTableWidget(this);
    m_filesTable->setObjectName("filesTable");
    m_filesTable->setStyleSheet("#filesTable {border: none;}");
    m_filesTable->horizontalHeader()->setVisible(false);
    m_filesTable->verticalHeader()->setVisible(false);
    m_filesTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_filesTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_filesTable->setDragDropMode(QAbstractItemView::DragOnly);

    //layout
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->addWidget(m_fieldNameLabel);
    m_mainLayout->addWidget(m_noFilesLabel);
    m_mainLayout->addWidget(m_filesTable);
    m_mainLayout->addWidget(toolbarFrame);

    //components
    m_metadataEngine = &MetadataEngine::getInstance();

    //contect menu actions
    m_addAction = new QAction(tr("Add files..."), this);
    m_deleteAction = new QAction(tr("Delete files"), this);
    m_exportAction = new QAction(tr("Export files to..."), this);
    m_openAction = new QAction(tr("Open file"), this);

    //context menu connections
    connect(m_addAction, SIGNAL(triggered()),
            this, SLOT(addButtonClicked()));
    connect(m_exportAction, SIGNAL(triggered()),
            this, SLOT(exportButtonClicked()));
    connect(m_deleteAction, SIGNAL(triggered()),
            this, SLOT(removeButtonClicked()));
    connect(m_openAction, SIGNAL(triggered()),
            this, SLOT(fileItemDoubleClicked()));

    //connections
    connect(m_filesTable, SIGNAL(itemSelectionChanged()),
            this, SLOT(updateToolActions()));
    connect(m_addButton, SIGNAL(clicked()),
            this, SLOT(addButtonClicked()));
    connect(m_removeButton, SIGNAL(clicked()),
            this, SLOT(removeButtonClicked()));
    connect(m_exportButton, SIGNAL(clicked()),
            this, SLOT(exportButtonClicked()));
    connect(m_filesTable, SIGNAL(cellActivated(int,int)),
            this, SLOT(fileItemDoubleClicked()));

    this->heightUnits = 2;
    this->widthUnits = 2;

    //accept drops
    setAcceptDrops(true);

    updateToolActions();
    setupFocusPolicy();
}

FilesFormWidget::~FilesFormWidget()
{
}

void FilesFormWidget::setFieldName(const QString &name)
{
    m_fieldNameLabel->setText(name);
}

QString FilesFormWidget::getFieldName() const
{
    return m_fieldNameLabel->text();
}

void FilesFormWidget::clearData()
{
    m_filesTable->setColumnCount(0);
    m_filesTable->setRowCount(0);
    m_filesTable->clear();

    updateFileStatusLabel();
}

void FilesFormWidget::setData(const QVariant &data)
{
    if (!data.isNull()) {
        QStringList idList = data.toString().split(',',
                                                   QString::SkipEmptyParts);

        //table size
        int rows = idList.size();
        int columns = 2;
        if (m_showType) columns++;
        if (m_showDate) columns++;
        m_filesTable->setRowCount(rows);
        m_filesTable->setColumnCount(columns);
        m_filesTable->setColumnHidden(0, true); //hide ID column

        //strect first column and resize other to contents
        QHeaderView *header = m_filesTable->horizontalHeader();
        header->setSectionResizeMode(1, QHeaderView::Stretch);
        for (int i = 2; i < columns; i++) {
            header->setSectionResizeMode(i, QHeaderView::ResizeToContents);
        }

        for (int i = 0; i < rows; i++) {
            bool ok;
            int id = idList.at(i).toInt(&ok);
            if (!ok) continue;

            addFileToTable(id, i);
        }
    } else {
        clearData();
    }

    updateFileStatusLabel();
}

QVariant FilesFormWidget::getData() const
{
    QString idString;

    int rows = m_filesTable->rowCount();
    for (int i = 0; i < rows; i++) {
        QTableWidgetItem *item = m_filesTable->item(i, 0);
        idString.append(item->text());
        idString.append(",");
    }

    return idString;
}

void FilesFormWidget::loadMetadataDisplayProperties(const QString &metadata)
{
    MetadataPropertiesParser parser(metadata);
    if (metadata.size() == 0) return;

    if (parser.getValue("showFileType") == "1")
        m_showType = true;
    if (parser.getValue("showAddedDate") == "1")
        m_showDate = true;
}


//-----------------------------------------------------------------------------
// Protected
//-----------------------------------------------------------------------------

void FilesFormWidget::contextMenuEvent(QContextMenuEvent *event)
{
    //handle event only if pos within image label or no img frame
    if (!isOnValidFilePos(event->pos())) {
        QWidget::contextMenuEvent(event);
        return;
    }

    QMenu menu(this);
    menu.addAction(m_addAction);
    if (m_filesTable->selectionModel()->selectedRows().size() > 0) {
        menu.addAction(m_openAction);
        menu.addAction(m_exportAction);
        menu.addAction(m_deleteAction);
    }
    menu.exec(event->globalPos());
}

void FilesFormWidget::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasFormat("text/uri-list")) {
        QList<QUrl> urls = event->mimeData()->urls();
        FileManager fm(this);

        //check that files are not dragged from files dir
        foreach (QUrl url, urls) {
            if (url.path().contains(fm.getFilesDirectory()))
                return;
        }

        event->acceptProposedAction();
    }
}

void FilesFormWidget::dropEvent(QDropEvent *event)
{
    if (event->mimeData()->hasUrls()) {
        QList<QUrl> urls = event->mimeData()->urls();
        QStringList fileList;

        foreach (QUrl url, urls) {
            QString filePath = url.path();
#ifdef Q_OS_WIN
            filePath.remove(0, 1); //on windows: /C:/file_path is returned from mime
#endif // Q_OS_WIN
            fileList.append(filePath);
        }

        importFiles(fileList);
    }

    event->acceptProposedAction();
}


//-----------------------------------------------------------------------------
// Protected slots
//-----------------------------------------------------------------------------

void FilesFormWidget::validateData()
{
    //always valid
    emit dataEdited();
}


//-----------------------------------------------------------------------------
// Private slots
//-----------------------------------------------------------------------------

void FilesFormWidget::updateToolActions()
{
    bool selection = !m_filesTable->selectedItems().isEmpty();

    m_removeButton->setEnabled(selection);
    m_exportButton->setEnabled(selection);
}

void FilesFormWidget::addButtonClicked()
{
    QStringList files = QFileDialog::getOpenFileNames(this,
                                                      tr("Import Files"),
                                                      QDir::homePath()
                                                      );
    if (files.isEmpty())
        return;

    importFiles(files);
}

void FilesFormWidget::removeButtonClicked()
{
    //ask for confirmation
    QMessageBox box(QMessageBox::Warning, tr("Delete Files"),
                    tr("Are you sure you want to delete the selected files?"
                       "<br><br><b>Warning:</b> This cannot be undone!"),
                    QMessageBox::Yes | QMessageBox::No,
                    this);
    box.setDefaultButton(QMessageBox::Yes);
    box.setWindowModality(Qt::WindowModal);
    int r = box.exec();
    if (r == QMessageBox::No) return;

    QModelIndexList rowIndexes = m_filesTable->selectionModel()->selectedRows();
    int size = rowIndexes.size();

    QList<int> rowList;
    foreach(QModelIndex index, rowIndexes) {
        rowList.append(index.row());
    }

    //sort list so that removing from bottom to top works
    qSort(rowList.begin(), rowList.end());

    FileManager fm(this);
    QEventLoop waitLoop(this);
    connect(&fm, SIGNAL(removeFileCompletedSignal(QString)),
            &waitLoop, SLOT(quit()));
    connect(&fm, SIGNAL(fileOpFailed()),
            &waitLoop, SLOT(quit()));

    //init progress dialog
    QProgressDialog progressDialog(tr("Removing file 0 of %1")
                                   .arg(size),
                                   tr("Cancel"), 1,
                                   size, this);
    progressDialog.setWindowModality(Qt::WindowModal);
    progressDialog.setWindowTitle(tr("Progress"));
    progressDialog.setMinimumDuration(0);
    progressDialog.show();
    int progress = 0;

    for (int i = size - 1; i >= 0; i--) {
#ifdef Q_OS_WIN
        //workaroung for windows bug where dialog remains hidden sometimes
        qApp->processEvents();
#endif // Q_OS_WIN
        if (progressDialog.wasCanceled())
            break;
        progressDialog.setValue(++progress);
        progressDialog.setLabelText(tr("Removing file %1 of %2")
                                    .arg(progress)
                                    .arg(size));

        int row = rowList.at(i);
        bool ok;
        int id = m_filesTable->item(row, 0)->text().toInt(&ok);
        if (!ok) continue;

        QString fileName, hashName;
        QDateTime dateAdded;
        m_metadataEngine->getContentFile(id, fileName, hashName, dateAdded);

        //remove file
        fm.startRemoveFile(hashName);
        waitLoop.exec(); //wait until fm completes

        //remove from table
        m_filesTable->removeRow(row);
    }

    validateData();

    //clear undo stack
    QUndoStack *stack = MainWindow::getUndoStack();
    if (stack) {
        stack->clear();
    }

}

void FilesFormWidget::exportButtonClicked()
{
    QString destDirPath =
            QFileDialog::getExistingDirectory(this,
                                              tr("Export selected files to"),
                                              QDir::homePath());

    if (destDirPath.isEmpty())
        return;

    QModelIndexList rowIndexes = m_filesTable->selectionModel()->selectedRows();

    QList<int> rowList;
    foreach(QModelIndex index, rowIndexes) {
        rowList.append(index.row());
    }

    int size = rowList.size();
    FileManager fm(this);

    //init progress dialog
    QProgressDialog progressDialog(tr("Copying file 0 of %1")
                                   .arg(size),
                                   tr("Cancel"), 1,
                                   size, this);
    progressDialog.setWindowModality(Qt::WindowModal);
    progressDialog.setWindowTitle(tr("Progress"));
    progressDialog.setMinimumDuration(0);
    progressDialog.show();
    int progress = 0;

    for (int i = 0; i < size; i++) {
#ifdef Q_OS_WIN
        //workaroung for windows bug where dialog remains hidden sometimes
        qApp->processEvents();
#endif // Q_OS_WIN
        if (progressDialog.wasCanceled())
            break;
        progressDialog.setValue(++progress);
        progressDialog.setLabelText(tr("Copying file %1 of %2")
                                    .arg(progress)
                                    .arg(size));

        int row = rowList.at(i);
        bool ok;
        int id = m_filesTable->item(row, 0)->text().toInt(&ok);
        if (!ok) continue;

        QString fileName, hashName;
        QDateTime dateAdded;
        m_metadataEngine->getContentFile(id, fileName, hashName, dateAdded);

        //copy file
        QString outputFile = destDirPath + "/" + fileName;
        if (outputFile.isEmpty())
            return;

        QFile::copy(fm.getFilesDirectory() + hashName, outputFile);
    }
}

void FilesFormWidget::fileItemDoubleClicked()
{
    //open file
    int row = m_filesTable->currentRow();
    if (row >= 0) {
        bool ok;
        int id = m_filesTable->item(row, 0)->text().toInt(&ok);
        if (!ok) return;
        QString fileName, hashName;
        QDateTime dateAdded;
        m_metadataEngine->getContentFile(id,
                                         fileName,
                                         hashName,
                                         dateAdded);
        FileManager fm(this);
        fm.openContentFile(hashName);
    }
}

void FilesFormWidget::addHashNameToTable(const QString &hashName)
{
    int id = m_metadataEngine->getContentFileId(hashName);

    int rows = m_filesTable->rowCount();
    if (!rows) {
        //call setData() because columns are not configured
        //and list is empty
        setData(QString::number(id) +  ",");
    } else {
        m_filesTable->setRowCount(++rows);
        addFileToTable(id, rows - 1);
    }
}


//-----------------------------------------------------------------------------
// Private
//-----------------------------------------------------------------------------

void FilesFormWidget::setupFocusPolicy()
{
    m_addButton->setFocusPolicy(Qt::ClickFocus);
    m_removeButton->setFocusPolicy(Qt::ClickFocus);
    m_exportButton->setFocusPolicy(Qt::ClickFocus);

    setFocusProxy(m_addButton);
    setFocusPolicy(Qt::StrongFocus);
}

void FilesFormWidget::updateFileStatusLabel()
{
    bool empty = m_filesTable->rowCount() == 0;

    m_noFilesLabel->setVisible(empty);
    m_filesTable->setVisible(!empty);
}

void FilesFormWidget::importFiles(const QStringList &list)
{
    QStringList fileList = list;

    FileManager fm(this);
    QEventLoop waitLoop(this);
    connect(&fm, SIGNAL(addFileCompletedSignal(QString)),
            this, SLOT(addHashNameToTable(QString)));
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

    validateData();
}

void FilesFormWidget::addFileToTable(int id, int row)
{
    FileManager fm;
    QString filesDir = fm.getFilesDirectory();
    QFileIconProvider iconProvider;

    //get file
    QString fileName, hashName;
    QDateTime dateAdded;
    m_metadataEngine->getContentFile(id, fileName, hashName, dateAdded);

    QFileInfo info(filesDir + hashName);
    QIcon icon = iconProvider.icon(info);
    QTableWidgetItem *idItem = new QTableWidgetItem(icon,
                                                    QString::number(id));
    m_filesTable->setItem(row, 0, idItem);
    QTableWidgetItem *nameItem = new QTableWidgetItem(fileName);
    nameItem->setIcon(icon);
    m_filesTable->setItem(row, 1, nameItem);
    if (m_showType) {
        QString suffix = info.suffix().toUpper();
        QTableWidgetItem *typeItem = new QTableWidgetItem(suffix);
        m_filesTable->setItem(row, 2, typeItem);
    }
    if (m_showDate) {
        QTableWidgetItem *dateItem = new QTableWidgetItem(dateAdded.toString());
        int pos;
        if (m_showType) pos = 3;
        else pos = 2;
        m_filesTable->setItem(row, pos, dateItem);
    }
}

QStringList FilesFormWidget::getAllFilesFromDir(const QString &dirPath)
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

bool FilesFormWidget::isOnValidFilePos(const QPoint &pos)
{
    if (m_noFilesLabel->isVisible())
        return m_noFilesLabel->geometry().contains(pos);
    else
        return m_filesTable->geometry().contains(pos);
}
