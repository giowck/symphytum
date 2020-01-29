/**
  * \class FormLayoutMatrix
  * \brief This class is used by FormView to manage the layout
  *        and position of the form widgets within the view.
  *        FormView organizes its widgets in a matrix style (rows/columns) structure, in order
  *        to keep internally the layout states of the widgets.
  *        The matrix is represented by rows and columns
  *        beginning with value 0 (ie. first element is at (0,0).
  *        A form widget (FW) is saved as a pointer in a cell of the matrix.
  *        Each FW has 2 attributes that describe how many cells are needed by the
  *        widget in the matrix and layout (in the FormView), these are widthUnits (columns) and
  *        heightUnits (rows). I f a FW is bigger than one cell i.e its width or/and height is bigger
  *        than 1, the cells that belong to the same FW are marked as EXTENDED_FORM_WIDGET and only
  *        the first cell (first column/row of the widget) coontains the actual widget (FW* pointer).
  * \author Giorgio Wicklein - GIOWISYS Software
  * \date 10/04/2012
  */

#ifndef FORMLAYOUTMATRIX_H
#define FORMLAYOUTMATRIX_H


//-----------------------------------------------------------------------------
// Headers
//-----------------------------------------------------------------------------

#include <QtCore/QMap>
#include <QtCore/QList>


//-----------------------------------------------------------------------------
// Forward declarations
//-----------------------------------------------------------------------------

class QString;
class AbstractFormWidget;
typedef AbstractFormWidget FormWidget;


//-----------------------------------------------------------------------------
// FormLayoutMatrix
//-----------------------------------------------------------------------------

class FormLayoutMatrix
{
public:
    FormLayoutMatrix();
    FormLayoutMatrix(const FormLayoutMatrix &other);

    /** Return the number of allocated rows    */
    int rowCount() const;

    /** Return the number of allocated columns */
    int columnCount() const;

    /** Return the form widget at given row and column */
    AbstractFormWidget* getFormWidget(int row, int column) const;

    /** Get the associated FW of a cell which contains the
      * extended widget constant to mark the cell at the given row
      * and column as part of a form widget with size bigger 1 units
      * @param row - the row index where we expect the extension cell
      * @param column - the column index where we expect extenion const
      * @return FormWidget pointer to which the extension cell belongs,
      *         if index doesn't contain the extension const, NULL is returned
      */
    AbstractFormWidget* getFormWidgetByExtended(int row, int column) const;

    /** Set the form widget to be on the given index position.
      * An existing FW pointer at the index pos is overwritten.
      * @param fw the widget to set
      * @param row the targetted row (beginning with 0)
      * @param column the targetted column (beginning with 0)
      */
    void setFormWidget(AbstractFormWidget* fw, int row, int column);

    /** Add a form widget to the matrix at a convienent index position
      * @param fw the widget to add to the matrix
      */
    void addFormWidget(AbstractFormWidget* fw);

    /** Removes the FormWidget at the given index
      * @return the form widget which was at the index pos
      */
    AbstractFormWidget* removeFormWidget(int row, int column);

    /** This mehtod is used by FormView to handle drag/movement operations
      * on the layout. If the user drags/moves a FormWidget on the FormView,
      * the drop position represents the new index, where the dragged widget
      * should be placed. This method takes care of existing widgets at the
      * given index position. If the newRow and/or newColumn index is bigger
      * than the matrix size, new space will be allocated to fit the new index.
      * @param fw - the form widget that should be moved to the new index
      * @param newRow - the new row position where the FW should be placed
      * @param newColumn - the new column position where the FW should be placed
      */
    void formWidgetMovement(AbstractFormWidget* fw, int newRow, int newColumn);

    /** This mehtod is used by FormView to handle FW resize operations
      * on the layout. This method takes care of existing widgets if the
      * new size overlaps them. If the new FW size is bigger
      * than the matrix size, new space will be allocated to fit the new size.
      * @param fw - the form widget that should be resized
      * @param newWidthUnits - the new FW width
      * @param newHeightUnits - the new FW height
      */
    void formWidgetResize(AbstractFormWidget* fw, int newWidthUnits, int newHeightUnits);

    /** Find the matrix index (row/column) of the given form widget
      * @param fw the form widget from which we want the index position
      * @param row an integer passed as reference to save the row position
      * @param column an integer passed as reference to save the column position
      * @return bool indicating if the search was successful
      */
    bool findFormWidgetIndex(AbstractFormWidget* fw, int &row, int &column);

    /** Look if there are some improvements to do like
      * removing unused (empty) matrix rows or columns
      * @param aggressive - if true all empty rows/columns will be pruned away,
      *        if false, only empty rows/columns at the edges will be removed
      */
    void simplifyMatrix(const bool aggressive);

    /** Create a QString representation of the layout matrix */
    QString toString();

    /** Overload operator [] to access the given row */
    QList<FormWidget*>& operator[] (const int row); //reference retuning could cause crash if out of scope

    /** Overload operator == for deep comparison */
    bool operator== (const FormLayoutMatrix& other) const;

    /** Overload operator != for deep comparison */
    bool operator!= (const FormLayoutMatrix& other) const;

    /** The matrix contains either:
      * - a valid FormWidget* pointer
      * - 0x01 pointer to mark empty spaces
      * - 0x02 constant to mark that the cell
      *        is part of another widget which
      *        is bigger that 1 unit...
      */
    enum MatrixEntryState{
        NO_FORM_WIDGET = 0x01,      /**< No form widget, this is an empty cell           */
        EXTENDED_FORM_WIDGET = 0x02 /**< The cell is not empty because it is part
                                         of another widget with sizes bigger than 1 unit */
    };

private:
    /** helper function to remove an entire column */
    void removeColumn(int column);

    /** helper function to remove an entire row */
    void removeRow(int row);

    /** Increment the size of the matrix.
      * @param rows how many rows are needed
      * @param columns how many columns are needed
      */
    void growMatrixSize(int rows, int columns);

    /** Find a free space in the matrix that fits the needs
      * in width and height.
      * @param widthUnits the horizontal units needed
      * @param heightUnits the vertical units needed
      * @param row reference where to save the row number where the free space starts
      * @param column reference where to save the col number where the free space starts
      * @return bool whether free space was found or not
      */
    bool findFreeSpace(int widthUnits, int heightUnits, int &row, int &column);

    /** Create free space in the matrix with the given needs
      * in width and height.
      * @param widthUnits the horizontal units needed
      * @param heightUnits the vertical units needed
      * @param row reference where to save the row number where the free space starts
      * @param column reference where to save the col number where the free space starts
      */
    void createFreeSpace(int heightUnits, int widthUnits, int &row, int &column);

    int m_rows;                              /**< Current row count    */
    int m_columns;                           /**< Current column count */
    QMap<int, QList<FormWidget*> > m_matrix; /**< Map of form widgets in rows and columns.
                                              *   the key (int) represents the row and the
                                              *   list of FW represents current row's columns.
                                              */
    bool m_pruneEmptyRowsCols; /**< If true, empty rows/columns are pruned
                                *   from the matrix, @see simplifyMastrix()
                                *   for additional info and context
                                */
};

#endif // FORMLAYOUTMATRIX_H
