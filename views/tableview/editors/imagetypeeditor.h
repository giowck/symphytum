/**
  * \class ImageTypeEditor
  * \brief This is a delegate editor for image field type.
  * \author Giorgio Wicklein - GIOWISYS Software
  * \date 11/10/2012
  */

#ifndef IMAGETYPEEDITOR_H
#define IMAGETYPEEDITOR_H


//-----------------------------------------------------------------------------
// Headers
//-----------------------------------------------------------------------------

#include <QtWidgets/QWidget>


//-----------------------------------------------------------------------------
// Forward declarations
//-----------------------------------------------------------------------------

class QPushButton;


//-----------------------------------------------------------------------------
// ImageTypeEditor
//-----------------------------------------------------------------------------

class ImageTypeEditor : public QWidget
{
    Q_OBJECT

public:
    explicit ImageTypeEditor(QWidget *parent = nullptr);

    /** Set current file id */
    void setImage(int fileId);

    /** Get current file id */
    int getImage();

signals:
    void editingFinished();

private slots:
    void browseButtonClicked();
    void setLastFileHashResult(const QString &hashName);
    
private:
    QPushButton *m_browseButton;
    int m_currentFileId;
    QString m_lastFileHashResult;
};

#endif // IMAGETYPEEDITOR_H
