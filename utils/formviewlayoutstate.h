/**
  * \class FormViewLayoutState
  * \brief This utility is used to save a FormLayoutMatrix object in a safe state
  *        since FormLayoutMatrix contains pointers to FormWidgets which may not
  *        be valid anymore after a collection id change. So here are saved all
  *        layout data in a independent persistent state. A FormViewLayoutState
  *        is generated directly from a FormLayoutMatrix object and can be
  *        converted back to it. This is used by the undo framework to allow
  *        layout changes to be reverted back (undone). Note that to make this
  *        work, the state of the collection (number of fields) must be the
  *        same between the constructor call and toFormLayoutMatrix() call,
  *        Because the id of the field is derived from the passed list
  *        of AbstractFormWidgets.
  * \author Giorgio Wicklein - GIOWISYS Software
  * \date 04/08/2012
  */

#ifndef FORMVIEWLAYOUTSTATE_H
#define FORMVIEWLAYOUTSTATE_H


//-----------------------------------------------------------------------------
// Headers
//-----------------------------------------------------------------------------

#include <QtCore/QList>


//-----------------------------------------------------------------------------
// Forward declarations
//-----------------------------------------------------------------------------

class FormLayoutMatrix;
class AbstractFormWidget;


//-----------------------------------------------------------------------------
// FormViewLayoutState
//-----------------------------------------------------------------------------

class FormViewLayoutState
{
public:
    /** Save layout state from matrix and list of form widgets */
    FormViewLayoutState(FormLayoutMatrix *matrix,
                        const QList<AbstractFormWidget*> &formWidgetList);

    FormViewLayoutState(const FormViewLayoutState &other);

    /** Create FormLayoutMatrix by passing a valid list of FormWidgets */
    FormLayoutMatrix* toFormLayoutMatrix(const QList<AbstractFormWidget*>
                                         &formWidgetList);

private:
    struct FormWidgetItem {
        int id;
        int width;
        int height;
        int row;
        int column;
    };

    QList<FormWidgetItem> m_formWidgetItems;
};

#endif // FORMVIEWLAYOUTSTATE_H
