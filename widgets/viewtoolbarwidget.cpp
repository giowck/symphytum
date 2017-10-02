/*
 *  Copyright (c) 2012 Giorgio Wicklein <giowckln@gmail.com>
 */

//-----------------------------------------------------------------------------
// Hearders
//-----------------------------------------------------------------------------

#include "viewtoolbarwidget.h"
#include "searchlineedit.h"

#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QFrame>
#include <QtWidgets/QToolButton>
#include <QtWidgets/QFrame>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QButtonGroup>

//-----------------------------------------------------------------------------
// Public
//-----------------------------------------------------------------------------

ViewToolBarWidget::ViewToolBarWidget(QWidget *parent) :
    QWidget(parent)
{
    //mainFrame
    m_mainFrame = new QFrame(this);
    m_mainFrame->setObjectName("mainFrame");

    //tool buttons
    QKeySequence newKey(QKeySequence::New);
    m_newRecordButton = new QToolButton(m_mainFrame);
    m_newRecordButton->setText(tr("New Record"));
    m_newRecordButton->setToolTip(tr("New Record (%1)")
                                  .arg(newKey.toString(QKeySequence::NativeText)));
    m_newRecordButton->setStatusTip(tr("Create a new empty record in the current collection"));
    m_newRecordButton->setIconSize(QSize(24, 24));
    m_newRecordButton->setIcon(QIcon(":/images/icons/newrecord.png"));
    m_newRecordButton->setObjectName("toolActionLeft");
    m_newRecordButton->setFocusPolicy(Qt::NoFocus);

    QKeySequence duplicateKey(QString("CTRL+D"));
    m_duplicateRecordButton = new QToolButton(m_mainFrame);
    m_duplicateRecordButton->setText(tr("Duplicate Record"));
    m_duplicateRecordButton->setToolTip(tr("Duplicate Record (%1)")
                                        .arg(duplicateKey.toString(QKeySequence::NativeText)));
    m_duplicateRecordButton->setStatusTip(tr("Duplicate current record and add it to the same collection"));
    m_duplicateRecordButton->setIconSize(QSize(24, 24));
    m_duplicateRecordButton->setIcon(QIcon(":/images/icons/duplicaterecord.png"));
    m_duplicateRecordButton->setObjectName("toolActionMid");
    m_duplicateRecordButton->setFocusPolicy(Qt::NoFocus);
    m_duplicateRecordButton->setShortcut(duplicateKey);

#ifdef Q_OS_OSX
    QKeySequence deleteKey(Qt::Key_Backspace);
#else
    QKeySequence deleteKey(QKeySequence::Delete);
#endif // Q_OS_OSX
    m_deleteRecordButton = new QToolButton(m_mainFrame);
    m_deleteRecordButton->setText(tr("Delete Record"));
    m_deleteRecordButton->setToolTip(tr("Delete Record (%1)")
                                     .arg(deleteKey.toString(QKeySequence::NativeText)));
    m_deleteRecordButton->setStatusTip(tr("Removes permanently current record from the collection"));
    m_deleteRecordButton->setIconSize(QSize(24, 24));
    m_deleteRecordButton->setIcon(QIcon(":/images/icons/deleterecord.png"));
    m_deleteRecordButton->setObjectName("toolActionRight");
    m_deleteRecordButton->setFocusPolicy(Qt::NoFocus);
    m_deleteRecordButton->setShortcut(deleteKey);

//    //separator1
//    QFrame *separator1 = new QFrame(m_mainFrame);
//    separator1->setObjectName(QString::fromUtf8("separator1"));
//    separator1->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
//    separator1->setFrameShape(QFrame::VLine);
//    separator1->setFrameShadow(QFrame::Raised);

    QKeySequence newFieldKey(Qt::CTRL + Qt::SHIFT + Qt::Key_N);
    m_newFieldButton = new QToolButton(m_mainFrame);
    m_newFieldButton->setText(tr("New Field"));
    m_newFieldButton->setToolTip(tr("New Field (%1)")
                                  .arg(newFieldKey.toString(QKeySequence::NativeText)));
    m_newFieldButton->setStatusTip(tr("Add a new field to the current collection"));
    m_newFieldButton->setIconSize(QSize(24, 24));
    m_newFieldButton->setIcon(QIcon(":/images/icons/newfield.png"));
    m_newFieldButton->setObjectName("toolActionLeft");
    m_newFieldButton->setFocusPolicy(Qt::NoFocus);
    m_newFieldButton->setShortcut(newFieldKey);

    QKeySequence dupFieldKey(Qt::CTRL + Qt::SHIFT + Qt::Key_D);
    m_duplicateFieldButton = new QToolButton(m_mainFrame);
    m_duplicateFieldButton->setText(tr("Duplicate Field"));
    m_duplicateFieldButton->setToolTip(tr("Duplicate Field (%1)")
                                       .arg(dupFieldKey.toString(QKeySequence::NativeText)));
    m_duplicateFieldButton->setStatusTip(tr("Duplicate the selected field to the same collection"));
    m_duplicateFieldButton->setIconSize(QSize(24, 24));
    m_duplicateFieldButton->setIcon(QIcon(":/images/icons/duplicatefield.png"));
    m_duplicateFieldButton->setObjectName("toolActionMid");
    m_duplicateFieldButton->setFocusPolicy(Qt::NoFocus);
    m_duplicateFieldButton->setShortcut(dupFieldKey);

#ifdef Q_OS_OSX
    QKeySequence delFieldKey(Qt::CTRL + Qt::SHIFT + Qt::Key_Backspace);
#else
    QKeySequence delFieldKey(Qt::CTRL + Qt::SHIFT + Qt::Key_Delete);
#endif // Q_OS_OSX
    m_deleteFieldButton = new QToolButton(m_mainFrame);
    m_deleteFieldButton->setText(tr("Delete Field"));
    m_deleteFieldButton->setToolTip(tr("Delete Field (%1)")
                                    .arg(delFieldKey.toString(QKeySequence::NativeText)));
    m_deleteFieldButton->setStatusTip(tr("Remove permanently the selected field from the collection"));
    m_deleteFieldButton->setIconSize(QSize(24, 24));
    m_deleteFieldButton->setIcon(QIcon(":/images/icons/deletefield.png"));
    m_deleteFieldButton->setObjectName("toolActionRight");
    m_deleteFieldButton->setFocusPolicy(Qt::NoFocus);
    m_deleteFieldButton->setShortcut(delFieldKey);

//    //separator2
//    QFrame *separator2 = new QFrame(m_mainFrame);
//    separator2->setObjectName(QString::fromUtf8("separator2"));
//    separator2->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
//    separator2->setFrameShape(QFrame::VLine);
//    separator2->setFrameShadow(QFrame::Raised);

    //formView
    m_formViewButton = new QPushButton(tr("Form"), m_mainFrame);
    m_formViewButton->setStatusTip(tr("Change current view mode to a form-like view"));
    m_formViewButton->setCheckable(true);
    m_formViewButton->setChecked(true);
    m_formViewButton->setFocusPolicy(Qt::NoFocus);
    m_formViewButton->setObjectName("viewModeActionLeft");

    //tableView
    m_tableViewButton = new QPushButton(tr("Table"), m_mainFrame);
    m_tableViewButton->setStatusTip(tr("Change current view mode to a table-like view"));
    m_tableViewButton->setCheckable(true);
    m_tableViewButton->setFocusPolicy(Qt::NoFocus);
    m_tableViewButton->setObjectName("viewModeActionRight");

    //button group
    m_viewModeButtonGroup = new QButtonGroup(m_mainFrame);
    m_viewModeButtonGroup->addButton(m_formViewButton);
    m_viewModeButtonGroup->addButton(m_tableViewButton);

//    //separator3
//    QFrame *separator3 = new QFrame(m_mainFrame);
//    separator3->setObjectName(QString::fromUtf8("separator3"));
//    separator3->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
//    separator3->setFrameShape(QFrame::VLine);
//    separator3->setFrameShadow(QFrame::Raised);

    //previous record
    QKeySequence previous(Qt::CTRL + Qt::Key_Left);
    m_previousRecordButton = new QToolButton(m_mainFrame);
    m_previousRecordButton->setText(tr("Previous Record"));
    m_previousRecordButton->setToolTip(tr("Previous Record (%1)")
                                       .arg(previous.toString(QKeySequence::NativeText)));
    m_previousRecordButton->setStatusTip(tr("Navigate to the previous record"));
    m_previousRecordButton->setIconSize(QSize(24, 24));
    m_previousRecordButton->setIcon(QIcon(":/images/icons/previousrecord.png"));
    m_previousRecordButton->setObjectName("toolActionPrevRec");
    m_previousRecordButton->setFocusPolicy(Qt::NoFocus);
    m_previousRecordButton->setShortcut(previous);

    //next record
    QKeySequence next(Qt::CTRL + Qt::Key_Right);
    m_nextRecordButton = new QToolButton(m_mainFrame);
    m_nextRecordButton->setText(tr("Next Record"));
    m_nextRecordButton->setToolTip(tr("Next Record (%1)")
                                   .arg(next.toString(QKeySequence::NativeText)));
    m_nextRecordButton->setStatusTip(tr("Navigate to the next record"));
    m_nextRecordButton->setIconSize(QSize(24, 24));
    m_nextRecordButton->setIcon(QIcon(":/images/icons/nextrecord.png"));
    m_nextRecordButton->setObjectName("toolActionNextRec");
    m_nextRecordButton->setFocusPolicy(Qt::NoFocus);
    m_nextRecordButton->setShortcut(next);

//    //separator4
//    QFrame *separator4 = new QFrame(m_mainFrame);
//    separator4->setObjectName(QString::fromUtf8("separator4"));
//    separator4->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
//    separator4->setFrameShape(QFrame::VLine);
//    separator4->setFrameShadow(QFrame::Raised);

    //search line
    m_searchLineEdit = new SearchLineEdit(m_mainFrame);
    m_searchLineEdit->setFocusPolicy(Qt::ClickFocus);

    //layout
    m_recordLayout = new QHBoxLayout;
    m_recordLayout->addWidget(m_newRecordButton);
    m_recordLayout->addWidget(m_duplicateRecordButton);
    m_recordLayout->addWidget(m_deleteRecordButton);
    m_recordLayout->setSpacing(0);

    m_fieldLayout = new QHBoxLayout;
    m_fieldLayout->addWidget(m_newFieldButton);
    m_fieldLayout->addWidget(m_duplicateFieldButton);
    m_fieldLayout->addWidget(m_deleteFieldButton);
    m_fieldLayout->setSpacing(0);

    m_viewModeLayout = new QHBoxLayout;
    m_viewModeLayout->addWidget(m_formViewButton);
    m_viewModeLayout->addWidget(m_tableViewButton);
    m_viewModeLayout->setSpacing(0);

    m_navLayout = new QHBoxLayout;
    m_navLayout->addWidget(m_previousRecordButton);
    m_navLayout->addWidget(m_nextRecordButton);
    m_navLayout->setSpacing(0);

    m_frameLayout = new QHBoxLayout(m_mainFrame);
    m_frameLayout->addLayout(m_recordLayout);
    //m_frameLayout->addWidget(separator1);
    m_frameLayout->addLayout(m_fieldLayout);
    //m_frameLayout->addWidget(separator2);
    m_frameLayout->addStretch();
    m_frameLayout->addLayout(m_viewModeLayout);
    //m_frameLayout->addWidget(separator3);
    m_frameLayout->addStretch();
    m_frameLayout->addLayout(m_navLayout);
    //m_frameLayout->addWidget(separator4);
    m_frameLayout->addWidget(m_searchLineEdit);

    m_mainLayout = new QHBoxLayout(this);
    m_mainLayout->addWidget(m_mainFrame);
    m_mainLayout->setContentsMargins(0,0,0,0);

    //style
    setStyleSheet(
                "QFrame#mainFrame {"
                "background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,"
                "stop: 0 #a6a6a6, stop: 0.08 #7f7f7f,"
                "stop: 0.39999 #717171, stop: 0.4 #626262,"
                "stop: 0.9 #4c4c4c, stop: 1 #333333);"
                "}"
                "QToolButton#toolActionLeft {"
                "color: #333;"
                "border-top: 2px solid #555;"
                "border-left: 2px solid #555;"
                "border-bottom: 2px solid #555;"
                "border-top-left-radius: 9px;"
                "border-bottom-left-radius: 9px;"
                "padding: 3px;"
                "background: qradialgradient(cx: 0.3, cy: -0.4,"
                "fx: 0.3, fy: -0.4,"
                "radius: 1.35, stop: 0 #fff, stop: 1 #888);"
                "}"
                "QToolButton#toolActionMid {"
                "color: #333;"
                "border-top: 2px solid #555;"
                "border-bottom: 2px solid #555;"
                "border-right: 1px solid #555;"
                "border-left: 1px solid #555;"
                "padding: 3px;"
                "background: qradialgradient(cx: 0.3, cy: -0.4,"
                "fx: 0.3, fy: -0.4,"
                "radius: 1.35, stop: 0 #fff, stop: 1 #888);"
                "}"
                "QToolButton#toolActionRight {"
                "color: #333;"
                "border-top: 2px solid #555;"
                "border-right: 2px solid #555;"
                "border-bottom: 2px solid #555;"
                "border-top-right-radius: 9px;"
                "border-bottom-right-radius: 9px;"
                "padding: 3px;"
                "background: qradialgradient(cx: 0.3, cy: -0.4,"
                "fx: 0.3, fy: -0.4,"
                "radius: 1.35, stop: 0 #fff, stop: 1 #888);"
                "}"
                "QToolButton#toolActionPrevRec{"
                "color: #333;"
                "border-top: 2px solid #555;"
                "border-right: 1px solid #555;"
                "border-left: 2px solid #555;"
                "border-bottom: 2px solid #555;"
                "border-top-left-radius: 9px;"
                "border-bottom-left-radius: 9px;"
                "padding: 3px;"
                "background: qradialgradient(cx: 0.3, cy: -0.4,"
                "fx: 0.3, fy: -0.4,"
                "radius: 1.35, stop: 0 #fff, stop: 1 #888);"
                "}"
                "QToolButton#toolActionNextRec {"
                "color: #333;"
                "border-top: 2px solid #555;"
                "border-right: 2px solid #555;"
                "border-bottom: 2px solid #555;"
                "border-top-right-radius: 9px;"
                "border-bottom-right-radius: 9px;"
                "padding: 3px;"
                "background: qradialgradient(cx: 0.3, cy: -0.4,"
                "fx: 0.3, fy: -0.4,"
                "radius: 1.35, stop: 0 #fff, stop: 1 #888);"
                "}"
                "QToolButton:hover#toolActionLeft,"
                "QToolButton:hover#toolActionMid,"
                "QToolButton:hover#toolActionRight,"
                "QToolButton:hover#toolActionPrevRec,"
                "QToolButton:hover#toolActionNextRec {"
                "background: qradialgradient(cx: 0.3, cy: -0.4,"
                "fx: 0.3, fy: -0.4,"
                "radius: 1.35, stop: 0 #fff, stop: 1 #bbb);"
                "}"
                "QToolButton:pressed#toolActionLeft,"
                "QToolButton:pressed#toolActionMid,"
                "QToolButton:pressed#toolActionRight,"
                "QToolButton:pressed#toolActionPrevRec,"
                "QToolButton:pressed#toolActionNextRec {"
                "background: qradialgradient(cx: 0.4, cy: -0.1,"
                "fx: 0.4, fy: -0.1,"
                "radius: 1.35, stop: 0 #fff, stop: 1 #ddd);"
                "}"
                "QPushButton#viewModeActionLeft {"
                "min-width: 90px;"
                "color: #333;"
                "border-left: 2px solid #555;"
                "border-top: 2px solid #555;"
                "border-bottom: 2px solid #555;"
                "border-top-left-radius: 9px;"
                "border-bottom-left-radius: 9px;"
                "padding: 3px;"
                "background: qradialgradient(cx: 0.3, cy: -0.4,"
                "fx: 0.3, fy: -0.4,"
                "radius: 1.35, stop: 0 #fff, stop: 1 #888);"
                "}"
                "QPushButton#viewModeActionRight {"
                "min-width: 80px;"
                "color: #333;"
                "border-right: 2px solid #555;"
                "border-left: 1px solid #555;"
                "border-top: 2px solid #555;"
                "border-bottom: 2px solid #555;"
                "border-top-right-radius: 9px;"
                "border-bottom-right-radius: 9px;"
                "padding: 3px;"
                "background: qradialgradient(cx: 0.3, cy: -0.4,"
                "fx: 0.3, fy: -0.4,"
                "radius: 1.35, stop: 0 #fff, stop: 1 #888);"
                "}"
                "QPushButton:hover#viewModeActionLeft,"
                "QPushButton:hover#viewModeActionRight {"
                "background: qradialgradient(cx: 0.3, cy: -0.4,"
                "fx: 0.3, fy: -0.4,"
                "radius: 1.35, stop: 0 #fff, stop: 1 #bbb);"
                "}"
                "QPushButton:pressed#viewModeActionLeft,"
                "QPushButton:pressed#viewModeActionRight {"
                "background: qradialgradient(cx: 0.4, cy: -0.1,"
                "fx: 0.4, fy: -0.1,"
                "radius: 1.35, stop: 0 #fff, stop: 1 #ddd);"
                "}"
                "QPushButton:checked#viewModeActionLeft,"
                "QPushButton:checked#viewModeActionRight {"
                "background: qradialgradient(cx: 0.4, cy: -0.1,"
                "fx: 0.4, fy: -0.1,"
                "radius: 1.35, stop: 0 #fff , stop: 1 #ddd);"
                "}"
                );

//    //adjust button's height as toolbutton's one
//    m_formViewButton->setMinimumHeight(m_newRecordButton->height());
//    m_tableViewButton->setMinimumHeight(m_newRecordButton->height());

    setLayout(m_mainLayout);
    setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Maximum);

    createConnections();
}

void ViewToolBarWidget::setViewModeState(ViewMode m)
{
    switch(m) {
    case FormViewMode:
        m_formViewButton->setChecked(true);
        break;
    case TableViewMode:
        m_tableViewButton->setChecked(true);
        break;
    }
}


//-----------------------------------------------------------------------------
// Public slots
//-----------------------------------------------------------------------------

void ViewToolBarWidget::setSearchLineFocus()
{
    m_searchLineEdit->setFocus();
}


//-----------------------------------------------------------------------------
// Private slots
//-----------------------------------------------------------------------------

void ViewToolBarWidget::SearchLineEditReturnPressed()
{
    emit searchSignal(m_searchLineEdit->text());
}


//-----------------------------------------------------------------------------
// Private
//-----------------------------------------------------------------------------

void ViewToolBarWidget::createConnections()
{
    connect(m_formViewButton, SIGNAL(clicked()),
            this, SIGNAL(formViewModeSignal()));
    connect(m_tableViewButton, SIGNAL(clicked()),
            this, SIGNAL(tableViewModeSignal()));

    connect(m_nextRecordButton, SIGNAL(clicked()),
            this, SIGNAL(nextRecordSignal()));
    connect(m_previousRecordButton, SIGNAL(clicked()),
            this, SIGNAL(previousRecordSignal()));

    connect(m_newRecordButton, SIGNAL(clicked()),
            this, SIGNAL(newRecordSignal()));
    connect(m_duplicateRecordButton, SIGNAL(clicked()),
            this, SIGNAL(duplicateRecordSignal()));
    connect(m_deleteRecordButton, SIGNAL(clicked()),
            this, SIGNAL(deleteRecordSignal()));

    connect(m_newFieldButton, SIGNAL(clicked()),
            this, SIGNAL(newFieldSignal()));
    connect(m_duplicateFieldButton, SIGNAL(clicked()),
            this, SIGNAL(duplicateFieldSignal()));
    connect(m_deleteFieldButton, SIGNAL(clicked()),
            this, SIGNAL(deleteFieldSignal()));

    connect(m_searchLineEdit, SIGNAL(textChanged(QString)),
            this, SIGNAL(searchSignal(QString)));
    connect(m_searchLineEdit, SIGNAL(returnPressed()),
            this, SLOT(SearchLineEditReturnPressed()));
}
