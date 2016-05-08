/**
  * \class ImageFormWidget
  * \brief A form widget representing image fields.
  * \author Giorgio Wicklein - GIOWISYS Software
  * \date 08/10/2012
  */

#ifndef IMAGEFORMWIDGET_H
#define IMAGEFORMWIDGET_H

//-----------------------------------------------------------------------------
// Headers
//-----------------------------------------------------------------------------

#include "abstractformwidget.h"


//-----------------------------------------------------------------------------
// Forward declarations
//-----------------------------------------------------------------------------

class QLabel;
class QVBoxLayout;
class QHBoxLayout;
class QPushButton;
class QFrame;
class QPixmap;
class QAction;


//-----------------------------------------------------------------------------
// ImageFormWidget
//-----------------------------------------------------------------------------

class ImageFormWidget : public AbstractFormWidget
{
    Q_OBJECT

public:
    explicit ImageFormWidget(QWidget *parent = 0);
    ~ImageFormWidget();

    void setFieldName(const QString &name);
    QString getFieldName() const;
    void clearData();
    void setData(const QVariant &data);
    QVariant getData() const;

    /**
     * Supported display properties are:
     * (none for now)
     */
    void loadMetadataDisplayProperties(const QString &metadata);

protected:
    /** Reimplemented to adapt image size */
    void showEvent(QShowEvent *event);

    void contextMenuEvent(QContextMenuEvent *event);
    void mouseDoubleClickEvent(QMouseEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void dragEnterEvent(QDragEnterEvent *event);
    void dropEvent(QDropEvent *event);

protected slots:
    /**
     * Supported edit properties are:
     * (none for now)
     */
    void validateData();

private slots:
    void browseButtonClicked();
    void setLastFileHashResult(const QString &hashName);
    void saveAsActionTriggered();
    void deleteActionTriggered();
    void openActionTriggered();

private:
    /** Set the focus policy to accept focus and to redirect it to input line */
    void setupFocusPolicy();

    /** Update the size of the pixmap by keeping aspect ration*/
    void updatePixmapSize();

    /** Return whether the point is inside the image label or the no image frame */
    bool isOnImgOrNoImgFrame(const QPoint &pos);

    /** Import an image file */
    void importImage(const QString &file);

    QVBoxLayout *m_mainLayout;
    QLabel *m_fieldNameLabel;
    QHBoxLayout *m_browseLayout;
    QFrame *m_noImageFrame;
    QVBoxLayout *m_frameLayout;
    QPushButton *m_browseButton;
    QLabel *m_imageLabel;
    int m_currentFileId;
    QString m_lastFileHashResult;
    QPixmap *m_currentPixmap;
    QAction *m_deleteAction;
    QAction *m_selectAction;
    QAction *m_saveAsAction;
    QAction *m_openAction;
    QPoint m_dragStartPosition;
    QList<QString> m_supportedFileTypes;
};

#endif // IMAGEFORMWIDGET_H
