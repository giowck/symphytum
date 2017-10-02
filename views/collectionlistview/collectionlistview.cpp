/*
 *  Copyright (c) 2012 Giorgio Wicklein <giowckln@gmail.com>
 */

//-----------------------------------------------------------------------------
// Hearders
//-----------------------------------------------------------------------------

#include "collectionlistview.h"
#include "collectionviewdelegate.h"
#include "../../models/collectionlistmodel.h"
#include "../../components/metadataengine.h"
#include "../../components/settingsmanager.h"
#include "../../components/undocommands.h"
#include "../../components/sync_framework/syncsession.h"
#include "../../widgets/mainwindow.h"
#include "../../utils/collectionfieldcleaner.h"

#include <QtWidgets/QAction>
#include <QtWidgets/QMenu>
#include <QtGui/QContextMenuEvent>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QUndoStack>


//-----------------------------------------------------------------------------
// Public
//-----------------------------------------------------------------------------

CollectionListView::CollectionListView(QWidget *parent) :
    QListView(parent), m_currentCollectionId(0)
{
    setAttribute(Qt::WA_MacShowFocusRect, 0); //on mac disable focus rect around borders
    setFocusPolicy(Qt::ClickFocus);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setItemDelegate(new CollectionViewDelegate(this));

    setStyleSheet(
                "QListView { "
                "border: none; "
                "background-color: transparent;"
                "font-size: 13px;"
                "}"
                "QListView:item { "
                "padding-bottom: 3px;"
                "padding-top: 3px;"
                "padding-left: 15px;"
                "border: none;" //workaround qt stylesheet bug, border must be specificated
                "}"
                "QListView:item:selected { "
                "border-top: 1px solid #57A5DA;"
                "border-bottom: 1px solid #2B7CBD;"
                "background-color: qlineargradient(spread:pad, x1:0.506926, y1:1, x2:0.512, y2:0, stop:0 rgba(46, 137, 205, 255),"
                                                  "stop:1 rgba(108, 186, 230, 255));"
                "color: white;"
                "}"
#ifdef Q_OS_OSX
                "QListView:item:selected:!active { "
                "border-top: 1px solid #BBC5D5;"
                "border-bottom: 1px solid #92A0B8;"
                "background-color: qlineargradient(spread:pad, x1:0.506926, y1:1, x2:0.512, y2:0, stop:0 rgba(155, 170, 195, 255),"
                                                  "stop:1 rgba(195, 204, 222, 255));"
                "color: white;"
                "}"
#endif // Q_OS_OSX
                );

    //set model
    attachModel();

    //connections
    connect(this->itemDelegate(), SIGNAL(closeEditor(QWidget*)),
            this, SLOT(editingFinished()));

    //contect menu actions
    m_newCollectionAction = new QAction(tr("New"), this);
    m_deleteCollectionAction = new QAction(tr("Delete"), this);

    //context menu connections
    connect(m_deleteCollectionAction, SIGNAL(triggered()),
            this, SLOT(deleteCollectionActionTriggered()));
    connect(m_newCollectionAction, SIGNAL(triggered()),
            this, SLOT(newCollectionActionTriggered()));

    //connect collection change (if triggered from elsewhere)
    MetadataEngine *meta = &MetadataEngine::getInstance();
    connect(meta, SIGNAL(currentCollectionIdChanged(int)),
            this, SLOT(currentCollectionIdChanged(int)));

    setSizePolicy(QSizePolicy::Minimum, QSizePolicy::MinimumExpanding);
}

void CollectionListView::createNewCollection()
{
    if (SyncSession::IS_READ_ONLY) return;

    //add item
    m_model->addCollection();

    //set local data changed
    SyncSession::LOCAL_DATA_CHANGED = true;

    //prepare collection table and metadata
    MetadataEngine::getInstance().createNewCollection();

    //FIXME: temporary workaround for listview not updating the items
    //needs investigation, caused by migration from Qt4 to Qt5
    this->detachModel();
    this->attachModel();

    //edit it
    QModelIndex index = m_model->index(m_model->rowCount() - 1, 1);
    setCurrentIndex(index);
    edit(index);
}

void CollectionListView::deleteCollection()
{
    int collectionId = MetadataEngine::getInstance().getCurrentCollectionId();

    if ((collectionId == 0) || SyncSession::IS_READ_ONLY) return; //0 stands for invalid

    //ask for confirmation
    QMessageBox box(QMessageBox::Question, tr("Delete Collection"),
                    tr("Are you sure you want to delete the selected collection?"
                       "<br><br><b>Warning:</b> This cannot be undone!"),
                    QMessageBox::Yes | QMessageBox::No,
                    this);
    box.setDefaultButton(QMessageBox::Yes);
    box.setWindowModality(Qt::WindowModal);
    int r = box.exec();
    if (r == QMessageBox::No) return;

    //check all fields for delete triggers
    CollectionFieldCleaner cleaner(this);
    cleaner.cleanCollection(collectionId);

    //delete metadata and tables
    MetadataEngine::getInstance().deleteCollection(collectionId);

    //delete from model
    m_model->removeRow(currentIndex().row());

    //delete settings about collection's column positions in TableView
    SettingsManager s;
    QString settingsKey = QString("collection_") + QString::number(collectionId);
    s.deleteObjectProperties(settingsKey);

    //reset cached id
    m_currentCollectionId = 0;

    //clear undo stack since this action is not undoable
    QUndoStack *stack = MainWindow::getUndoStack();
    if (stack) stack->clear();

    //set first collection as current one
    QModelIndex first = m_model->index(0, 1);
    if (first.isValid())
        setCurrentIndex(first);
    else
        MetadataEngine::getInstance().setCurrentCollectionId(0); //set invalid

    //set local data changed
    SyncSession::LOCAL_DATA_CHANGED = true;

    //FIXME: temporary workaround for listview not updating the items
    //needs investigation, caused by migration from Qt4 to Qt5
    this->detachModel();
    this->attachModel();
}

void CollectionListView::attachModel()
{
    //setup model
    m_model = new CollectionListModel(this);
    setModel(m_model);
    setModelColumn(1);

    //connections
    connect(m_model, SIGNAL(dataChanged(QModelIndex,QModelIndex,QVector<int>)),
            this, SLOT(editingFinished()));

    //select last used collection
    selectCurrentCollection();
}

void CollectionListView::detachModel()
{
    setModel(0);

    //delete model
    disconnect(m_model, SIGNAL(dataChanged(QModelIndex,QModelIndex,QVector<int>)),
               this, SLOT(editingFinished()));
    if (m_model) {
        delete m_model;
        m_model = 0;
    }
}


//-----------------------------------------------------------------------------
// Protected
//-----------------------------------------------------------------------------

void CollectionListView::currentChanged(const QModelIndex &current,
                                        const QModelIndex &previous)
{
    QListView::currentChanged(current, previous);

    //set current collection on selection change
    if (current.isValid())
        collectionSelected(current);
}

void CollectionListView::contextMenuEvent(QContextMenuEvent *event)
{
    QMenu menu(this);
    menu.addAction(m_newCollectionAction);
    if (MetadataEngine::getInstance().getCurrentCollectionId() != 0)
        menu.addAction(m_deleteCollectionAction);
    menu.exec(event->globalPos());
}


//-----------------------------------------------------------------------------
// Private slots
//-----------------------------------------------------------------------------

void CollectionListView::collectionSelected(const QModelIndex &index)
{
    if (!index.isValid()) return;
    int collectionId = index.model()->index(index.row(), 0).data().toInt();

    if (collectionId == m_currentCollectionId) return;

    //create undo command
    if (m_currentCollectionId != 0) { //only if valid
        QUndoStack *stack = MainWindow::getUndoStack();
        if (stack) {
            stack->push(new ChangeCollectionCommand(m_currentCollectionId,
                                                    collectionId));
        }
    }

    MetadataEngine::getInstance().setCurrentCollectionId(collectionId);
    //metadata engine will emit a signal to notify current collection change
}

void CollectionListView::editingFinished()
{
    selectCurrentCollection();
}

void CollectionListView::newCollectionActionTriggered()
{
    createNewCollection();
}

void CollectionListView::deleteCollectionActionTriggered()
{
    deleteCollection();
}

void CollectionListView::currentCollectionIdChanged(int collectionId)
{
    if (collectionId != m_currentCollectionId) {
        m_currentCollectionId = collectionId;
        selectCurrentCollection();
    }
}


//-----------------------------------------------------------------------------
// Private
//-----------------------------------------------------------------------------

void CollectionListView::selectCurrentCollection()
{
    int id = MetadataEngine::getInstance().getCurrentCollectionId();
    m_currentCollectionId = id;

    //get the index for the collection id
    Qt::MatchFlags flags = Qt::MatchFlags( Qt::MatchExactly | Qt::MatchWrap );
    QModelIndexList indexList = m_model->match(m_model->index(0,0), Qt::DisplayRole,
                                               id, flags);

    //select
    if (!indexList.isEmpty()) {
        int row = indexList.at(0).row();
        setCurrentIndex(m_model->index(row, 1));
    }
}
