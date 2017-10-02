/*
 *  Copyright (c) 2012 Giorgio Wicklein <giowckln@gmail.com>
 */

//-----------------------------------------------------------------------------
// Hearders
//-----------------------------------------------------------------------------

#include "imageformwidget.h"
#include "../../utils/metadatapropertiesparser.h"
#include "../../utils/formwidgetvalidator.h"
#include "../../components/metadataengine.h"
#include "../../components/filemanager.h"
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
#include <QtWidgets/QApplication>
#include <QtGui/QDragEnterEvent>
#include <QtGui/QDropEvent>
#include <QtWidgets/QUndoStack>
#include <QtGui/QDrag>


//-----------------------------------------------------------------------------
// Public
//-----------------------------------------------------------------------------

ImageFormWidget::ImageFormWidget(QWidget *parent) :
    AbstractFormWidget(parent),
    m_currentFileId(0),
    m_currentPixmap(0)
{
    m_fieldNameLabel = new QLabel("Invalid Name", this);
    m_mainLayout = new QVBoxLayout(this);

    //static styling
    m_fieldNameLabel->setStyleSheet("QLabel {color: gray;}");

    //no image frame
    m_noImageFrame = new QFrame(this);
    m_noImageFrame->setContentsMargins(0, 0, 0, 0);
    m_noImageFrame->setObjectName("noImgFrame");
    m_noImageFrame->setStyleSheet("QFrame#noImgFrame {border: 2px dashed lightgrey; border-radius: 12px;}");
    m_frameLayout = new QVBoxLayout(m_noImageFrame);
    m_frameLayout->setContentsMargins(0, 0, 0, 0);

    QLabel *dragHereLabel = new QLabel(tr("Drag your image here"), m_noImageFrame);
    dragHereLabel->setStyleSheet("color: grey;");
    dragHereLabel->setWordWrap(true);
    dragHereLabel->setAlignment(Qt::AlignHCenter);

    QLabel *dragIcon = new QLabel(m_noImageFrame);
    dragIcon->setScaledContents(true);
    dragIcon->setMaximumSize(64, 64);
    dragIcon->setMinimumSize(64, 64);
    dragIcon->setPixmap(QPixmap(":/images/icons/draghere.png"));
    QHBoxLayout *dragIconLayout = new QHBoxLayout;
    dragIconLayout->addStretch();
    dragIconLayout->addWidget(dragIcon);
    dragIconLayout->addStretch();


    m_browseButton = new QPushButton(tr("or click here..."), m_noImageFrame);
    m_browseButton->setFlat(true);
    m_browseButton->setStyleSheet("color: grey;");
    m_browseLayout = new QHBoxLayout;
    m_browseLayout->addStretch();
    m_browseLayout->addWidget(m_browseButton);
    m_browseLayout->addStretch();

    m_frameLayout->addWidget(dragHereLabel);
    m_frameLayout->addStretch();
    m_frameLayout->addLayout(dragIconLayout);
    m_frameLayout->addStretch();
    m_frameLayout->addLayout(m_browseLayout);

    //image label
    m_imageLabel = new QLabel(this);
    m_imageLabel->setSizePolicy(QSizePolicy::MinimumExpanding,
                                QSizePolicy::MinimumExpanding);
    m_imageLabel->setVisible(false);
    m_imageLabel->setAlignment(Qt::AlignCenter);

    //main layout
    m_mainLayout->addWidget(m_fieldNameLabel);
    m_mainLayout->addWidget(m_imageLabel);
    m_mainLayout->addWidget(m_noImageFrame);

    this->heightUnits = 2;
    this->widthUnits = 1;

    //connections
    connect(m_browseButton, SIGNAL(clicked()),
            this, SLOT(browseButtonClicked()));

    //contect menu actions
    m_selectAction = new QAction(tr("Select image..."), this);
    m_deleteAction = new QAction(tr("Delete image"), this);
    m_saveAsAction = new QAction(tr("Save image as..."), this);
    m_openAction = new QAction(tr("Open image"), this);

    //context menu connections
    connect(m_selectAction, SIGNAL(triggered()),
            this, SLOT(browseButtonClicked()));
    connect(m_saveAsAction, SIGNAL(triggered()),
            this, SLOT(saveAsActionTriggered()));
    connect(m_deleteAction, SIGNAL(triggered()),
            this, SLOT(deleteActionTriggered()));
    connect(m_openAction, SIGNAL(triggered()),
            this, SLOT(openActionTriggered()));

    //accept drops
    setAcceptDrops(true);

    //supported file types
    m_supportedFileTypes.append("PNG");
    m_supportedFileTypes.append("JPG");
    m_supportedFileTypes.append("JPEG");
    m_supportedFileTypes.append("TIFF");
    m_supportedFileTypes.append("GIF");
    m_supportedFileTypes.append("BMP");
    m_supportedFileTypes.append("SVG");

    setupFocusPolicy();
}

ImageFormWidget::~ImageFormWidget()
{
    if (m_currentPixmap) {
        delete m_currentPixmap;
        m_currentPixmap = 0;
    }
}

void ImageFormWidget::setFieldName(const QString &name)
{
    m_fieldNameLabel->setText(name);
}

QString ImageFormWidget::getFieldName() const
{
    return m_fieldNameLabel->text();
}

void ImageFormWidget::clearData()
{
    m_imageLabel->hide();
    m_currentFileId = 0;
}

void ImageFormWidget::setData(const QVariant &data)
{
    if (data.isNull() || (!data.toInt())) {
        m_imageLabel->clear();
        m_imageLabel->hide();
        m_noImageFrame->show();
        m_currentFileId = 0;
    } else {
        int fileId = data.toInt();
        QString fileName;
        QString hashName;
        QDateTime createdDateTime;
        MetadataEngine::getInstance().getContentFile(fileId,
                                                     fileName,
                                                     hashName,
                                                     createdDateTime);
        FileManager fm(this);
        QString filePath = fm.getFilesDirectory() + hashName;

        //if file not found
        if (hashName.isEmpty()) {
            setData(QVariant());
            return;
        }

        if (m_currentPixmap)
            delete m_currentPixmap;
        m_currentPixmap = new QPixmap(filePath);
        m_imageLabel->setPixmap(*m_currentPixmap);

        m_currentFileId = fileId;
        m_noImageFrame->hide();
        m_imageLabel->show();

        updatePixmapSize();
    }
}

QVariant ImageFormWidget::getData() const
{
    return m_currentFileId;
}

void ImageFormWidget::loadMetadataDisplayProperties(const QString &metadata)
{
    Q_UNUSED(metadata);

    //none for now
}


//-----------------------------------------------------------------------------
// Protected
//-----------------------------------------------------------------------------

void ImageFormWidget::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);

    updatePixmapSize();
}

void ImageFormWidget::contextMenuEvent(QContextMenuEvent *event)
{
    //handle event only if pos within image label or no img frame
    if (!isOnImgOrNoImgFrame(event->pos())) {
        QWidget::contextMenuEvent(event);
        return;
    }

    QMenu menu(this);
    menu.addAction(m_selectAction);
    if (m_currentFileId) {
        menu.addAction(m_openAction);
        menu.addAction(m_saveAsAction);
        menu.addAction(m_deleteAction);
    }
    menu.exec(event->globalPos());
}

void ImageFormWidget::mouseDoubleClickEvent(QMouseEvent *event)
{
    //handle event only if pos within image label or no img frame
    if (!isOnImgOrNoImgFrame(event->pos())) {
        QWidget::mouseDoubleClickEvent(event);
        return;
    }

    if (!m_currentFileId) {
        browseButtonClicked();
        return;
    }

    openActionTriggered();
}

void ImageFormWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
             m_dragStartPosition = event->pos();

    QWidget::mousePressEvent(event);
}

void ImageFormWidget::mouseMoveEvent(QMouseEvent *event)
{
    bool stop = false;

    //handle event only if pos within image label or no img frame
    if (!isOnImgOrNoImgFrame(event->pos()))
        stop = true;

    if (!m_currentFileId)
        stop = true;
    if (!(event->buttons() & Qt::LeftButton))
        stop = true;
    if ((event->pos() - m_dragStartPosition).manhattanLength()
            < QApplication::startDragDistance())
        stop = true;
    if (stop) {
        QWidget::mouseMoveEvent(event);
        return;
    }

    //get file
    QString fileName, hashName;
    QDateTime dateAdded;
    MetadataEngine::getInstance().getContentFile(m_currentFileId,
                                                 fileName,
                                                 hashName,
                                                 dateAdded);
    FileManager fm(this);
    QString filePath = fm.getFilesDirectory() + hashName;

    if (filePath.isEmpty()) return;

    //set drag pixmap
    QSize scaledSize(128, 128);
    QSize pixmapSize = m_currentPixmap->size();
    if ((scaledSize.width() > pixmapSize.width()) ||
            (scaledSize.height() > pixmapSize.height())) {
        scaledSize = pixmapSize;
    }
    QPixmap dragPixmap = m_currentPixmap->scaled(scaledSize,
                                                 Qt::KeepAspectRatio,
                                                 Qt::SmoothTransformation);
    QMimeData *mimeData = new QMimeData;
    QList<QUrl> urls;
    urls.append(QUrl::fromLocalFile(filePath));
    mimeData->setUrls(urls);

    QDrag *drag = new QDrag(this);
    drag->setMimeData(mimeData);
    drag->setPixmap(dragPixmap);
    drag->setHotSpot(QPoint(dragPixmap.width()/2, dragPixmap.height()/2));
    drag->exec(Qt::CopyAction, Qt::CopyAction);
}

void ImageFormWidget::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasFormat("text/uri-list"))
        event->acceptProposedAction();
}

void ImageFormWidget::dropEvent(QDropEvent *event)
{
    if (event->mimeData()->hasUrls()) {
        QList<QUrl> urls = event->mimeData()->urls();
        //find first image
        foreach (QUrl url, urls) {
            QFileInfo info(url.path());
            QString fileSuffix = info.suffix().toUpper();
            if (m_supportedFileTypes.contains(fileSuffix)) {
                QString filePath = url.path();
#ifdef Q_OS_WIN
                filePath.remove(0, 1); //on windows: /C:/file_path is returned from mime
#endif // Q_OS_WIN
                importImage(filePath);
                break;
            }
        }
    }

    event->acceptProposedAction();
}


//-----------------------------------------------------------------------------
// Protected slots
//-----------------------------------------------------------------------------

void ImageFormWidget::validateData()
{
    //always valid
    emit dataEdited();
}


//-----------------------------------------------------------------------------
// Private slots
//-----------------------------------------------------------------------------

void ImageFormWidget::browseButtonClicked()
{
    QString file = QFileDialog::getOpenFileName(this,
                                                tr("Import Image"),
                                                QDir::homePath(),
                                                tr("Images (*.png *.jpeg "
                                                   "*.jpg *.tiff *.gif *.bmp *.svg)")
                                                );

    if (file.isEmpty())
        return;

    importImage(file);
}

void ImageFormWidget::setLastFileHashResult(const QString &hashName)
{
    m_lastFileHashResult = hashName;
}

void ImageFormWidget::saveAsActionTriggered()
{
    FileManager fm(this);
    MetadataEngine *meta = &MetadataEngine::getInstance();

    //get file
    QString fileName, hashName;
    QDateTime dateAdded;
    meta->getContentFile(m_currentFileId,
                         fileName,
                         hashName,
                         dateAdded);

    QString defaultOutputName = QDir::homePath() + "/" + fileName;
    QString outputFile = QFileDialog::getSaveFileName(this,
                                                      tr("Export Image"),
                                                      defaultOutputName);
    if (outputFile.isEmpty())
        return;

    QFile::copy(fm.getFilesDirectory() + hashName, outputFile);
}

void ImageFormWidget::deleteActionTriggered()
{
    //ask for confirmation
    QMessageBox box(QMessageBox::Warning, tr("Delete Image"),
                    tr("Are you sure you want to delete the selected image?"
                       "<br><br><b>Warning:</b> This cannot be undone!"),
                    QMessageBox::Yes | QMessageBox::No,
                    this);
    box.setDefaultButton(QMessageBox::Yes);
    box.setWindowModality(Qt::WindowModal);
    int r = box.exec();
    if (r == QMessageBox::No) return;

    FileManager fm(this);
    MetadataEngine *meta = &MetadataEngine::getInstance();

    QProgressDialog pd(this);
    pd.setWindowModality(Qt::WindowModal);
    pd.setWindowTitle(tr("Deleting file"));
    pd.setLabelText(tr("Deleting image file... Please wait!"));
    pd.setRange(0, 0);
    pd.setValue(-1);

    connect(&fm, SIGNAL(removeFileCompletedSignal(QString)),
            &pd, SLOT(close()));
    connect(&fm, SIGNAL(fileOpFailed()),
            &pd, SLOT(close()));

    //remove file
    QString fileName, hashName;
    QDateTime dateAdded;
    meta->getContentFile(m_currentFileId,
                         fileName,
                         hashName,
                         dateAdded);
    fm.startRemoveFile(hashName);

    //wait until file has been copied
    pd.exec();

    //save changes
    m_currentFileId = 0;
    setData(QVariant());
    validateData();

    //clear undo stack
    QUndoStack *stack = MainWindow::getUndoStack();
    if (stack) {
        stack->clear();
    }
}

void ImageFormWidget::openActionTriggered()
{
    QString fileName, hashName;
    QDateTime dateAdded;
    MetadataEngine::getInstance().getContentFile(m_currentFileId,
                                                 fileName,
                                                 hashName,
                                                 dateAdded);
    FileManager fm(this);
    fm.openContentFile(hashName);
}


//-----------------------------------------------------------------------------
// Private
//-----------------------------------------------------------------------------

void ImageFormWidget::setupFocusPolicy()
{
    m_browseButton->setFocusPolicy(Qt::ClickFocus);
    setFocusProxy(m_browseButton);
    setFocusPolicy(Qt::StrongFocus);
}

void ImageFormWidget::updatePixmapSize()
{
    if (m_currentPixmap) {
        QSize pixmapSize = m_currentPixmap->size();
        QSize newSize = m_imageLabel->size();

        if ((newSize.width() > pixmapSize.width()) &&
                (newSize.height() > pixmapSize.height())) {
            newSize = m_currentPixmap->size();
        }

        QPixmap newPixmap = m_currentPixmap->scaled(newSize,
                                                    Qt::KeepAspectRatio,
                                                    Qt::SmoothTransformation);
        m_imageLabel->setPixmap(newPixmap);
    }
}

bool ImageFormWidget::isOnImgOrNoImgFrame(const QPoint &pos)
{
    if (m_imageLabel->isVisible())
        return m_imageLabel->geometry().contains(pos);
    else
        return m_noImageFrame->geometry().contains(pos);
}

void ImageFormWidget::importImage(const QString &file)
{
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

        //clear data (save empty state)
        m_currentFileId = 0;
        setData(QVariant());
        validateData();
    }

    //add file
    fm.startAddFile(file);

    //wait until file has been copied
    pd.exec();

    //save changes
    m_currentFileId = meta->getContentFileId(m_lastFileHashResult);
    validateData();
}
