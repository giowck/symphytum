/**
  * \class TableViewDelegate
  * \brief This is the main delegate for the TableView.
  * \author Giorgio Wicklein - GIOWISYS Software
  * \date 09/06/2012
  */

#ifndef TABLEVIEWDELEGATE_H
#define TABLEVIEWDELEGATE_H


//-----------------------------------------------------------------------------
// Headers
//-----------------------------------------------------------------------------

#include <QtWidgets/QStyledItemDelegate>


//-----------------------------------------------------------------------------
// Forward declarations
//-----------------------------------------------------------------------------

class MetadataEngine;

struct TableViewDelegateFlags { //a bit of a hack
    static bool dataChangedOnLastEdit; /**< A flag to indicate that the data was really changed (edit finished)*/
};


//-----------------------------------------------------------------------------
// TableViewDelegate
//-----------------------------------------------------------------------------

class TableViewDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    explicit TableViewDelegate(QObject *parent = nullptr);
    
    void paint(QPainter *painter, const QStyleOptionViewItem &option,
               const QModelIndex &index) const;
    QSize sizeHint(const QStyleOptionViewItem &option,
                   const QModelIndex &index) const;
    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                          const QModelIndex &index) const;
    void setEditorData(QWidget *editor, const QModelIndex &index) const;
    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option,
                              const QModelIndex &index) const;
    void setModelData(QWidget *editor, QAbstractItemModel *model,
                      const QModelIndex &index) const;

private slots:
    void commitAndCloseCustomEditor();

private:
    //custom paint methods
    void paintTextType(QPainter *painter, const QStyleOptionViewItem &option,
                       const QModelIndex &index) const;
    void paintNumericType(QPainter *painter, const QStyleOptionViewItem &option,
                          const QModelIndex &index) const;
    void paintCreateDateType(QPainter *painter, const QStyleOptionViewItem &option,
                             const QModelIndex &index) const;
    void paintCheckboxType(QPainter *painter, const QStyleOptionViewItem &option,
                           const QModelIndex &index) const;
    void paintComboboxType(QPainter *painter, const QStyleOptionViewItem &option,
                           const QModelIndex &index) const;
    void paintProgressType(QPainter *painter, const QStyleOptionViewItem &option,
                           const QModelIndex &index) const;
    void paintImageType(QPainter *painter, const QStyleOptionViewItem &option,
                        const QModelIndex &index) const;
    void paintFilesType(QPainter *painter, const QStyleOptionViewItem &option,
                        const QModelIndex &index) const;

    //custom set editor data methods
    void setTextTypeEditorData(QWidget *editor, const QModelIndex &index) const;
    void setNumericTypeEditorData(QWidget *editor, const QModelIndex &index) const;
    void setCheckboxTypeEditorData(QWidget *editor, const QModelIndex &index) const;
    void setComboboxTypeEditorData(QWidget *editor, const QModelIndex &index) const;
    void setProgressTypeEditorData(QWidget *editor, const QModelIndex &index) const;
    void setImageTypeEditorData(QWidget *editor, const QModelIndex &index) const;
    void setFilesTypeEditorData(QWidget *editor, const QModelIndex &index) const;
    void setDateTypeEditorData(QWidget *editor, const QModelIndex &index) const;

    MetadataEngine *m_metadataEngine;
    bool m_cacheImages;
    bool m_hideImages;
};

#endif // TABLEVIEWDELEGATE_H
