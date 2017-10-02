/*
 *  Copyright (c) 2012 Giorgio Wicklein <giowckln@gmail.com>
 */

//-----------------------------------------------------------------------------
// Hearders
//-----------------------------------------------------------------------------

#include "formview.h"
#include "droprectwidget.h"
#include "selectrectwidget.h"
#include "resizedotwidget.h"
#include "../../components/formlayoutmatrix.h"
#include "../../components/databasemanager.h"
#include "../../components/settingsmanager.h"
#include "../../widgets/form_widgets/testformwidget.h"
#include "../../widgets/form_widgets/textformwidget.h"
#include "../../widgets/form_widgets/numberformwidget.h"
#include "../../widgets/form_widgets/dateformwidget.h"
#include "../../widgets/form_widgets/creationdateformwidget.h"
#include "../../widgets/form_widgets/moddateformwidget.h"
#include "../../widgets/form_widgets/checkboxformwidget.h"
#include "../../widgets/form_widgets/comboboxformwidget.h"
#include "../../widgets/form_widgets/progressformwidget.h"
#include "../../widgets/form_widgets/imageformwidget.h"
#include "../../widgets/form_widgets/filesformwidget.h"
#include "../../widgets/form_widgets/urlformwidget.h"
#include "../../widgets/form_widgets/emailformwidget.h"
#include "../../widgets/mainwindow.h"
#include "../../models/standardmodel.h"
#include "emptyformwidget.h"
#include "../../components/undocommands.h"
#include "../../utils/formviewlayoutstate.h"
#include "../../components/sync_framework/syncsession.h"

#include <QtCore/QPropertyAnimation>
#include <QtWidgets/QScrollBar>
#include <QtGui/QMouseEvent>
#include <QtWidgets/QApplication>
#include <QtWidgets/QStatusBar>
#include <QtGui/QContextMenuEvent>
#include <QtWidgets/QAction>
#include <QtWidgets/QMenu>
#include <QtWidgets/QUndoCommand>
#include <QtWidgets/QUndoStack>


//-----------------------------------------------------------------------------
// Public
//-----------------------------------------------------------------------------

FormView::FormView(QWidget *parent) :
    QAbstractItemView(parent), m_widthUnitPx(200), m_heightUnitPx(80),
    m_idealWidth(0), m_idealHeight(0), m_sizeIsDirty(true),
    m_isAnimating(false), m_isMovingFW(false), m_dropRectWidget(0),
    m_isSelectedFW(false), m_selectRectWidget(0), m_horizontalResizeGrip(0),
    m_verticalResizeGrip(0), m_isResizingFW(false), m_currentRow(-1),
    m_currentColumn(-1), m_emptyFormWidget(0), m_modifiedTrigger(false)
{
    initFormView();
    createContextActions();
}

FormView::~FormView()
{
    delete m_formLayoutMatrix;
}

QRect FormView::visualRect(const QModelIndex &index) const
{
    if (index.isValid()) {
        int column = index.column();
        if (column > 0) { //column 0 (ID) is not valid here
            return m_formWidgetList.at(column - 1)->geometry(); //-1 because column ID has not a form widget
        }
    }
    //else
    return QRect();
}

void FormView::scrollTo(const QModelIndex &index, ScrollHint hint)
{
    Q_UNUSED(hint);

    QRect area = viewport()->rect();
    QRect rect = visualRect(index);

    if (rect.left() < area.left())
        horizontalScrollBar()->setValue(
                    horizontalScrollBar()->value() + rect.left() - area.left());
    else if (rect.right() > area.right())
        horizontalScrollBar()->setValue(
                    horizontalScrollBar()->value() + qMin(
                        rect.right() - area.right(), rect.left() - area.left()));

    if (rect.top() < area.top())
        verticalScrollBar()->setValue(
                    verticalScrollBar()->value() + rect.top() - area.top());
    else if (rect.bottom() > area.bottom())
        verticalScrollBar()->setValue(
                    verticalScrollBar()->value() + qMin(
                        rect.bottom() - area.bottom(), rect.top() - area.top()));

    viewport()->update();
}

QModelIndex FormView::indexAt(const QPoint &point) const
{
    QModelIndex index;

    QWidget *w = viewport()->childAt(point);
    if (w) {
        //use parent for cast because childAt() returns the widget
        //at the given pos, which could be for example a QLabel,
        //so we get its parent (which is the form widget)
        FormWidget* fw = qobject_cast<FormWidget*>(w->parent());
        if (fw) {
            int column = fw->getFieldId();
            if (column != -1) {
                index = model()->index(m_currentRow, column);
            }
        }
    }


    return index;
}

void FormView::setModel(QAbstractItemModel *model)
{
    m_currentRow = -1; //set current row (item) invalid
    QAbstractItemView::setModel(model);

    //if model is valid
    if (model) {
        //connections
        createModelConnections(model);

        //init view
        createFields();

        //set initial row (item)
        if(model->rowCount() > 0) {
            m_currentRow = 0; //start with first item
            updateSelectionModel();
        }

        populateFields();
        renderFormLayoutMatrix();
    } else {
        removeFields(); //if any from previous workflow
    }

    updateEmptyState();
    clearFormWidgetSelection();
}

int FormView::getCurrentRow()
{
    return m_currentRow;
}

int FormView::getCurrentColumn()
{
    return m_currentColumn;
}

int FormView::getSelectedField()
{
    int id = -1; //-1 stands for no selection

    if (m_isSelectedFW) {
        id = m_formWidgetList.indexOf(m_selectedFW) + 1; //because column 0 is ID
        if (id == 0) id = -1; //if fw was not found (-1 +1 = 0)
    }

    return id;
}

void FormView::setFormLayoutState(FormViewLayoutState &state)
{
    FormLayoutMatrix *matrix = state.toFormLayoutMatrix(m_formWidgetList);
    if (matrix) {
        delete m_formLayoutMatrix;
        m_formLayoutMatrix = matrix;
        renderFormLayoutChange(m_formLayoutMatrix);
    }
}

void FormView::reloadAppearanceSettings()
{
    setupViewBackground();
}


//-----------------------------------------------------------------------------
// Protected slots
//-----------------------------------------------------------------------------

void FormView::navigateNextRecord()
{
    if (!model()) return;

    //set focus, so content of form widgets
    //is saved (focusOutEvent triggers editingFinished)
    setFocus();

    bool hasNext;
    int next = m_currentRow + 1;

    if (next >= model()->rowCount()) {
        if (model()->canFetchMore(QModelIndex())) {
            model()->fetchMore(QModelIndex());
            hasNext = true;
        } else {
            hasNext = false;
        }
    } else {
        hasNext = true;
    }

    if (hasNext) {
        m_currentRow = next;
        populateFields();
    }

    updateSelectionModel();

    //show current record number on status bar
    MainWindow::getStatusBar()->showMessage(
                tr("Record %1 of %2").arg(m_currentRow + 1)
                .arg(model()->rowCount()));
}

void FormView::navigatePreviousRecord()
{
    if (!model()) return;

    //set focus, so content of form widgets
    //is saved (focusOutEvent triggers editingFinished)
    setFocus();

    int previous = m_currentRow - 1;
    if ((previous >= 0) && (previous < model()->rowCount())) {
        m_currentRow = previous;
        populateFields();
    }

    updateSelectionModel();

    //show current record number on status bar
    MainWindow::getStatusBar()->showMessage(
                tr("Record %1 of %2").arg(m_currentRow + 1)
                .arg(model()->rowCount()));
}

void FormView::navigateToRecord(int record)
{
    //set focus, so content of form widgets
    //is saved (focusOutEvent triggers editingFinished)
    setFocus();

    if ((record >= 0) && (record < model()->rowCount())) {
        m_currentRow = record;
        populateFields();
    }

    updateSelectionModel();

    //show current record number on status bar
    MainWindow::getStatusBar()->showMessage(
                tr("Record %1 of %2").arg(m_currentRow + 1)
                .arg(model()->rowCount()));
}

void FormView::updateLastModified(int startRow, int endRow)
{
    if (!m_modifiedTrigger) return;
    if (!model()) return;

    QModelIndex index;
    int fieldCount;
    int column;

    //update last modified date for all edited records
    for (int r = startRow; r <= endRow; r++) {
        fieldCount = m_modFieldList.size();
        for (int i = 0; i < fieldCount; i++) {
            column = m_modFieldList.at(i);
            index = model()->index(r, column);
            model()->setData(index, QDateTime::currentDateTime());
        }
    }
}


//-----------------------------------------------------------------------------
// Protected slots
//-----------------------------------------------------------------------------

void FormView::dataChanged(const QModelIndex &topLeft,
                           const QModelIndex &bottomRight,
                           const QVector<int> &roles)

{
    QAbstractItemView::dataChanged(topLeft, bottomRight, roles);

    int start = topLeft.row();
    int end = bottomRight.row();

    //check if the current displayed item is in the range where edits happened
    if ((start <= m_currentRow) && (end >= m_currentRow)) {
        populateFields();
    }
}

void FormView::currentChanged(const QModelIndex &current,
                              const QModelIndex &previous)
{
    QAbstractItemView::currentChanged(current, previous);

    if (current.isValid()) {
        m_currentRow = current.row();
        populateFields();
    }
}

void FormView::rowsInserted(const QModelIndex &parent, int start, int end)
{
    QAbstractItemView::rowsInserted(parent, start, end);

    updateEmptyState();
}

void FormView::contextMenuEvent(QContextMenuEvent *event)
{
    QMenu menu(this);
    menu.addAction(m_newRecordContextAction);
    menu.addAction(m_duplicateRecordContextAction);
    menu.addAction(m_deleteRecordContextAction);
    menu.addSeparator();
    menu.addAction(m_newFieldContextAction);
    if (m_isSelectedFW) {
        menu.addAction(m_modifyFieldContextAction);
        menu.addAction(m_duplicateFieldContextAction);
        menu.addAction(m_deleteFieldContextAction);
    }
    menu.exec(event->globalPos());
}

void FormView::keyPressEvent(QKeyEvent *event)
{
    QAbstractItemView::keyPressEvent(event);

    //clear selection and focus
    if (event->key() == Qt::Key_Escape) {
        clearFormWidgetSelection();
        this->setFocus();
    }
}


//-----------------------------------------------------------------------------
// Protected
//-----------------------------------------------------------------------------

QModelIndex FormView::moveCursor(QAbstractItemView::CursorAction cursorAction,
                                 Qt::KeyboardModifiers modifiers)
{
    Q_UNUSED(modifiers);
    QModelIndex current;

    switch (cursorAction) {
    case MoveLeft:
    case MoveUp:
        navigatePreviousRecord();
        break;
    case MoveRight:
    case MoveDown:
        navigateNextRecord();
        break;
    case MoveHome:
    case MovePageUp:
        navigateToRecord(0);
        break;
    case MoveEnd:
    case MovePageDown:
        navigateToRecord(model()->rowCount() - 1);
        break;
    case MoveNext:
    case MovePrevious:
        //skip
        break;
    }

    current = currentIndex();
    return current;
}

int FormView::horizontalOffset() const
{
    //offset where viewport starts is just the value of scrollbars
    return horizontalScrollBar()->value();
}

int FormView::verticalOffset() const
{
    //offset where viewport starts is just the value of scrollbars
    return verticalScrollBar()->value();
}

void FormView::scrollContentsBy(int dx, int dy)
{
    if (m_isAnimating) {
        //update running animations because coordinates change on scroll
        updateAnimations();
    }

    scrollDirtyRegion(dx, dy);
    viewport()->scroll(dx, dy);
}

bool FormView::isIndexHidden(const QModelIndex &index) const
{
    if (index.column() == 0) //ID is hidden
        return true;
    else
        return false;
}

void FormView::setSelection(const QRect &rect, QItemSelectionModel::SelectionFlags command)
{    
    QModelIndex index = indexAt(rect.topLeft()); //support only single selection
    if (index.isValid()) {
        selectionModel()->setCurrentIndex(index, command);
    }
}

QRegion FormView::visualRegionForSelection(const QItemSelection &selection) const
{
    int ranges = selection.count();

    if (ranges == 0)
        return QRect();

    QRegion region;
    for (int i = 0; i < ranges; ++i) {
        QItemSelectionRange range = selection.at(i);
        for (int row = range.top(); row <= range.bottom(); ++row) {
            for (int col = range.left(); col <= range.right(); ++col) {
                QModelIndex index = model()->index(row, col, rootIndex());
                region += visualRect(index);
            }
        }
    }

    return region;
}

void FormView::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event);

    updateSize();
}

void FormView::mousePressEvent(QMouseEvent *event)
{
    //set start drag pos for a possible drag operation
    //the position is recorded so that in mouseMoveEvent
    //it is possible to distinguish mouse clicks from
    //mouse drag events
    if (event->button() == Qt::LeftButton)
        m_dragStartPosition = event->pos();

    //if for any strange reason there is an incomplete
    //resize operation, drop it
    if (m_isResizingFW) {
        clearFormWidgetResize();
        clearFormWidgetSelection();
    }

    //check if the user clicked on a resize grip
    //if so, start widget resize operation
    if (m_isSelectedFW) {
        //get the widget where the click happened
        QWidget *w = viewport()->childAt(event->pos());
        if (w) {
            //when the user clicks on a grip it really clicks on the QLabel in ResizeDotWidget
            //so we get the parent and make sure it is a resize grip
            QWidget *parent = w->parentWidget();
            if ((parent == m_horizontalResizeGrip) || (parent == m_verticalResizeGrip)) {
                //stop animations if there are any currently running
                stopAnimations();
                if (parent == m_horizontalResizeGrip)
                    m_horizontalResizeGrip->isResizing = true;
                else
                    m_verticalResizeGrip->isResizing = true;
                m_isResizingFW = true;

                //make sure the selection rect is visible above all
                m_selectedFW->lower();
            }
        }
    } else {
        clearFormWidgetSelection();
    }

    if (!m_isResizingFW) {
        //select current widget, to allow FW resize
        if ((event->button() == Qt::LeftButton) || (event->button() == Qt::RightButton)) {
            int row, column;
            translateViewCoordsToMatrixIndex(event->pos(), row, column);
            selectFormWidget(row, column);
        }
    }
}
void FormView::mouseMoveEvent(QMouseEvent *event)
{
    //TODO: do some optimizations/clean up for code reuse in this method
    bool startFWDrag = false;

    //if there is not active drag/resize operation
    if (!m_isMovingFW && !m_isResizingFW) {
        //check if a drag operation should be started
        if (event->buttons() & Qt::LeftButton) {
            //to distinguish between mouse click and drag events
            //the start drag position is compared to the minimum required
            //distance for a drag operation
            if ((event->pos() - m_dragStartPosition).manhattanLength()
                    >= QApplication::startDragDistance()) {
                startFWDrag = true;
            }
        }
    }

    //if necessary create a new drag operation
    if (startFWDrag) {
        //stop animations if there are any currently running
        stopAnimations();

        //remove selection if any
        clearFormWidgetSelection();

        //get form widget which is being dragged
        int row, column;
        translateViewCoordsToMatrixIndex(event->pos(), row, column);
        FormWidget *fw = m_formLayoutMatrix->getFormWidget(row, column);

        //if the index is an extension cell, get the its parent FW
        if (fw == (FormWidget*) FormLayoutMatrix::EXTENDED_FORM_WIDGET) {
            fw = m_formLayoutMatrix->getFormWidgetByExtended(row, column);
        }

        if ((fw != NULL) &&
            (fw != (FormWidget*) FormLayoutMatrix::NO_FORM_WIDGET)) {
            m_isMovingFW = true; //set form widget drag/move operation
            m_movingFW = fw;
            m_moveFWHotSpot = event->pos() - fw->pos();
            this->setCursor(Qt::ClosedHandCursor);
        }
    }

    //if there is an active FW drag/move operation
    if (m_isMovingFW) {
        QPoint mousePos = event->pos();
        int targetX, targetY;
        targetX = mousePos.x() - m_moveFWHotSpot.x();
        targetY = mousePos.y() - m_moveFWHotSpot.y();
        m_movingFW->move(targetX, targetY);

        //create/update a drop rect to mark the current target cell
        if (!m_dropRectWidget) {
            m_dropRectWidget = new DropRectWidget(viewport());
        }
        int row, column;
        translateViewCoordsToMatrixIndex(event->pos(), row, column);

        //here the column/row is adjusted depending on
        //where the user started the drag operation
        column -= (m_moveFWHotSpot.x()/m_widthUnitPx);
        row -= (m_moveFWHotSpot.y()/m_heightUnitPx);

        //if the current row or column is beyond matrix size
        //step column/row back to max allowed + 1
        //so a new/row is created to fit the FW
        if (row >= m_formLayoutMatrix->rowCount()) {
            row = m_formLayoutMatrix->rowCount();
        }
        if (column >= m_formLayoutMatrix->columnCount()) {
            column = m_formLayoutMatrix->columnCount();
        }

        QRect cellRect = translateMatrixIndexToViewCoords(m_movingFW->getWidthUnits(),
                                                          m_movingFW->getHeightUnits(),
                                                          row, column);
        //add 2px to the rect to avoid overlapping
        QRect dropRect;
        dropRect.setTopLeft(cellRect.topLeft() + QPoint(-2, -2));
        dropRect.setBottomRight(cellRect.bottomRight() + QPoint(+2, +2));

        m_dropRectWidget->setGeometry(dropRect);
        m_dropRectWidget->show();
        //set the drop widget to be above all other widgets if they overlap
        m_dropRectWidget->raise();

        //handle scrolling during move
        int xScrollBarValue = horizontalScrollBar()->value();
        int yScrollBarValue = verticalScrollBar()->value();
        int xMax = horizontalScrollBar()->maximum();
        int yMax = verticalScrollBar()->maximum();

        if (yScrollBarValue < yMax) {
            if ((viewport()->rect().bottom() - event->pos().y()) < 20)
                verticalScrollBar()->setValue(yScrollBarValue + 5); //scroll down
        }
        if (yScrollBarValue > 0) {
            if ((viewport()->rect().top() + event->pos().y()) < 20)
                verticalScrollBar()->setValue(yScrollBarValue - 5); //scroll up
        }
        if (xScrollBarValue < xMax) {
            if ((viewport()->rect().right() - event->pos().x()) < 20)
                horizontalScrollBar()->setValue(xScrollBarValue + 5); //scroll right
        }
        if (xScrollBarValue > 0) {
            if ((viewport()->rect().left() + event->pos().x()) < 20)
                horizontalScrollBar()->setValue(xScrollBarValue - 5); //scroll left
        }

    } else if (m_isResizingFW) { //if there's an active drag resize operation
        //update selection rect to mark target size
        int startRow, startColumn, endRow, endColumn;
        m_formLayoutMatrix->findFormWidgetIndex(m_selectedFW, startRow, startColumn);
        translateViewCoordsToMatrixIndex(event->pos(), endRow, endColumn);

        int newWidthUnits, newHeightUnits;
        newWidthUnits = endColumn - startColumn + 1; //+1 because size starts by 1 and not by 0
        newHeightUnits = endRow - startRow + 1;
        if (newWidthUnits < 1) newWidthUnits = 1; //size minimum is 1
        if (newHeightUnits < 1) newHeightUnits = 1;

        QRect cellRect;
        //whether horizontal resize or vertical resize
        if (m_horizontalResizeGrip->isResizing) { //horizontal
            cellRect = translateMatrixIndexToViewCoords(newWidthUnits,
                                                        m_selectedFW->getHeightUnits(),
                                                        startRow, startColumn);
            //move grip to mouse position
            m_horizontalResizeGrip->move(event->x() - (m_horizontalResizeGrip->geometry().width()/2),
                                         m_horizontalResizeGrip->y());
        } else { //vertical
            cellRect = translateMatrixIndexToViewCoords(m_selectedFW->getWidthUnits(),
                                                        newHeightUnits,
                                                        startRow, startColumn);
            //move grip to mouse position
            m_verticalResizeGrip->move(m_verticalResizeGrip->x(),
                                       event->y() - (m_verticalResizeGrip->geometry().height()/2));
        }

        //add 2px to the rect to avoid overlapping
        QRect selectRect;
        selectRect.setTopLeft(cellRect.topLeft() + QPoint(-2, -2));
        selectRect.setBottomRight(cellRect.bottomRight() + QPoint(+2, +2));
        m_selectRectWidget->setGeometry(selectRect);
        m_selectRectWidget->show();
    } else {
        QAbstractItemView::mouseMoveEvent(event);
    }
}

void FormView::mouseReleaseEvent(QMouseEvent *event)
{
    //if there is an active FW drag/move operation
    if (m_isMovingFW) {
        m_isMovingFW = false;

        //hide drop rect
        if (m_dropRectWidget)
            m_dropRectWidget->hide();

        this->setCursor(Qt::ArrowCursor);

        int row, column;

        //get the target coords where the widget will be moved
        //2px added to geometry because in move event 2px were added to avoid overlapping
        //of the dropRect with the underlying FW
        translateViewCoordsToMatrixIndex(QPoint(m_dropRectWidget->geometry().left()+2,
                                                m_dropRectWidget->geometry().y()+2),
                                         row, column);

        //save old layout
        FormViewLayoutState oldState(m_formLayoutMatrix, m_formWidgetList);

        //do drag/movement layout change with creation of new rows/cols if needed
        m_formLayoutMatrix->formWidgetMovement(m_movingFW, row, column);

        //save new layout
        FormViewLayoutState newState(m_formLayoutMatrix, m_formWidgetList);

        //set local data changed
        SyncSession::LOCAL_DATA_CHANGED = true;

        //create undo action
        QUndoStack *stack = MainWindow::getUndoStack();
        if (stack) {
            QUndoCommand *cmd = new FormLayoutChangeCommand(oldState,
                                                            newState,
                                                            this);
            stack->push(cmd);
        }

        m_movingFW = NULL;

        //render the changes with animations
        renderFormLayoutChange(m_formLayoutMatrix);

    } else if (m_isResizingFW) { //if tehere's an active drag resize  operation
        //calc the new FW size and set it up
        int startRow, startColumn, endRow, endColumn;
        m_formLayoutMatrix->findFormWidgetIndex(m_selectedFW, startRow, startColumn);
        translateViewCoordsToMatrixIndex(event->pos(), endRow, endColumn);

        int newWidthUnits, newHeightUnits;
        newWidthUnits = endColumn - startColumn + 1; //+1 because size starts by 1 and not by 0
        newHeightUnits = endRow - startRow + 1;
        if (newWidthUnits < 1) newWidthUnits = 1; //size minimum is 1
        if (newHeightUnits < 1) newHeightUnits = 1;

        //resizing goes only in one direction at time
        //so we restore the old size for the direction
        //which is not being changed
        if (m_horizontalResizeGrip->isResizing)
            newHeightUnits = m_selectedFW->getHeightUnits();
        if (m_verticalResizeGrip->isResizing)
            newWidthUnits = m_selectedFW->getWidthUnits();

        //save old layout
        FormViewLayoutState oldState(m_formLayoutMatrix, m_formWidgetList);

        //do resize op
        m_formLayoutMatrix->formWidgetResize(m_selectedFW, newWidthUnits, newHeightUnits);

        //save new layout
        FormViewLayoutState newState(m_formLayoutMatrix, m_formWidgetList);

        //set local data changed
        SyncSession::LOCAL_DATA_CHANGED = true;

        //create undo action
        QUndoStack *stack = MainWindow::getUndoStack();
        if (stack) {
            QUndoCommand *cmd = new FormLayoutChangeCommand(oldState,
                                                            newState,
                                                            this);
            stack->push(cmd);
        }

        //clear
        clearFormWidgetResize();
        clearFormWidgetSelection();

        //render the changes with animations
        renderFormLayoutChange(m_formLayoutMatrix);
    } else {
        QAbstractItemView::mouseReleaseEvent(event);
    }
}

void FormView::mouseDoubleClickEvent(QMouseEvent *event)
{
    //generate mouse press event
    QAbstractItemView::mouseDoubleClickEvent(event);

    //if a FW is selected, start modify action
    if (m_isSelectedFW) {
        emit modifyFieldSignal();
    }
}


//-----------------------------------------------------------------------------
// Private slots
//-----------------------------------------------------------------------------

void FormView::animationsFinished()
{
    m_isAnimating = false;
    m_animationList.clear();
}

void FormView::formWidgetDataChanged()
{
    FormWidget *fw = NULL;
    QObject *s = sender();
    if (s)
        fw = qobject_cast<AbstractFormWidget*>(s);
    if (!fw) return;

    int column = fw->getFieldId();
    if (column == -1) return; //if fw not valid

    QModelIndex index = model()->index(m_currentRow, column);
    if (!index.isValid()) return;
    QVariant data = fw->getData();

    //create undo action
    QVariant oldData = index.data();
    QVariant newData = data;
    if (oldData != newData) {
        QUndoStack *stack = MainWindow::getUndoStack();
        if (stack) {
            ModRecordCommand *cmd = new ModRecordCommand(
                        m_metadataEngine->getCurrentCollectionId(),
                        index.row(), index.column(),
                        oldData, newData);
            stack->push(cmd);
        }

        //sync set local data changed
        SyncSession::LOCAL_DATA_CHANGED = true;

        //save data to model
        model()->setData(index, data);

        //emit view signal for data changes
        //not needed because already emitted by setData() call above
        //emit dataChanged(index, index);

        //update last modification field types, if any
        updateLastModified(m_currentRow, m_currentRow);
    }
}

void FormView::formWidgetRequireAttention(QString &message)
{
    FormWidget *fw = NULL;
    QObject *s = sender();
    if (s)
        fw = qobject_cast<AbstractFormWidget*>(s);
    if (!fw) return;

    //ensure form widget is visible
    ensureFormWidgetVisible(fw);

    //shake animation
    if (!m_isAnimating) {
        QPropertyAnimation *anim = new QPropertyAnimation(fw, "geometry", fw);
        QRect start = fw->geometry();
        QRect endLeft(start.x() - 25, start.y(), start.width(), start.height());
        QRect endRight(start.x() + 25, start.y(), start.width(), start.height());
        anim->setDuration(500);
        anim->setStartValue(start);
        anim->setKeyValueAt(0.3, endLeft);
        anim->setKeyValueAt(0.6, endRight);
        anim->setEndValue(start);
        anim->setEasingCurve(QEasingCurve::OutBack);
        m_animationList.append(anim);
        startQueuedAnimations();
    }

    MainWindow::getStatusBar()->showMessage(message);
}

void FormView::reloadModel()
{
    setModel(model());
}

void FormView::modelSorted(int column)
{
    //clear undo stack because row ids are not longer valid
    QUndoStack *stack = MainWindow::getUndoStack();
    if (stack) stack->clear();

    //because after sorting the current displayed
    //record is invalid, set current to 0 (first)
    m_currentRow = 0;
    selectionModel()->setCurrentIndex(model()->index(m_currentRow, column),
                                      QItemSelectionModel::SelectCurrent |
                                      QItemSelectionModel::Clear);
}

void FormView::rowsDeleted(int startRow, int count)
{
    //Review on new collection types,
    //assuming StandardModel only
    StandardModel *s = qobject_cast<StandardModel*>(model());
    int rowCount;
    if (s)
        rowCount = s->realRowCount();
    else
        rowCount = model()->rowCount();

    //select previous row as current
    if (rowCount) {
        int previousRow = startRow - 1;
        int nextRow = startRow + count - 1;

        if ((previousRow >= 0) && (previousRow < rowCount)) {
            m_currentRow = previousRow;
        } else { //select item after deleted rows
            if (nextRow < rowCount) {
                m_currentRow = nextRow;
            }
        }

        updateSelectionModel();
    } else { //model is empty
        updateEmptyState();
    }
}

void FormView::updateEmptyState()
{
    bool isEmpty;
    if (model())
        isEmpty = !model()->rowCount();
    else
        isEmpty = true;

    bool noFields = m_metadataEngine->getFieldCount() <= 1; //1 cause of _id
    bool noCollections = !m_metadataEngine->getCurrentCollectionId();

    if (isEmpty || noFields) { //now model is empty 
        //setup messages
        if (noCollections)
            m_emptyFormWidget->setState(EmptyFormWidget::AllMissing);
        else if (noFields)
            m_emptyFormWidget->setState(EmptyFormWidget::FieldMissing);
        else if (!model())
            m_emptyFormWidget->setState(EmptyFormWidget::MissingModel);
        else
            m_emptyFormWidget->setState(EmptyFormWidget::RecordMissing);

        m_emptyFormWidget->setVisible(true);
        m_emptyFormWidget->raise();
    } else { //now model is not empty
        m_emptyFormWidget->setVisible(false);
    }
}

void FormView::handleFocusChange(QWidget *old, QWidget *now)
{
    Q_UNUSED(old);

    if (viewport()->isAncestorOf(now)) {
        //use parent for cast because the widget
        //could be for example a QLabel,
        //so we get its parent (which is the form widget)
        FormWidget *fw = qobject_cast<FormWidget*>(now->parent());
        if (fw) {
            ensureFormWidgetVisible(fw);
        }
    }
}


//-----------------------------------------------------------------------------
// Private
//-----------------------------------------------------------------------------

void FormView::initFormView()
{
    m_formLayoutMatrix = new FormLayoutMatrix();
    m_metadataEngine = &MetadataEngine::getInstance();
    m_emptyFormWidget = new EmptyFormWidget(this); //child of this instead of viewport()
    m_emptyFormWidget->setVisible(false);

    horizontalScrollBar()->setRange(0, 0);
    verticalScrollBar()->setRange(0, 0);

    setSelectionMode(QAbstractItemView::SingleSelection);
    setEditTriggers(QAbstractItemView::NoEditTriggers);

    //catch focus change to keep up with current selection/index
    connect(qApp, SIGNAL(focusChanged(QWidget*,QWidget*)),
            this, SLOT(handleFocusChange(QWidget*,QWidget*)));

    setupViewBackground();
}

void FormView::createModelConnections(QAbstractItemModel *model)
{
    connect(model, SIGNAL(layoutChanged()),
            this, SLOT(reloadModel()));

    //for now the only supported model is StandardModel
    //review when more collection types are introduced
    StandardModel *s = qobject_cast<StandardModel*>(model);
    if (s) {
        connect(s, SIGNAL(rowsDeleted(int,int)),
                this, SLOT(rowsDeleted(int,int)));
        connect(s, SIGNAL(modelSortedSignal(int)),
                this, SLOT(modelSorted(int)));
    }
}

void FormView::setFormWidget(AbstractFormWidget *fw, int row, int column)
{
    QRect coords = translateMatrixIndexToViewCoords(fw->getWidthUnits(),
                                                    fw->getHeightUnits(),
                                                    row, column);
    fw->setGeometry(coords);
}

QRect FormView::translateMatrixIndexToViewCoords(int widthUnits,
                                                  int heightUnits,
                                                  int row,
                                                  int column)
{
    int width = widthUnits * m_widthUnitPx;
    int height = heightUnits * m_heightUnitPx;
    int x = (column * m_widthUnitPx) - horizontalOffset();
    int y = (row * m_heightUnitPx) - verticalOffset();

    QRect r;
    r.setRect(x, y, width, height);

    return r;
}

void FormView::translateViewCoordsToMatrixIndex(QPoint coords, int &row, int &column)
{
    row = (coords.y() + verticalOffset()) / m_heightUnitPx;
    column = (coords.x() + horizontalOffset()) / m_widthUnitPx;

    if (row < 0) row = 0;
    if (column < 0) column = 0;
}

void FormView::renderFormLayoutMatrix(FormLayoutMatrix *matrix)
{
    if (matrix == 0) matrix = m_formLayoutMatrix;

    //query form widgets from the layout matrix and render them on the View
    for (int r = 0; r < matrix->rowCount(); r++) {
        for (int c = 0; c < matrix->columnCount(); c++) {
            FormWidget* fw = matrix->getFormWidget(r, c);
            if ((fw != (FormWidget*)FormLayoutMatrix::NO_FORM_WIDGET) &&
                (fw != (FormWidget*)FormLayoutMatrix::EXTENDED_FORM_WIDGET) &&
                (fw != NULL)) {
                setFormWidget(fw, r, c);
            }
        }
    }
}

void FormView::renderFormLayoutChange(FormLayoutMatrix *newMatrix)
{
    clearFormWidgetSelection();

    for (int i = 0; i < newMatrix->rowCount(); i++) {
        for (int j = 0; j < newMatrix->columnCount(); j++) {
            FormWidget *fw = newMatrix->getFormWidget(i, j);
            if (m_formWidgetList.contains(fw)) { //filter out non widgets
                QPropertyAnimation *a = new QPropertyAnimation(fw, "geometry", fw);
                a->setDuration(500);
                a->setStartValue(fw->geometry());
                QRect coords = translateMatrixIndexToViewCoords(
                            fw->getWidthUnits(), fw->getHeightUnits(),
                            i, j);
                a->setEndValue(coords);
                //a->setEasingCurve(QEasingCurve::OutBounce); //not so good on slow PCs
                m_animationList.append(a);
            }
        }
    }

    startQueuedAnimations();

    //the layout may change so we need to update the widget size and scrollbars
    updateSize();
    updateTabOrder();

    //save coordinates and sizes of all fields
    //they are not saved individually because
    //when simplifyMatrix() is called in form
    //layout matrix, it is possible that more
    //widgets are moved at once, so save all
    //using db transaction to speed writes up
    DatabaseManager::getInstance().beginTransaction();
    saveFieldCoordinates();
    saveFieldFormLayoutSizes();
    DatabaseManager::getInstance().endTransaction();
}

void FormView::startQueuedAnimations()
{
    stopAnimations();
    m_isAnimating = true;

    foreach(QPropertyAnimation *anim, m_animationList) {
        anim->start(QAbstractAnimation::DeleteWhenStopped);
    }

    if (m_animationList.size() > 0) {
        QPropertyAnimation *lastAnim = m_animationList.last();
        connect(lastAnim, SIGNAL(finished()),
                this, SLOT(animationsFinished()));
    } else {
        m_isAnimating = false;
    }
}

void FormView::stopAnimations()
{
    if (m_isAnimating) {
        foreach(QPropertyAnimation *anim, m_animationList) {
            QWidget *p = static_cast<QWidget*>(anim->parent());
            QRect endGeometry = anim->endValue().toRect();
            anim->stop();
            p->setGeometry(endGeometry);

        }
        m_animationList.clear();
        m_isAnimating = false;
    }
}

void FormView::updateAnimations()
{
    if (m_isAnimating) {
        foreach(QPropertyAnimation *anim, m_animationList) {
            FormWidget *fw = static_cast<FormWidget*>(anim->parent());
            int row, column;
            if (m_formLayoutMatrix->findFormWidgetIndex(fw, row, column)) {
                QRect newCoords = translateMatrixIndexToViewCoords(
                                  fw->getWidthUnits(), fw->getHeightUnits(),
                                  row, column);
                anim->setEndValue(newCoords);
            } else {
                anim->stop();
            }
        }
    }
}

void FormView::updateScrollBars()
{
    horizontalScrollBar()->setSingleStep(m_widthUnitPx);
    horizontalScrollBar()->setPageStep(viewport()->width());
    horizontalScrollBar()->setRange(0, qMax(0, m_idealWidth - viewport()->width()));

    verticalScrollBar()->setSingleStep(m_heightUnitPx);
    verticalScrollBar()->setPageStep(viewport()->height());
    verticalScrollBar()->setRange(0, qMax(0, m_idealHeight - viewport()->height()));

}

void FormView::calculateSizeIfNeeded()
{
    if (m_sizeIsDirty) {
        int rows = m_formLayoutMatrix->rowCount();
        int columns = m_formLayoutMatrix->columnCount();

        m_idealHeight = rows * m_heightUnitPx + 2;
        m_idealWidth = columns * m_widthUnitPx + 2;

        //the actual widget size is never set because only the viewport (what is visible)
        //matters and the scrollbars are calculated from the ideal sizes
    }
}

void FormView::selectFormWidget(int row, int column)
{
    //create selection rectangle
    if (!m_selectRectWidget) {
        m_selectRectWidget = new SelectRectWidget(viewport());
    }

    //if the index is not valid remove previous selection
    if ((row >= m_formLayoutMatrix->rowCount()) ||
            (column >= m_formLayoutMatrix->columnCount())) {
        clearFormWidgetSelection();
        return;
    }

    FormWidget *fw = m_formLayoutMatrix->getFormWidget(row, column);

    //if the index is an extension cell, get the its parent FW
    if (fw == (FormWidget*) FormLayoutMatrix::EXTENDED_FORM_WIDGET) {
        fw = m_formLayoutMatrix->getFormWidgetByExtended(row, column);
        m_formLayoutMatrix->findFormWidgetIndex(fw, row, column);
    }

    if ((fw != NULL) &&
        (fw != (FormWidget*) FormLayoutMatrix::NO_FORM_WIDGET)) {
        stopAnimations(); //in case there are some running
        QRect cellRect = translateMatrixIndexToViewCoords(fw->getWidthUnits(),
                                                          fw->getHeightUnits(),
                                                          row, column);
        QRect selectionRect;
        selectionRect.setTopLeft(cellRect.topLeft() + QPoint(-2, -2));
        selectionRect.setBottomRight(cellRect.bottomRight() + QPoint(+2, +2));

        m_selectRectWidget->setGeometry(selectionRect);
        m_selectRectWidget->show();
        //set the widget to be under the covered widget,
        //so if selection is active the user can still click
        //on the FW at the given cell
        m_selectRectWidget->lower();
        //set the selected widget above all others
        fw->raise();
        m_selectedFW = fw;
    } else {
        clearFormWidgetSelection();
        return;
    }

    m_isSelectedFW = true;


    //when a FW is selected, show 2 resize grips
    //on the right and bottom center
    if (!m_horizontalResizeGrip)
        m_horizontalResizeGrip = new ResizeDotWidget(viewport());
    if (!m_verticalResizeGrip)
        m_verticalResizeGrip = new ResizeDotWidget(viewport());

    //deltaPx = additional pixel used to position the resize widgets in the setGeometry method
    //deltaPx is subtracted from the x/y coordinate to move the resize grips closer to the respective FW
    int deltaPx;
#ifdef Q_OS_OSX
    deltaPx = 8;
#else
    deltaPx = 5;
#endif

    m_horizontalResizeGrip->setGeometry((fw->geometry().right() - deltaPx),
                                        (fw->geometry().y() +
                                            (((m_heightUnitPx * fw->getHeightUnits()) / 2)) - 3),
                                        8, 8);
    m_horizontalResizeGrip->setCursor(Qt::SizeHorCursor);
    m_horizontalResizeGrip->show();
    m_horizontalResizeGrip->raise(); //move resize grip above all widgets

    m_verticalResizeGrip->setGeometry((((fw->geometry().right() -
                                            (fw->geometry().width() / 2))) - 2),
                                      (fw->geometry().bottom() - deltaPx), 8, 8);
    m_verticalResizeGrip->setCursor(Qt::SizeVerCursor);
    m_verticalResizeGrip->show();
    m_verticalResizeGrip->raise(); //move resize grip above all widgets
}

void FormView::clearFormWidgetSelection()
{
    if (m_isSelectedFW) {
        m_selectRectWidget->hide();
        m_selectedFW = NULL;
        m_isSelectedFW = false;
        m_horizontalResizeGrip->hide();
        m_verticalResizeGrip->hide();
    }
}

void FormView::clearFormWidgetResize()
{
    if (m_isResizingFW) {
        m_isResizingFW = false;
        m_horizontalResizeGrip->isResizing = false;
        m_verticalResizeGrip->isResizing = false;
    }
}

void FormView::createFields()
{
    //delete all fields if any
    removeFields();

    //create a form widget for every column
    //of current collection
    int fieldCount = m_metadataEngine->getFieldCount();
    for (int i = 1; i < fieldCount; i++) { //starting by 1 because ID is 0
        MetadataEngine::FieldType type = m_metadataEngine->getFieldType(i);
        FormWidget* fw = createFormWidget(type, i);
        fw->hide();

        //setup form widget metadata display proprieties if needed
        initFormWidget(fw, type, i);

        //add to form widget list
        m_formWidgetList.append(fw);

        //connect fw, so data changes are saved in the model
        connect(fw, SIGNAL(dataEdited()),
                this, SLOT(formWidgetDataChanged()));

        //handle attention requests by animating the widget
        connect(fw, SIGNAL(requiresAttention(QString&)),
                this, SLOT(formWidgetRequireAttention(QString&)));

        //set fw's size from metadata
        int w, h;
        m_metadataEngine->getFieldFormLayoutSize(i, w, h);
        if (w > 0) //-1 stands for not set (use default)
            fw->setWidthUnits(w);
        if (h > 0) //-1 stands for not set (use default)
            fw->setHeightUnits(h);

        //move form widget to the saved coordinate pos (if any)
        int x, y;
        bool v = m_metadataEngine->getFieldCoordinate(i, x, y);
        if (v) { //if pos was saved, set it
            m_formLayoutMatrix->setFormWidget(fw, y, x);
        } else { //else find best pos by algorithm
            m_formLayoutMatrix->addFormWidget(fw);
        }
    }

    //make form widgets visible since they are ready
    FormWidget* fw;
    foreach(fw, m_formWidgetList) {
        fw->show();
    }

    updateSize();
    updateTabOrder();
}

void FormView::removeFields()
{
    //stop animation if any
    //because parent widgets
    //will become invalid
    //and animationsFinished()
    //will fail
    stopAnimations();

    FormWidget* f;
    foreach (f, m_formWidgetList) {
        f->close();
        delete f;
    }

    //clear form layout matrix
    delete m_formLayoutMatrix;
    m_formLayoutMatrix = 0;
    m_formLayoutMatrix = new FormLayoutMatrix();

    //clear form widget list
    m_formWidgetList.clear();

    //reset modified trigger
    m_modifiedTrigger = false;
    //clear ModDateType fields
    m_modFieldList.clear();
}

void FormView::populateFields()
{
    //make sure  if row is valid
    if (m_currentRow == -1) {
        clearFields();
        return; //-1 means unset/invalid row
    }

    //load all data from model
    QModelIndex index;
    QAbstractItemModel *m = model();
    while (m->canFetchMore(index))
        m->fetchMore(index);

    FormWidget *fw;
    int column = 1; //0 is ID, so start with 1
    foreach (fw, m_formWidgetList) {
        index = m->index(m_currentRow, column);
        if (index.isValid()) {
            fw->setData(index.data());
        }
        column++;
    }
}

void FormView::clearFields()
{
    FormWidget* f;
    foreach (f, m_formWidgetList) {
        f->clearData();
    }
}

void FormView::updateTabOrder()
{
    int rows = m_formLayoutMatrix->rowCount();
    int columns = m_formLayoutMatrix->columnCount();
    QList<FormWidget*> orderedList;

    //get all form widgets in order
    for (int i = 0; i < rows; i++)
        for (int j = 0; j < columns; j++) {
            FormWidget *fw = m_formLayoutMatrix->getFormWidget(i, j);
            if ((fw != (FormWidget*) FormLayoutMatrix::NO_FORM_WIDGET) &&
                    (fw != (FormWidget*) FormLayoutMatrix::EXTENDED_FORM_WIDGET)) {
                orderedList.append(fw);
            }
        }

    //setup tab order
    //using focus proxy as workaround for bug QTBUG-10907
    int w = 0;
    if (orderedList.size() > 0) {
        //set initial order
        QWidget *proxy = orderedList.at(0)->focusProxy();
        if (proxy)
            setTabOrder(this, proxy);
        else
            setTabOrder(this, orderedList.at(0));
    }
    while (w < orderedList.size() - 1) {
        QWidget *widget = orderedList.at(w);
        QWidget *next = orderedList.at(++w);
        QWidget *proxy = widget->focusProxy();
        if (proxy)
            setTabOrder(proxy, next);
        else
            setTabOrder(widget, next);
    }
}

void FormView::initFormWidget(AbstractFormWidget *const fw,
                              MetadataEngine::FieldType type, const int column)
{
    //for now type is not used
    Q_UNUSED(type);

    //set field name
    fw->setFieldName(m_metadataEngine->getFieldName(column));

    //set field display metadata properties
    fw->loadMetadataDisplayProperties(
                m_metadataEngine->getFieldProperties(
                    MetadataEngine::DisplayProperty, column));
}

void FormView::saveFieldCoordinates()
{
    FormWidget* fw;
    foreach(fw, m_formWidgetList) {
        int row, column;
        bool v = m_formLayoutMatrix->findFormWidgetIndex(fw, row, column);
        if (!v) {
            row = column = -1; //set invalid pos if failed
        }

        m_metadataEngine->setFieldCoordinate(fw->getFieldId(), column, row);
    }
}

void FormView::saveFieldFormLayoutSizes()
{
    FormWidget* fw;
    foreach(fw, m_formWidgetList) {
        int w, h;
        w = fw->getWidthUnits();
        h = fw->getHeightUnits();
        int row, column;
        bool v = m_formLayoutMatrix->findFormWidgetIndex(fw, row, column);
        if (!v) {
            w = h = 1; //set min size if failed
        }
        m_metadataEngine->setFieldFormLayoutSize(fw->getFieldId(), w, h);
    }
}

AbstractFormWidget* FormView::createFormWidget(MetadataEngine::FieldType type,
                                               int columnId)
{
    FormWidget* f;

    switch (type) {
    case MetadataEngine::TextType:
        f = new TextFormWidget(viewport());
        break;
    case MetadataEngine::NumericType:
        f = new NumberFormWidget(viewport());
        break;
    case MetadataEngine::DateType:
        f = new DateFormWidget(viewport());
        break;
    case MetadataEngine::CreationDateType:
        f = new CreationDateFormWidget(viewport());
        break;
    case MetadataEngine::ModDateType:
        f = new ModDateFormWidget(viewport());
        m_modifiedTrigger = true; //modified trigger activation
        m_modFieldList.append(columnId); //add to list
        break;
    case MetadataEngine::CheckboxType:
        f = new CheckboxFormWidget(viewport());
        break;
    case MetadataEngine::ComboboxType:
        f = new ComboboxFormWidget(viewport());
        break;
    case MetadataEngine::ProgressType:
        f = new ProgressFormWidget(viewport());
        break;
    case MetadataEngine::ImageType:
        f = new ImageFormWidget(viewport());
        break;
    case MetadataEngine::FilesType:
        f = new FilesFormWidget(viewport());
        break;
    case MetadataEngine::URLTextType:
        f = new URLFormWidget(viewport());
        break;
    case MetadataEngine::EmailTextType:
        f = new EmailFormWidget(viewport());
        break;
    default:
        f = new TestFormWidget(viewport());
        break;
    }

    f->setFieldId(columnId);

    return f;
}

void FormView::updateSelectionModel()
{
    //update selection model, so TableView is also updated
    selectionModel()->setCurrentIndex(model()->index(m_currentRow, 1),
                                      QItemSelectionModel::SelectCurrent |
                                      QItemSelectionModel::Clear);
}

void FormView::ensureFormWidgetVisible(AbstractFormWidget *fw)
{
    //this is a bit hacky because form widgets are childs of viewport()
    //and fw->pos() would return viewport coordinates and not in form view
    //coordinates, so use a workaround to get the index

    QModelIndex index;
    int column = fw->getFieldId();
    if (column != -1) {
        m_currentColumn = column;
        index = model()->index(m_currentRow, column);
    }
    if (index.isValid())
        scrollTo(index);
}

void FormView::createContextActions()
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

void FormView::updateSize()
{
    //the widget (view) size is not valid anymore
    m_sizeIsDirty = true;

    calculateSizeIfNeeded();
    updateScrollBars();

    //update EmptyFormWidget's geometry
    m_emptyFormWidget->setGeometry(viewport()->geometry());
}

void FormView::setupViewBackground()
{
    SettingsManager sm;
    QString colorCode = sm.restoreProperty("backgroundColor", "formView").toString();

    //not using style sheets because on OS X scrolling stops working on the view
    //Qt bug workaround?
    QPalette p = QApplication::palette(this);
    if (!colorCode.isEmpty()) {
        p.setColor(QPalette::Base, QColor(colorCode));
    }

    setPalette(p);
}
