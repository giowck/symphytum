/*
 *  Copyright (c) 2012 Giorgio Wicklein <giowckln@gmail.com>
 */

//-----------------------------------------------------------------------------
// Hearders
//-----------------------------------------------------------------------------

#include "formviewlayoutstate.h"
#include "../components/formlayoutmatrix.h"
#include "../widgets/form_widgets/abstractformwidget.h"


//-----------------------------------------------------------------------------
// Public
//-----------------------------------------------------------------------------

FormViewLayoutState::FormViewLayoutState(FormLayoutMatrix *matrix,
                                         const QList<AbstractFormWidget *>
                                         &formWidgetList)
{
    //save matrix
    for (int i = 0; i < matrix->rowCount(); i++) {
        for (int j = 0; j < matrix->columnCount(); j++) {
            AbstractFormWidget *fw = matrix->getFormWidget(i, j);

            //if fw is a valid form widget
            if ((fw != ((void*) FormLayoutMatrix::NO_FORM_WIDGET)) &&
                    (fw != ((void*) FormLayoutMatrix::EXTENDED_FORM_WIDGET))) {
                //init item
                FormWidgetItem item;
                item.id = formWidgetList.indexOf(fw);
                item.width = fw->getWidthUnits();
                item.height = fw->getHeightUnits();
                item.row = i;
                item.column = j;

                //append
                m_formWidgetItems.append(item);
            }
        }
    }
}

FormViewLayoutState::FormViewLayoutState(const FormViewLayoutState &other) :
    m_formWidgetItems(other.m_formWidgetItems)
{

}

FormLayoutMatrix* FormViewLayoutState::toFormLayoutMatrix(
        const QList<AbstractFormWidget *> &formWidgetList)
{
    if (formWidgetList.size() != m_formWidgetItems.size())
        return NULL;

    FormLayoutMatrix *matrix = new FormLayoutMatrix;

    foreach (FormWidgetItem item, m_formWidgetItems) {
        AbstractFormWidget *fw = formWidgetList.at(item.id);
        fw->setWidthUnits(item.width);
        fw->setHeightUnits(item.height);
        matrix->setFormWidget(fw, item.row, item.column);
    }

    return matrix;
}
