/**
  * \class FilesFormWidget
  * \brief A form widget for file management.
  * \author Giorgio Wicklein - GIOWISYS Software
  * \date 22/10/2012
  */

#ifndef FILESFORMWIDGET_H
#define FILESFORMWIDGET_H

//-----------------------------------------------------------------------------
// Headers
//-----------------------------------------------------------------------------

#include "abstractformwidget.h"

#include <QtWidgets/QTableWidget>


//-----------------------------------------------------------------------------
// Forward declarations
//-----------------------------------------------------------------------------

class QLabel;
class QVBoxLayout;
class QToolButton;
class QTableWidget;
class MetadataEngine;

class FilesTableWidget : public QTableWidget
{
    Q_OBJECT
public:
    FilesTableWidget(QWidget *parent = 0);
    ~FilesTableWidget();
protected:
    void startDrag(Qt::DropActions supportedActions);
};


//-----------------------------------------------------------------------------
// FilesFormWidget
//-----------------------------------------------------------------------------

class FilesFormWidget : public AbstractFormWidget
{
    Q_OBJECT

public:
    explicit FilesFormWidget(QWidget *parent = 0);
    ~FilesFormWidget();

    void setFieldName(const QString &name);
    QString getFieldName() const;
    void clearData();
    void setData(const QVariant &data);
    QVariant getData() const;

    /**
     * Supported display properties are:
     * - showFileType: 1, 0
     * - showAddedDate: 1, 0
     */
    void loadMetadataDisplayProperties(const QString &metadata);

protected:
    void contextMenuEvent(QContextMenuEvent *event);
    void dragEnterEvent(QDragEnterEvent *event);
    void dropEvent(QDropEvent *event);

protected slots:
    /**
     * Supported edit properties are:
     * (none for now)
     */
    void validateData();

private slots:
    void updateToolActions();
    void addButtonClicked();
    void removeButtonClicked();
    void exportButtonClicked();
    void fileItemDoubleClicked();
    void addHashNameToTable(const QString &hashName);

private:
    /** Set the focus policy to accept focus and to redirect it to input line */
    void setupFocusPolicy();

    /** Show no files label if needed */
    void updateFileStatusLabel();

    /** Import the specified files (dirs allowed) */
    void importFiles(const QStringList &list);

    /** Append the specified file to the files table */
    void addFileToTable(int id, int row);

    /** Return a list of all files contained in the dir */
    QStringList getAllFilesFromDir(const QString &dirPath);

    /** Return whether the point is inside the files table or no files label */
    bool isOnValidFilePos(const QPoint &pos);

    QLabel *m_fieldNameLabel;
    QLabel *m_noFilesLabel;
    QVBoxLayout *m_mainLayout;
    QToolButton *m_addButton;
    QToolButton *m_removeButton;
    QToolButton *m_exportButton;
    FilesTableWidget *m_filesTable;
    QAction *m_deleteAction;
    QAction *m_addAction;
    QAction *m_exportAction;
    QAction *m_openAction;
    MetadataEngine *m_metadataEngine;
    bool m_showType, m_showDate;
};

#endif // FILESFORMWIDGET_H
