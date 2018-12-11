/**
  * \class ViewToolBarWidget
  * \brief This widget represents a tool-bar-like widget holding
  *        actions such as search, new/duplicate/delete record and more.
           This widget is used in MainWindow.
  * \author Giorgio Wicklein - GIOWISYS Software
  * \date 18/05/2012
  */

#ifndef VIEWTOOLBARWIDGET_H
#define VIEWTOOLBARWIDGET_H


//-----------------------------------------------------------------------------
// Headers
//-----------------------------------------------------------------------------

#include <QWidget>


//-----------------------------------------------------------------------------
// Forward declarations
//-----------------------------------------------------------------------------

class QHBoxLayout;
class QToolButton;
class QFrame;
class QPushButton;
class QButtonGroup;
class SearchLineEdit;


//-----------------------------------------------------------------------------
// ViewToolBarWidget
//-----------------------------------------------------------------------------

class ViewToolBarWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ViewToolBarWidget(QWidget *parent = nullptr);

    /** This enum indicates the possible view modes */
    enum ViewMode {
        FormViewMode, /**< Form-like view mode */
        TableViewMode /**< Table-like view mode */
    };

    /** This method is used to set the view mode buttons to the specified state */
    void setViewModeState(ViewMode m);

public slots:
    /** Set the focus on the search line */
    void setSearchLineFocus();

signals:
    /** Emitted on form-view-mode button click */
    void formViewModeSignal();

    /** Emitted on table-view-mode button click */
    void tableViewModeSignal();

    /** Emitted on next record button click */
    void nextRecordSignal();

    /** Emitted on previous record button click */
    void previousRecordSignal();

    /** Emitted on new record button click */
    void newRecordSignal();

    /** Emitted on duplicate record button click */
    void duplicateRecordSignal();

    /** Emitted on delete record button click */
    void deleteRecordSignal();

    /** Emitted on new field button click */
    void newFieldSignal();

    /** Emitted on duplicate field button click */
    void duplicateFieldSignal();

    /** Emitted on delete field button click */
    void deleteFieldSignal();

    /** Emitted on search event */
    void searchSignal(const QString &s);

private slots:
    /** Called on return key press on SearchLineEdit */
    void SearchLineEditReturnPressed();

private:
    void createConnections();

    QHBoxLayout *m_mainLayout;
    QHBoxLayout *m_frameLayout;
    QHBoxLayout *m_recordLayout;
    QHBoxLayout *m_fieldLayout;
    QHBoxLayout *m_viewModeLayout;
    QHBoxLayout *m_navLayout;
    QFrame *m_mainFrame;
    QToolButton *m_newRecordButton;
    QToolButton *m_duplicateRecordButton;
    QToolButton *m_deleteRecordButton;
    QToolButton *m_newFieldButton;
    QToolButton *m_duplicateFieldButton;
    QToolButton *m_deleteFieldButton;
    QToolButton *m_previousRecordButton;
    QToolButton *m_nextRecordButton;
    QPushButton *m_formViewButton;
    QPushButton *m_tableViewButton;
    QButtonGroup *m_viewModeButtonGroup;
    SearchLineEdit *m_searchLineEdit;
};

#endif // VIEWTOOLBARWIDGET_H
