/**
  * \class TableView
  * \brief This class implements a View from Qt's Model/View framework.
  *        TableView presents the data from the model in a table-like view.
  * \author Giorgio Wicklein - GIOWISYS Software
  * \date 02/06/2012
  */

#ifndef TABLEVIEW_H
#define TABLEVIEW_H


//-----------------------------------------------------------------------------
// Headers
//-----------------------------------------------------------------------------

#include <QtWidgets/QTableView>


//-----------------------------------------------------------------------------
// Forward declarations
//-----------------------------------------------------------------------------

class TableViewDelegate;
class QAction;


//-----------------------------------------------------------------------------
// TableView
//-----------------------------------------------------------------------------

class TableView : public QTableView
{
    Q_OBJECT

public:
    explicit TableView(QWidget *parent = 0);

    /** Reimplemented to call custom view init methods after setting model */
    void setModel(QAbstractItemModel *model);

    /** Get the last edited row (record) */
    int getLastEditRow();

    /** Get the last edited column (field) */
    int getLastEditColumn();

    /** Reload default row size from settings */
    void reloadRowSize();

signals:
    /** Emitted when new field action was triggered from context menu */
    void newFieldSignal();

    /** Emitted when duplicate field action was triggered from context menu */
    void duplicateFieldSignal();

    /** Emitted when delete field action was triggered from context menu */
    void deleteFieldSignal();

    /** Emitted when modify field action was triggered from context menu */
    void modifyFieldSignal();

    /** Emitted when new record action was triggered from context menu */
    void newRecordSignal();

    /** Emitted when duplicate record action was triggered from context menu */
    void duplicateRecordSignal();

    /** Emitted when duplicate record action was triggered from context menu */
    void deleteRecordSignal();

    /** This signal is emitted only if the user changed something by using
     *  the delegate's editor. This means: user finished editing manually
     */
    void recordEditFinished(int startRow, int endRow);

protected slots:
    void closeEditor(QWidget *editor, QAbstractItemDelegate::EndEditHint hint);
    void dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight,
                     const QVector<int> &roles);
    void currentChanged(const QModelIndex &current, const QModelIndex &previous);

protected:
    bool edit(const QModelIndex &index, EditTrigger trigger, QEvent *event);
    void contextMenuEvent(QContextMenuEvent *event);

private slots:
    void initView();
    void saveSectionOrder();
    void saveSectionSizes();

    /** Manually editing (ie. editor closed and data committed) complete */
    void editingFinished();

private:
    /** Call fetchMore() on model until all data is loaded */
    void modelFetchAll();

    /** Create actions for context menu */
    void createContextActions();

    void restoreSectionOrder();
    void restoreSectionSizes();
    void restoreRowSize();

    //context menu actions
    QAction *m_newFieldContextAction;
    QAction *m_duplicateFieldContextAction;
    QAction *m_deleteFieldContextAction;
    QAction *m_modifyFieldContextAction;
    QAction *m_newRecordContextAction;
    QAction *m_duplicateRecordContextAction;
    QAction *m_deleteRecordContextAction;

    TableViewDelegate *m_delegate;
    int m_lastUsedRow; /**< Keep track of last focused row */
    int m_lastUsedColumn; /**< Keep track of last focused column */
};

#endif // TABLEVIEW_H
