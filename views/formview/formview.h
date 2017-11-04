/**
  * \class FormView
  * \brief This class implements a View from Qt's Model/View framework.
  *        FormView presents the data from the model in a form-like view.
  * \author Giorgio Wicklein - GIOWISYS Software
  * \date 10/04/2012
  */

#ifndef FORMVIEW_H
#define FORMVIEW_H


//-----------------------------------------------------------------------------
// Headers
//-----------------------------------------------------------------------------

#include <QtWidgets/QAbstractItemView>
#include <QtCore/QList>

#include "../../components/metadataengine.h"


//-----------------------------------------------------------------------------
// Forward declarations
//-----------------------------------------------------------------------------

class FormLayoutMatrix;
class AbstractFormWidget;
class QPropertyAnimation;
class DropRectWidget;
class SelectRectWidget;
class ResizeDotWidget;
class EmptyFormWidget;
class QAction;
class FormViewLayoutState;


//-----------------------------------------------------------------------------
// FormView
//-----------------------------------------------------------------------------

class FormView : public QAbstractItemView
{
    Q_OBJECT

public:
    explicit FormView(QWidget *parent = 0);
    ~FormView();

    QRect visualRect(const QModelIndex &index) const;
    void scrollTo(const QModelIndex &index, ScrollHint hint = EnsureVisible);
    QModelIndex indexAt(const QPoint &point) const;

    /** Reimplemented to call custom view init methods after setting model */
    void setModel(QAbstractItemModel *model);

    /** Get the current row (record) */
    int getCurrentRow();

    /** Get the current column (field) */
    int getCurrentColumn();

    /** Get the id of the selected field, retuns -1 if none selected */
    int getSelectedField();

    /** Set form layout state to the specified layout matrix */
    void setFormLayoutState(FormViewLayoutState &state);

    /** This reloads all properties and settings related to form view's appearence */
    void reloadAppearanceSettings();

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

public slots:
    /** Set current item to next */
    void navigateNextRecord();

    /** Set current item to next */
    void navigatePreviousRecord();

    /** Set the specified record as current */
    void navigateToRecord(int record);

    /** Update all fields of type ModDateType to current time stamp (new mod date) */
    void updateLastModified(int startRow, int endRow);

protected slots:
     void dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight,
                      const QVector<int> &roles);
     void currentChanged(const QModelIndex &current, const QModelIndex &previous);
     void rowsInserted(const QModelIndex &parent, int start, int end);
     void contextMenuEvent(QContextMenuEvent *event);
     void keyPressEvent(QKeyEvent *event);

protected:
    QModelIndex moveCursor(QAbstractItemView::CursorAction cursorAction,
                           Qt::KeyboardModifiers modifiers);

    /** Pure virtual method from QAbstractItemView. It returns the x-offset
      * of the viewport within the (ideal-sized) widget.
      */
    int horizontalOffset() const;

    /** Pure virtual method from QAbstractItemView. It returns the y-offset
      * of the viewport within the (ideal-sized) widget.
      */
    int verticalOffset() const;

    /** Virtual method from QAbstractScrollArea. It is called when the scroll
      * bars are moved by dx, dy, so the viewport is scrolled.
      */
    void scrollContentsBy(int dx, int dy);

    bool isIndexHidden(const QModelIndex &index) const;
    void setSelection(const QRect &rect, QItemSelectionModel::SelectionFlags command);
    QRegion visualRegionForSelection(const QItemSelection &selection) const;

    /** Virtual method from QWidget which is called whenever the view
      * was resized, generating a resize event
      */
    void resizeEvent(QResizeEvent *event);

    /** This event is reimplemented because of custom move handling for FormWidgets */
    void mousePressEvent(QMouseEvent *event);

    /** This event is reimplemented because of custom move handling for FormWidgets */
    void mouseMoveEvent(QMouseEvent *event);

    /** This event is reimplemented because of custom move handling for FormWidgets */
    void mouseReleaseEvent(QMouseEvent *event);

    /** Handle double click to modify fields */
    void mouseDoubleClickEvent(QMouseEvent *event);

private slots:
    /** Called when the last animation from the animation list is done */
    void animationsFinished();

    /** Called when data changed in a form widget */
    void formWidgetDataChanged();

    /**
     * This slot shakes the widget through animations to
     * notify the user that the specified field requires
     * attention. Useful for example when an invalid input
     * is detected (by using edit triggers). The passed
     * message is shown on the status bar.
     */
    void formWidgetRequireAttention(QString &message);

    /** Reload current model. Useful for heavy changes in model */
    void reloadModel();

    /** Called if model has been sorted */
    void modelSorted(int column);

    /**
     * Inform FormView that the specified rows (records) have been deleted
     * @param startRow - the first row that has been deleted
     * @param count - the deleted rows count, beginning with startRow,
     *                1 means that only startRow has been deleted
     */
    void rowsDeleted(int startRow, int count);

    /**
     * This slot updates the state of the view. If the model is empty,
     * ie. no rows (records), then the form is marked as empty and all
     * fields are hidden and a static text is shown that no records available.
     * If the model is not empty the original state is restored.
     */
    void updateEmptyState();

    /**
     * This slot is connected to QApplications' focus changed signal.
     * If the newly focused widget is a FormWidget, then the current
     * index is adapted to fit the focus.
     */
    void handleFocusChange(QWidget *old, QWidget *now);

private:
    /** Initialization steps */
    void initFormView();

    /** Setup signal/slot connections */
    void createModelConnections(QAbstractItemModel *model);

    /** Set a form widget at the given index position on the FormView.
      * Since the matrix grid on the FormView is virtual, the real position
      * of the widget is calculated and then the widget is moved to the determined position.
      * The real layout matrix is hold by FormLayoutMatrix. This function is used
      * to render the form widget at the given position, which is mapped
      * from the real layout matrix.
      * @param fw the FormWidget that should be moved to row/column index
      * @param row the form layout matrix row which will be translated to FormView coordinates
      * @param column the form layout matrix column which will be translated to FormView coordinates
      */
    void setFormWidget(AbstractFormWidget* fw, int row, int column);

    /** Translate FormLayoutMatrix indexes to real FormView widget coordinates
      * @param widthUnits the widthUnits used by a form widget
      * @param heightUnits the heightUnits used by a from widget
      * @param row the row index from the matrix
      * @param column the column index from the matrix
      * @return QRect with the coordinates
      */
    QRect translateMatrixIndexToViewCoords(int widthUnits,
                                           int heightUnits,
                                           int row,
                                           int column
                                           );

    /** Translate real FormView widget coordinates to FormLayoutMatrix indexes
      * @param coords the QPoint which represents view coordinates
      * @param row an int reference where the calculated row is saved
      * @param column an int reference where the calculated column is saved
      */
    void translateViewCoordsToMatrixIndex(QPoint coords, int &row, int &column);

    /** Render a FormLayoutMatrix. The matrix is mapped with its form widgets to real coordinates
      * in the FormView.
      * @param matrix the matrix which should be rendered on FormView. If matrix==0 then
      *        the default member matrix m_formLayoutMatrix will be used.
      */
    void renderFormLayoutMatrix(FormLayoutMatrix* matrix=0);

    /** This function is used to handle all the transition animations to render
      * changes between current layout and new layout matrix
      * @param newMatrix the new matrix layout
      */
    void renderFormLayoutChange(FormLayoutMatrix* newMatrix);

    /** This executes all animation queued in the m_animationList */
    void startQueuedAnimations();

    /** This stops all active animations */
    void stopAnimations();

    /** Update the animations with new end position values. When scrolling,
      * SelectRectWidget the animations need to be updates because the end position coordinates
      * are not valid anymore.
      */
    void updateAnimations();

    /** This updates the range of the horizontal and vertical scrollbar */
    void updateScrollBars();

    /** Calculate or update the size (width, height) of the widget (view) */
    void calculateSizeIfNeeded();

    /** Set the selection state of the specified item at the
      * given row/column index, if the cell is empty, reset selection.
      * The selection state shows two grab handles to allow widget resizing.
      */
    void selectFormWidget(int row, int column);

    /** Clear current selection of any FormWidget */
    void clearFormWidgetSelection();

    /** Clear current resize operation for a FW */
    void clearFormWidgetResize();

    /** Create form widgets (fields) for the currently active collection */
    void createFields();

    /** Removes all form widgets (fields) from FormView */
    void removeFields();

    /** Populate form widgets (fields) for the current row (item) from model */
    void populateFields();

    /** Clear all form widgets (fields) */
    void clearFields();

    /** Updates the tab order according to form layout */
    void updateTabOrder();

    /**
     * This method sets display properties from metadata for the form widget
     * representing a given field. There are metadata properties that define
     * the appearance of fields, so this properties are applied here.
     * @param fw - the form widget representing the field at the given column
     * @param type - the data type of the field
     * @param column - the column number of the field represented by fw
     */
    void initFormWidget(AbstractFormWidget* const fw,
                        MetadataEngine::FieldType type, const int column);

    /**
     * This method saves the position (matrix coordinates)
     * of the all form widgets (fields) to metadata
     */
    void saveFieldCoordinates();

    /**
     * This method saves the sizes (width/height units)
     * of the all form widgets (fields) to metadata
     */
    void saveFieldFormLayoutSizes();

    /**
     * Factory method for FormWidget creation
     * @param type - the data type of the field (column)
     * @param columnId - the id of the column with that type
     * @return pointer to the newly created FormWidget
     */
    AbstractFormWidget* createFormWidget(MetadataEngine::FieldType type,
                                         int columnId);

    /**
     * Select current row as current index with selection, so that
     * any attached views are informed about selection/current change
     */
    void updateSelectionModel();

    /** Scroll to the specified FormWidget */
    void ensureFormWidgetVisible(AbstractFormWidget *fw);

    /** Create actions for the context menu */
    void createContextActions();

    /** Update size of the view and scrollbars */
    void updateSize();

    /** Load background theme from settings */
    void setupViewBackground();

    /** Load font config from settings */
    void setupViewFonts();

    FormLayoutMatrix *m_formLayoutMatrix;         /**< Layout holder for form widgets */
    QList<AbstractFormWidget*> m_formWidgetList;  /**< A list of all form widgets     */
    QList<QPropertyAnimation*> m_animationList;   /**< A list with queued animation   */
    int m_widthUnitPx;                    /**< Define how many pixels one width layout unit
                                               in the virtual FormView grid is */
    int m_heightUnitPx;                   /**< Define how many pixels one heigth layout unit
                                               in the virtual FormView grid is */
    int m_idealWidth;   /**< The ideal width of the view to show all items
                           without need of scrollbars */
    int m_idealHeight;  /**< The ideal width of the view to show all items
                           without need of scrollbars */
    bool m_sizeIsDirty; /**< Whether the size of the widget (view) is valid
                             or needs to be recalculated */
    bool m_isAnimating; /**< Whether there are some animations currently running */
    bool m_isMovingFW;  /**< Whether a FormWidget is being moved (custom drag) */
    AbstractFormWidget *m_movingFW; /**< The FormWidget currently in a move event (custom drag)     */
    QPoint m_moveFWHotSpot;         /**< Click offset on FormWidget during move event (custom drag) */
    QPoint m_dragStartPosition;     /**< The start position on a custom drag operation              */
    DropRectWidget *m_dropRectWidget; /**< A rect widget to mark the
                                           drop position during drag operation */
    bool m_isSelectedFW;  /**< Whether a FormWidget is currently selected */
    AbstractFormWidget *m_selectedFW; /**< The FormWidget currently selected */
    SelectRectWidget *m_selectRectWidget; /**< A rect widget to mark current FW selection */
    ResizeDotWidget *m_horizontalResizeGrip; /**< A dot widget next to selcted widget to allow resizing */
    ResizeDotWidget *m_verticalResizeGrip;   /**< A dot widget next to selcted widget to allow resizing */
    bool m_isResizingFW; /**< Whether a FormWidget is being manually resized */
    int m_currentRow; /**< The current row (item) that is active */
    int m_currentColumn; /**< The current column (field) that is active */
    MetadataEngine *m_metadataEngine; /**< Pointer to metadata engine instance */
    EmptyFormWidget *m_emptyFormWidget; /**< A placeholder widget for an empty form */
    bool m_modifiedTrigger; /**< Whether at least one field
                                 of type ModDateType is present.
                                 If this flag is true, all fields
                                 of ModDateType are updated after
                                 changes to records */
    QList<int> m_modFieldList; /**< List of fields with ModDateType as type */

    //context menu actions
    QAction *m_newFieldContextAction;
    QAction *m_duplicateFieldContextAction;
    QAction *m_deleteFieldContextAction;
    QAction *m_modifyFieldContextAction;
    QAction *m_newRecordContextAction;
    QAction *m_duplicateRecordContextAction;
    QAction *m_deleteRecordContextAction;
};

#endif // FORMVIEW_H
