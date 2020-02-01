/*
 *  Copyright (c) 2012 Giorgio Wicklein <giowckln@gmail.com>
 */

//-----------------------------------------------------------------------------
// Hearders
//-----------------------------------------------------------------------------

#include "tableview.h"
#include "tableviewdelegate.h"
#include "../../components/settingsmanager.h"
#include "../../components/metadataengine.h"

#include <QtWidgets/QHeaderView>
#include <QtWidgets/QMenu>
#include <QtWidgets/QAction>
#include <QtGui/QContextMenuEvent>


//-----------------------------------------------------------------------------
// Public
//-----------------------------------------------------------------------------

TableView::TableView(QWidget *parent) :
    QTableView(parent), m_lastUsedRow(-1), m_lastUsedColumn(-1)
{
    m_delegate = new TableViewDelegate(this);
    setItemDelegate(m_delegate);
    setSortingEnabled(true);

    createContextActions();

    //connections
    connect(m_delegate, SIGNAL(commitData(QWidget*)),
            this, SLOT(editingFinished()));
    
#ifdef Q_OS_WIN
    //fix wrong text color and improve default font
    setStyleSheet("QTableView { selection-color: palette(text); font-family: Segoe UI; }");
#endif
}

void TableView::setModel(QAbstractItemModel *model)
{
    QTableView::setModel(model);

    if (model) {
        //connections
        connect(model, SIGNAL(layoutChanged()),
                this, SLOT(initView()));

        //init view
        restoreSectionOrder();
        restoreSectionSizes();
        restoreRowSize();
        initView();
    }
}

int TableView::getLastEditRow()
{
    return m_lastUsedRow;
}

int TableView::getLastEditColumn()
{
    return m_lastUsedColumn;
}

void TableView::reloadRowSize()
{
    restoreRowSize();
}


//-----------------------------------------------------------------------------
// Protected slots
//-----------------------------------------------------------------------------

void TableView::closeEditor(QWidget *editor, QAbstractItemDelegate::EndEditHint hint)
{
    QTableView::closeEditor(editor, hint);
    QModelIndex index;

    modelFetchAll();

    switch (hint) {
    case QAbstractItemDelegate::EditNextItem:
        if (m_lastUsedRow < model()->rowCount()) {
            QHeaderView *header = horizontalHeader();
            int currentVisual = header->visualIndex(m_lastUsedColumn);
            int nextLogical = header->logicalIndex(currentVisual + 1);
            int nextVisual = currentVisual + 1;

            //if it is not the last column, edit next column
            if (nextVisual < model()->columnCount()) {
                index = model()->index(m_lastUsedRow, nextLogical);
                if (index.isValid()) {
                    setCurrentIndex(index);
                    QTableView::edit(currentIndex());
                }
            } else { //start editing next item on next row
                int firstLogical = header->logicalIndex(1); //1 because _id (0) is hidden
                index = model()->index(m_lastUsedRow + 1, firstLogical);
                if (index.isValid()) {
                    setCurrentIndex(index);
                    QTableView::edit(currentIndex());
                } else {
                    //last row, so select only
                    setCurrentIndex(model()->
                                    index(m_lastUsedRow, m_lastUsedColumn));
                }
            }
        }
        break;
    default:
        //focus last used cell only
        index = model()->index(m_lastUsedRow, m_lastUsedColumn);
        if (index.isValid()) {
            setCurrentIndex(index);
        }
        break;
    }
}

void TableView::dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight,
                            const QVector<int> &roles)
{
    modelFetchAll();

    QTableView::dataChanged(topLeft, bottomRight, roles);
}

void TableView::currentChanged(const QModelIndex &current, const QModelIndex &previous)
{
    modelFetchAll();

    QTableView::currentChanged(current, previous);
}


//-----------------------------------------------------------------------------
// Protected
//-----------------------------------------------------------------------------

bool TableView::edit(const QModelIndex &index, EditTrigger trigger, QEvent *event)
{
    modelFetchAll();

    //Update last used row/column only on real edit triggers
    if ((trigger != QAbstractItemView::CurrentChanged) &&
            (trigger != QAbstractItemView::NoEditTriggers)) {
        if (index.isValid()) {
            m_lastUsedRow = index.row();
            m_lastUsedColumn = index.column();
        } else {
            m_lastUsedRow = -1;
            m_lastUsedColumn = -1;
        }
    }

    return QTableView::edit(index, trigger, event);
}

void TableView::contextMenuEvent(QContextMenuEvent *event)
{
    QMenu menu(this);
    menu.addAction(m_newRecordContextAction);
    if (selectionModel()->hasSelection()) {
        menu.addAction(m_duplicateRecordContextAction);
        menu.addAction(m_deleteRecordContextAction);
    }
    menu.addSeparator();
    menu.addAction(m_newFieldContextAction);
    if (selectionModel()->hasSelection()) {
        menu.addAction(m_modifyFieldContextAction);
        menu.addAction(m_duplicateFieldContextAction);
        menu.addAction(m_deleteFieldContextAction);
    }
    menu.exec(event->globalPos());
}


//-----------------------------------------------------------------------------
// Private slots
//-----------------------------------------------------------------------------

void TableView::initView()
{
    QHeaderView *header = horizontalHeader();

    header->blockSignals(true); //block signals to avoid saveSectionSizes/Order being called
    header->hideSection(0); //hide _id column
    header->blockSignals(false);

    setAlternatingRowColors(true);

    //allow section move and resizing
    horizontalHeader()->setSectionsMovable(true);
    connect(horizontalHeader(), SIGNAL(sectionMoved(int,int,int)),
            this, SLOT(saveSectionOrder()), Qt::UniqueConnection);
    connect(horizontalHeader(), SIGNAL(sectionMoved(int,int,int)),
            this, SLOT(saveSectionSizes()), Qt::UniqueConnection);
    connect(horizontalHeader(), SIGNAL(sectionResized(int,int,int)),
            this, SLOT(saveSectionSizes()), Qt::UniqueConnection);
}

void TableView::saveSectionOrder()
{
    //save section order
    SettingsManager s;
    QHeaderView *header = horizontalHeader();
    int id = MetadataEngine::getInstance().getCurrentCollectionId();
    QString collection = QString("collection_") + QString::number(id);
    QList<int> sectionOrder;
    QAbstractItemModel *m = model();
    if (m) {
        int columnCount = m->columnCount();

        for (int i = 1; i < columnCount; i++) { //+1 because of _id column 0
            sectionOrder.append(header->visualIndex(i));
        }

        QList<QVariant> list;
        foreach(int i, sectionOrder){
            list << i;
        }

        s.saveProperty("tableview_section_order", collection, list);
    }
}

void TableView::saveSectionSizes()
{
    //save section order
    SettingsManager s;
    QHeaderView *header = horizontalHeader();
    int id = MetadataEngine::getInstance().getCurrentCollectionId();
    QString collection = QString("collection_") + QString::number(id);
    QList<int> sectionSizes;
    QAbstractItemModel *m = model();
    if (m) {
        int columnCount = m->columnCount();

        for (int i = 1; i < columnCount; i++) { //+1 because of _id column 0
            sectionSizes.append(header->sectionSize(header->visualIndex(i)));
        }

        QList<QVariant> list;
        foreach(int i, sectionSizes){
            list << i;
        }

        s.saveProperty("tableview_section_sizes", collection, list);
    }
}

void TableView::editingFinished()
{
    int row = m_lastUsedRow;
    if (TableViewDelegateFlags::dataChangedOnLastEdit) {
        emit recordEditFinished(row, row);
    }
}


//-----------------------------------------------------------------------------
// Private
//-----------------------------------------------------------------------------

void TableView::modelFetchAll()
{
    QAbstractItemModel *m = model();
    if (m) {
        const QModelIndex invalidIndex;
        while (m->canFetchMore(invalidIndex))
            m->fetchMore(invalidIndex);
    }
}

void TableView::createContextActions()
{
    m_newFieldContextAction = new QAction(tr("New field"), this);
    m_duplicateFieldContextAction = new QAction(tr("Duplicate field"), this);
    m_deleteFieldContextAction = new QAction(tr("Delete field"), this);
    m_modifyFieldContextAction = new QAction(tr("Modify field"), this);
    m_newRecordContextAction = new QAction(tr("New record"), this);
    m_duplicateRecordContextAction = new QAction(tr("Duplicate record"), this);
    m_deleteRecordContextAction = new QAction(tr("Delete record"), this);

    //connections
    connect(m_newFieldContextAction, SIGNAL(triggered()),
            this, SIGNAL(newFieldSignal()));
    connect(m_duplicateFieldContextAction, SIGNAL(triggered()),
            this, SIGNAL(duplicateFieldSignal()));
    connect(m_deleteFieldContextAction, SIGNAL(triggered()),
            this, SIGNAL(deleteFieldSignal()));
    connect(m_modifyFieldContextAction, SIGNAL(triggered()),
            this, SIGNAL(modifyFieldSignal()));
    connect(m_newRecordContextAction, SIGNAL(triggered()),
            this, SIGNAL(newRecordSignal()));
    connect(m_duplicateRecordContextAction, SIGNAL(triggered()),
            this, SIGNAL(duplicateRecordSignal()));
    connect(m_deleteRecordContextAction, SIGNAL(triggered()),
            this, SIGNAL(deleteRecordSignal()));
}

void TableView::restoreSectionOrder()
{
    //restore section order
    SettingsManager s;
    QHeaderView *header = horizontalHeader();
    int id = MetadataEngine::getInstance().getCurrentCollectionId();
    QString collection = QString("collection_") + QString::number(id);
    QList<int> sectionOrder;

    QList<QVariant> list;
    list = s.restoreProperty("tableview_section_order", collection).toList();
    foreach(QVariant v, list){
      sectionOrder << v.toInt();
    }

    header->blockSignals(true); //block signals to avoid saveSectionSizes/Order being called
    for (int i = 0; i < sectionOrder.size(); i++) {
        int visualIndex = header->visualIndex(i+1); //+1 because of _id column 0

        header->moveSection(visualIndex, sectionOrder.at(i));
    }
    header->blockSignals(false);
}

void TableView::restoreSectionSizes()
{
    SettingsManager s;
    QHeaderView *header = horizontalHeader();

    //setup column resize mode
    int tableColumnWidthMode =  s.restoreProperty(
                "columnResizeMode", "tableView").toInt();
    if (tableColumnWidthMode == 1) { //1 means fixed resize mode
        header->setSectionResizeMode(QHeaderView::ResizeToContents);
    } else {
        header->setSectionResizeMode(QHeaderView::Interactive);

        //restore section sizes
        int id = MetadataEngine::getInstance().getCurrentCollectionId();
        QString collection = QString("collection_") + QString::number(id);
        QList<int> sectionSizes;

        QList<QVariant> list;
        list = s.restoreProperty("tableview_section_sizes", collection).toList();
        foreach(QVariant v, list){
          sectionSizes << v.toInt();
        }

        header->blockSignals(true); //block signals to avoid saveSectionSizes/Order being called
        for (int i = 0; i < sectionSizes.size(); i++) {
            int visualIndex = header->visualIndex(i+1); //+1 because of _id column 0
            header->resizeSection(visualIndex, sectionSizes.at(i));
        }
        header->blockSignals(false);
    }
}

void TableView::restoreRowSize()
{
    QHeaderView *verticalHeader = this->verticalHeader();
    SettingsManager s;
    int rowSize = s.restoreProperty("rowSize", "tableView").toInt();
    static int defaultRowSize = verticalHeader->defaultSectionSize(); //static, remember the first value

    if (rowSize) {
        verticalHeader->setDefaultSectionSize(defaultRowSize * rowSize);
    }
}
