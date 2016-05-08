/**
  * \class FilesTypeEditor
  * \brief This is a delegate editor for files field type.
  * \author Giorgio Wicklein - GIOWISYS Software
  * \date 26/10/2012
  */

#ifndef FILESTYPEEDITOR_H
#define FILESTYPEEDITOR_H


//-----------------------------------------------------------------------------
// Headers
//-----------------------------------------------------------------------------

#include <QtWidgets/QWidget>


//-----------------------------------------------------------------------------
// Forward declarations
//-----------------------------------------------------------------------------

class QPushButton;


//-----------------------------------------------------------------------------
// FilesTypeEditor
//-----------------------------------------------------------------------------

class FilesTypeEditor : public QWidget
{
    Q_OBJECT

public:
    explicit FilesTypeEditor(QWidget *parent = 0);

    /** Set files as file ids separated by comma */
    void setFiles(const QString &files);

    /** Get current files list */
    QString getFiles();

signals:
    void editingFinished();

private slots:
    void browseButtonClicked();
    void addHashNameToFileList(const QString &hashName);

private:
    /** Return a list of all files contained in the dir */
    QStringList getAllFilesFromDir(const QString &dirPath);

    QPushButton *m_browseButton;
    QList<int> m_fileList;
};

#endif // FILESTYPEEDITOR_H
