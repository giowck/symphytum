/*
 *  Copyright (c) 2012 Giorgio Wicklein <giowckln@gmail.com>
 */

//-----------------------------------------------------------------------------
// Hearders
//-----------------------------------------------------------------------------

#include "formlayoutmatrix.h"
#include "settingsmanager.h"
#include "../widgets/form_widgets/abstractformwidget.h"

#include <QtCore/QString>
#include <QtCore/QVariant>


//-----------------------------------------------------------------------------
// Public
//-----------------------------------------------------------------------------

FormLayoutMatrix::FormLayoutMatrix() : m_rows(0), m_columns(0)
{
    //load settings
    SettingsManager sm;
    m_pruneEmptyRowsCols = sm.restoreProperty("pruneEmptyRowsCols", "formView").toBool(); //default false
}

FormLayoutMatrix::FormLayoutMatrix(const FormLayoutMatrix &other) :
    m_rows(other.m_rows), m_columns(other.m_columns),
    m_matrix(other.m_matrix)
{

}

int FormLayoutMatrix::rowCount() const
{
    return m_rows;
}

int FormLayoutMatrix::columnCount() const
{
    return m_columns;
}

AbstractFormWidget* FormLayoutMatrix::getFormWidget(int row, int column) const
{
    //check boundaries
    if ((row < m_rows) && (column < m_columns)) {
        return m_matrix[row].at(column);
    } else {
        return NULL;
    }
}

AbstractFormWidget* FormLayoutMatrix::getFormWidgetByExtended(int row, int column) const
{
    bool found = false;
    FormWidget *fw = NULL;

    //check boundaries
    if ((row < m_rows) && (column < m_columns)) {

        //check if the cell is really an extension
        if (m_matrix[row][column] != (FormWidget*)EXTENDED_FORM_WIDGET)
            return NULL;

        //iterate through every row and column backwards
        for (int i = row; i >= 0; i--) {
            for (int j = column; j >= 0; j--) {
                fw = m_matrix[i][j];
                if ((fw != (FormWidget*)NO_FORM_WIDGET) &&
                    (fw != (FormWidget*)EXTENDED_FORM_WIDGET)) {
                    int tRow, tColumn;
                    tRow = i + fw->getHeightUnits() - 1;
                    tColumn = j + fw->getWidthUnits() - 1;

                    if ((m_matrix[tRow][tColumn] == (FormWidget*)EXTENDED_FORM_WIDGET) &&
                        (row <= tRow) && (column <= tColumn)) {
                        found = true;
                        break;
                    }
                }
            }
            if (found) break;
        }
    }

    if (!found) fw = NULL;
    return fw;
}

void FormLayoutMatrix::setFormWidget(AbstractFormWidget *fw, int row, int column)
{   
    //check boundaries
    if ((row < m_rows) && (column < m_columns)) {
        int maxRow = row + fw->getHeightUnits() - 1;
        int maxColumn = column + fw->getWidthUnits() - 1;

        //check if the widget fits in current matrix size
        //if not grow the matrix to fit the mac row/column
        if ((maxRow >= m_rows) || (maxColumn >= m_columns)) {
            int rowDelta = maxRow - m_rows + 1; //+1 because row count starts by 1
            int columnDelta = maxColumn - m_columns + 1; //+1 because cols count starts by 1
            growMatrixSize(m_rows + rowDelta, m_columns + columnDelta);
        }

        //first mark all matrix cells which are part of the form widget
        for (int i = row; i < (row + fw->getHeightUnits()); i++) {
            for (int j = column; j < (column + fw->getWidthUnits()); j++) {
                m_matrix[i].replace(j,(FormWidget*)EXTENDED_FORM_WIDGET);
            }
        }
        //set the first row/column that belong to the FW with its address
        m_matrix[row].replace(column, fw);

    } else {
        //increment matrix size
        //grow function expects row/column count
        growMatrixSize(row + fw->getHeightUnits(), column + fw->getWidthUnits());
        setFormWidget(fw, row, column);
    }
}

void FormLayoutMatrix::addFormWidget(AbstractFormWidget *fw)
{   
    int row, column;
    bool spaceAvailable = findFreeSpace(fw->getWidthUnits(), fw->getHeightUnits(), row, column);

    if (spaceAvailable) {
        setFormWidget(fw, row, column);
    } else {
        createFreeSpace(fw->getWidthUnits(), fw->getHeightUnits(), row, column);
        setFormWidget(fw, row, column);
    }
}

AbstractFormWidget* FormLayoutMatrix::removeFormWidget(int row, int column)
{
    AbstractFormWidget* fw;

    //check boundaries
    if ((row < m_rows) && (column < m_columns)) {
        fw = m_matrix[row].at(column);
        //set all cells that are part of the widget to NOFW
        for (int i = row; i < (row + fw->getHeightUnits()); i++) {
            for (int j = column; j < (column + fw->getWidthUnits()); j++) {
                m_matrix[i].replace(j,(FormWidget*)NO_FORM_WIDGET);
            }
        }
    } else {
        return NULL;
    }

    return fw;
}

void FormLayoutMatrix::formWidgetMovement(AbstractFormWidget *fw, int newRow, int newColumn)
{
    //list of widget that should be removed from the current index
    QList<FormWidget*> widgetsToRemove;

    //list of widget that should be added again to the matrix
    QList<FormWidget*> widgetsToAdd;

    //remove dragged widget from its original index
    widgetsToRemove.append(fw);

    //remove widget (old widget if any) on target index and (eventually if any) widgets
    //on index + units needed to make sure the dragged widget has enough room
    int maxRow = newRow + fw->getHeightUnits() - 1;
    int maxColumn = newColumn + fw->getWidthUnits() - 1;
    for (int r = newRow; ((r <= maxRow) && (r < m_rows)); r++) {
        for (int c = newColumn; ((c <= maxColumn) && (c < m_columns)); c++) {
            FormWidget *f = m_matrix[r][c];
            if (f != (FormWidget*)FormLayoutMatrix::NO_FORM_WIDGET) {

                //if cell is an extension and part of another big FW, get its parent
                if (f == (FormWidget*)FormLayoutMatrix::EXTENDED_FORM_WIDGET)
                    f = getFormWidgetByExtended(r, c);

                //if the widget is not the dragged one and is not already processed
                //(this is possible cause of getFWbyExtended call), remove it
                //and add it back again at a convenient position
                if ((f != fw) && (!widgetsToRemove.contains(f))) {
                    widgetsToRemove.append(f); //rm to make room
                    widgetsToAdd.append(f);    //add it later again
                }
            }
        }
    }

    //remove marked widgets
    for (int i = 0; i < widgetsToRemove.size(); i++) {
        int rowIndex, columnIndex;
        FormWidget *f = widgetsToRemove.at(i);
        findFormWidgetIndex(f, rowIndex, columnIndex);
        removeFormWidget(rowIndex, columnIndex);
    }

    //set the currently dragged FW to the drop position
    setFormWidget(fw, newRow, newColumn);

    //add previously removed widgets again
    for (int i = 0; i < widgetsToAdd.size(); i++) {
        addFormWidget(widgetsToAdd.at(i));
    }

    //remove empty rows/cols if they exist
    simplifyMatrix(m_pruneEmptyRowsCols);
}

void FormLayoutMatrix::formWidgetResize(AbstractFormWidget *fw, int newWidthUnits, int newHeightUnits)
{
    //get current index
    int row, column;
    findFormWidgetIndex(fw, row, column);

    //list of widget that should be removed, cause they are on the way
    QList<FormWidget*> widgetsToRemove;

    //list of widget that should be added again to the matrix
    QList<FormWidget*> widgetsToAdd;

    //mark all widgets for removal that are in the way with the new FW and its new size
    //also remove current FW (selected one) to add it again later with new size
    int maxRow = row + newHeightUnits - 1;
    int maxColumn = column + newWidthUnits - 1;
    for (int r = row; ((r <= maxRow) && (r < m_rows)); r++) {
        for (int c = column; ((c <= maxColumn) && (c < m_columns)); c++) {
            FormWidget *f = m_matrix[r][c];
            if (f != (FormWidget*)FormLayoutMatrix::NO_FORM_WIDGET) {

                //if cell is an extension and part of another big FW, get its parent
                if (f == (FormWidget*)FormLayoutMatrix::EXTENDED_FORM_WIDGET)
                    f = getFormWidgetByExtended(r, c);

                //make sure the widget was not already processed (cause of extended call)
                if (!widgetsToRemove.contains(f)) {
                    widgetsToRemove.append(f); //rm to make room
                    //the selected fw is not added now, because it is added manually with setFormWidget method
                    if (f != fw) {
                        widgetsToAdd.append(f); //add it later again
                    }
                }
            }
        }
    }

    //remove marked widgets
    for (int i = 0; i < widgetsToRemove.size(); i++) {
        int rowIndex, columnIndex;
        FormWidget *f = widgetsToRemove.at(i);
        findFormWidgetIndex(f, rowIndex, columnIndex);
        removeFormWidget(rowIndex, columnIndex);
    }

    //set the selected widget at the same position but with its new size
    fw->setWidthUnits(newWidthUnits);
    fw->setHeightUnits(newHeightUnits);
    setFormWidget(fw, row, column);

    //add previously removed widgets again
    for (int i = 0; i < widgetsToAdd.size(); i++) {
        addFormWidget(widgetsToAdd.at(i));
    }

    //remove empty rows/cols if they exist
    simplifyMatrix(m_pruneEmptyRowsCols);
}

bool FormLayoutMatrix::findFormWidgetIndex(AbstractFormWidget *fw, int &row, int &column)
{
    bool found = false;
    row = 0;
    column = 0;

    QMap<int, QList<FormWidget*> >::const_iterator map_it;

    for (map_it = m_matrix.begin(); map_it != m_matrix.end(); map_it++) {
        column = map_it->indexOf(fw);
        if ((found = (column != -1))) break;
        row++;
    }

    return found;
}

void FormLayoutMatrix::simplifyMatrix(const bool aggressive)
{
    //prune away ALL empty rows/columns if aggro mode
    if (aggressive) {
        bool rowRemoved = false;

        //check for empty rows
        for (int i = 0; i < m_rows; i++) {
            bool markedRow = false; //current row removal flag
            QList<AbstractFormWidget*>& currentColumn = m_matrix[i];

            if (currentColumn.isEmpty()) {
                markedRow = true;
            } else {
                markedRow = true; //init value
                //check if all elements of the row are empty
                for (int c = 0; c < m_columns; c++) {
                    AbstractFormWidget* p = currentColumn.at(c);
                    //markedRow is only true if all previous and current cell is empty
                    markedRow = markedRow && ((p == NULL) || (p == (void*)NO_FORM_WIDGET));
                }
            }

            if (markedRow) {
                removeRow(i);
                rowRemoved = true;
                simplifyMatrix(m_pruneEmptyRowsCols); //repeat for all rows but now with one row less
                break; //break cycle because after removal the indexes are not coherent anymore
            }
        }

        //if no row was deleted the indexes are still valid, so we can check columns
        if (!rowRemoved) {
            //check for empty columns
            for (int i = 0; i < m_columns; i++) {
                bool markedColumn = true; //current column removal flag
                int j = 0;
                while (j < m_rows) {
                    AbstractFormWidget* p = m_matrix[j].at(i);
                    if ((p == (void*)NO_FORM_WIDGET) || (p == NULL)) {
                        j++;
                    } else {
                        markedColumn = false;
                        break; //there is an item that is valid so exit cycle
                    }
                }

                if (markedColumn) {
                    removeColumn(i);
                    simplifyMatrix(m_pruneEmptyRowsCols); //repeat for all columns but now with one column less
                    break; //break cycle because after removal the indexes are not coherent anymore
                }
            }
        }
    } else { //allow empty rows/columns inside the matrix just remove those at the edges of the layout
        bool scanningEdge = true;

        //instead of pruning out all empty rows/columns
        //only prune away extra rows/columns
        //ie., those on the edges of the layout

        //check for empty rows, from outside in
        for (int i = m_rows-1; i >= 0; i--) {
            if (scanningEdge == true) { //only test for removal if we're on the edge
                bool markedRow = false; //current row removal flag
                QList<AbstractFormWidget*>& currentColumn = m_matrix[i];

                if (currentColumn.isEmpty()) {
                    markedRow = true;
                } else {
                    markedRow = true; //init value
                    //check if all elements of the row are empty
                    for (int c = 0; c < m_columns; c++) {
                        AbstractFormWidget* p = currentColumn.at(c);
                        //markedRow is only true if all previous and current cell is empty
                        markedRow = markedRow && ((p == NULL) || (p == (void*)NO_FORM_WIDGET));
                    }
                }

                //if we find an empty row, we remove it and then re-try with the next-to-last row
                if (markedRow) {
                    removeRow(i);
                    //removing an edge row will not affect the indexing of subsequent loops
                } else {
                    //this row is not empty, then we have found all the empty edges, no more removals
                    scanningEdge = false;
                }
            }
        }

        //if no row was deleted the indexes are still valid, so we can check columns
        //check for empty columns
        scanningEdge = true;
        for (int i = m_columns-1; i >= 0; i--) {
            if (scanningEdge == true) {
                bool markedColumn = true; //current column removal flag
                int j = 0;
                while (j < m_rows) {
                    AbstractFormWidget* p = m_matrix[j].at(i);
                    if ((p == (void*)NO_FORM_WIDGET) || (p == NULL)) {
                        j++;
                    } else {
                        markedColumn = false;
                        break; //there is an item that is valid so exit cycle
                    }
                }

                if (markedColumn) {
                    removeColumn(i);
                    //removing an edge column will not affect the indexing of subsequent loops
                } else {
                    //this row is not empty, then we have found all the empty edges, no more removals
                    scanningEdge = false;
                }
            }
        }
    }
}

QString FormLayoutMatrix::toString()
{
    QString s;
    s.append("\n");

    for (int i = 0; i < m_rows; i++) {
        for (int j = 0; j < m_columns; j++) {
            void* p = m_matrix[i].at(j);
            if (p == NULL)
                s.append("NULL");
            else if (p == (void*)NO_FORM_WIDGET)
                s.append("NOFW");
            else if (p == (void*)EXTENDED_FORM_WIDGET)
                s.append("EXFW");
            else
                s.append("OKFW");
            s.append(" | ");
        }
        s.append("\n");
    }

    return s;
}

QList<FormWidget*>& FormLayoutMatrix::operator[] (const int row)
{
    Q_ASSERT(row < m_rows);

    return m_matrix[row];
}

bool FormLayoutMatrix::operator== (const FormLayoutMatrix& other) const
{
    bool b;

    if ((this->columnCount() != other.columnCount()) ||
            this->rowCount() != other.rowCount()) {
        b = false;
    } else {
        b = (m_matrix == other.m_matrix);
    }

    return b;
}

bool FormLayoutMatrix::operator!= (const FormLayoutMatrix& other) const
{
    return !(*this == other);
}


//-----------------------------------------------------------------------------
// Private
//-----------------------------------------------------------------------------

void FormLayoutMatrix::removeColumn(int column)
{
    for (int i = 0; i < this->m_rows; i++)
        m_matrix[i].removeAt(column);

    m_columns--;
}

void FormLayoutMatrix::removeRow(int row)
{
    //clear list (rm all columns of the row)
    m_matrix[row].clear();

    //shift every row after the row to remove one position down
    for (int i = row+1; i < this->m_rows; i++) {
        m_matrix[i-1] = m_matrix[i];
    }

    m_rows--;

    //clear all unused rows above row size
    for (int i = m_matrix.size() - 1; i >= this->m_rows; i--) {
        m_matrix[i].clear();
    }
}

void FormLayoutMatrix::growMatrixSize(int rows, int columns)
{
    if (rows > m_rows) {
        //increment rows
        int previousCount = m_rows;
        m_rows += (rows-m_rows);

        //add as many times as columns a no form widget pointer to new added rows
        for (int i = previousCount; i < m_rows; i++) {
            for (int j = 0; j < m_columns; j++) {
                m_matrix[i].append((FormWidget*)NO_FORM_WIDGET);
            }
        }
    }

    if (columns > m_columns) {
        //increment columns
        int delta = columns - m_columns;
        m_columns += delta;

        //add delta times a no form widget pointer to each row
        for (int i = 0; i < m_rows; i++) {
            for (int j = 0; j < delta; j++) {
                m_matrix[i].append((FormWidget*)NO_FORM_WIDGET);
            }
        }
    }
}

bool FormLayoutMatrix::findFreeSpace(int widthUnits, int heightUnits, int &row, int &column)
{
    bool found = false;

    //iterate through every row and column
    for (int i = 0; i < m_rows; i++) {
        QList<FormWidget*>& currentRow = m_matrix[i];
        for (int j = 0; j < m_columns; j++) {
            //if current item is free, check for space requirement
            if (currentRow.at(j) == (void*)NO_FORM_WIDGET) {
                bool isFree = true;
                //since a free item is available, space requirements are checked next
                for (int r = 0; r < heightUnits; r++) {
                    for (int c = 0; c < widthUnits; c++) {
                        int rowOffset = i+r;
                        int columnOffset = j+c;
                        if ((rowOffset >= m_rows) || (columnOffset >= m_columns)) {
                            isFree = false;
                        } else {
                            if (m_matrix[rowOffset].at(columnOffset) != (void*)NO_FORM_WIDGET) {
                                //one of the needed fields is not empty
                                isFree = false;
                            }
                        }
                    }
                }
                //if there are enough free fields we break the iteration and set found flag
                if ((found = isFree)) {
                    row = i;
                    column = j;
                    break;
                }
            }
            if (found) break;
        }
        if (found) break;
    }
    return found;
}

void FormLayoutMatrix::createFreeSpace(int widthUnits, int heightUnits, int &row, int &column)
{
    int moreRows = m_rows - heightUnits;
    int moreColumns = m_columns - widthUnits;
    int rowDelta = 0;
    int columnDelta = 0;

    //check if more rows or columns are needed
    if (moreRows < 0) rowDelta = -(moreRows);
    if (moreColumns < 0) columnDelta = -(moreColumns);

    //increment matrix size if needed to make sure the row/column count is enough
    growMatrixSize(m_rows + rowDelta, m_columns + columnDelta);

    //check if the recent growMatrix call made eventually enough free space
    //if not then add new rows, so we are sure the widget fits
    if (rowDelta != heightUnits) {
        //add new rows where the widget will be placed
        growMatrixSize(m_rows + heightUnits, 0);
    }

    //the form widget is always added to the newly created row
    row = m_rows - heightUnits;
    column = 0;
}
