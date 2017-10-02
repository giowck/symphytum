/*
 *  Copyright (c) 2012 Giorgio Wicklein <giowckln@gmail.com>
 */

//-----------------------------------------------------------------------------
// Hearders
//-----------------------------------------------------------------------------

#include "comboboxfieldwizard.h"
#include "ui_comboboxfieldwizard.h"

#include "../../components/metadataengine.h"
#include "../../components/databasemanager.h"
#include "../../utils/metadatapropertiesparser.h"

#include <QtSql/QSqlQuery>


//-----------------------------------------------------------------------------
// Public
//-----------------------------------------------------------------------------

ComboboxFieldWizard::ComboboxFieldWizard(const QString &fieldName,
                                         QWidget *parent,
                                         AbstractFieldWizard::EditMode editMode) :
    AbstractFieldWizard(fieldName, parent, editMode),
    ui(new Ui::ComboboxFieldWizard),
    m_default(-1)
{
    ui->setupUi(this);

    connect(ui->backButton, SIGNAL(clicked()),
            this, SIGNAL(backSignal()));
    connect(ui->finishButton, SIGNAL(clicked()),
            this, SLOT(finishButtonClicked()));
    connect(ui->addItemButton, SIGNAL(clicked()),
            this, SLOT(addItemButtonClicked()));
    connect(ui->renameItemButton, SIGNAL(clicked()),
            this, SLOT(renameItemButtonClicked()));
    connect(ui->removeItemButton, SIGNAL(clicked()),
            this, SLOT(removeItemButtonClicked()));
    connect(ui->itemsListWidget, SIGNAL(itemSelectionChanged()),
            this, SLOT(updateListButtons()));
    connect(ui->itemsListWidget, SIGNAL(itemChanged(QListWidgetItem*)),
            this, SLOT(updateFinishButton()));
    connect(ui->itemsListWidget, SIGNAL(itemChanged(QListWidgetItem*)),
            this, SLOT(updateDefaultComboBox()));
    connect(ui->defaultCombo, SIGNAL(activated(int)),
            this, SLOT(defaultItemChanged()));
    connect(ui->clearDefaultButton, SIGNAL(clicked()),
            this, SLOT(clearDefaultItemSlot()));
}

ComboboxFieldWizard::~ComboboxFieldWizard()
{
    delete ui;
}

void ComboboxFieldWizard::getFieldProperties(QString &displayProperties,
                                         QString &editProperties,
                                         QString &triggerProperties)
{
    //create display properties metadata string
    if (ui->requiredFieldCheckBox->isChecked())
        displayProperties.append("markEmpty:1;");
    //item list
    QString metadataItemList;
    metadataItemList.append("items:");
    int size = ui->itemsListWidget->count();
    for (int i = 0; i < size; i++) {
        QListWidgetItem *it = ui->itemsListWidget->item(i);
        QString s = it->text();
        //replace some escape codes
        s.replace(",", "\\comma");
        s.replace(":", "\\colon");
        s.replace(";", "\\semicolon");
        metadataItemList.append(s);
        if ((i + 1) < size)
            metadataItemList.append(",");
    }
    metadataItemList.append(";");
    displayProperties.append(metadataItemList);
    //default
    if (ui->defaultCombo->currentIndex() != -1) {
        displayProperties.append("default:" +
                                 QString::number(
                                     ui->defaultCombo->currentIndex()));
    }

    //create edit properties metadata string
    //nothing for now
    Q_UNUSED(editProperties);

    //create trigger properties metadata string
    //nothing for now
    Q_UNUSED(triggerProperties);
}

void ComboboxFieldWizard::loadField(const int fieldId, const int collectionId)
{
    AbstractFieldWizard::loadField(fieldId, collectionId);

    MetadataEngine *meta = &MetadataEngine::getInstance();

    //display properties
    QString displayProperties = meta->getFieldProperties(meta->DisplayProperty,
                                                         fieldId, collectionId);
    MetadataPropertiesParser displayParser(displayProperties);
    if (displayParser.getValue("markEmpty") == "1")
        ui->requiredFieldCheckBox->setChecked(true);
    //get items
    if (displayProperties.size()) {
        QStringList items = displayParser.getValue("items")
                .split(',', QString::SkipEmptyParts);
        foreach (QString s, items) {
            //replace some escape codes
            s.replace("\\comma", ",");
            s.replace("\\colon", ":");
            s.replace("\\semicolon", ";");
            QListWidgetItem *item = new QListWidgetItem(s,
                                                        ui->itemsListWidget);
            item->setFlags(item->flags() | Qt::ItemIsEditable);
        }
    }
    //default
    QString defaultValueString = displayParser.getValue("default");
    if (!defaultValueString.isEmpty()) {
        m_default = defaultValueString.toInt();
    }

    updateDefaultComboBox();
}


//-----------------------------------------------------------------------------
// Private slots
//-----------------------------------------------------------------------------

void ComboboxFieldWizard::updateFinishButton()
{
    bool enabled = ui->itemsListWidget->count();

    ui->finishButton->setEnabled(enabled);
}

void ComboboxFieldWizard::updateListButtons()
{
    bool hasSelection = ui->itemsListWidget->selectedItems().size() > 0;

    ui->renameItemButton->setEnabled(hasSelection);
    ui->removeItemButton->setEnabled(hasSelection);
}

void ComboboxFieldWizard::addItemButtonClicked()
{
    QListWidgetItem *item = new QListWidgetItem(tr("New item"),
                                                ui->itemsListWidget);
    item->setFlags(item->flags() | Qt::ItemIsEditable);
    ui->itemsListWidget->editItem(
                ui->itemsListWidget->item(
                    ui->itemsListWidget->count()-1));

    updateDefaultComboBox();
}

void ComboboxFieldWizard::renameItemButtonClicked()
{
    if (!ui->itemsListWidget->selectedItems().size())
        return;

    QListWidgetItem *item = ui->itemsListWidget->currentItem();
    ui->itemsListWidget->editItem(item);
}

void ComboboxFieldWizard::removeItemButtonClicked()
{
    if (!ui->itemsListWidget->selectedItems().size())
        return;

    //only if in editing mode
    if (m_currentEditMode == AbstractFieldWizard::ModifyEditMode) {
        int itemId = ui->itemsListWidget->currentRow();
        MetadataEngine *meta = &MetadataEngine::getInstance();
        int collectionId = m_collectionId;
        int fieldId = m_fieldId;
        QString tableName = meta->getTableName(collectionId);
        m_pendingSqlStatements.append(
                    QString("UPDATE \"%1\" SET \"%2\"=NULL WHERE \"%2\"=%3")
                    .arg(tableName).arg(fieldId).arg(itemId));

        //hide item from list (but don't remove, otherwise item ids gets messed up)
        QListWidgetItem *item = ui->itemsListWidget->item(itemId);
        item->setHidden(true);
    }

    updateDefaultComboBox();
}

void ComboboxFieldWizard::updateDefaultComboBox()
{
    ui->defaultCombo->clear();

    int defaultIndex = -1;
    int size = ui->itemsListWidget->count();
    int j = 0;
    for (int i = 0; i < size; i++) {
        QListWidgetItem *item = ui->itemsListWidget->item(i);
        if (!item->isHidden()) {
            ui->defaultCombo->addItem(item->text());
            if (i == m_default)
                defaultIndex = j;
            j++;
        }
    }

    ui->defaultCombo->setCurrentIndex(defaultIndex);
}

void ComboboxFieldWizard::defaultItemChanged()
{
    //only if in editing mode
    if (m_currentEditMode == AbstractFieldWizard::ModifyEditMode) {
        MetadataEngine *meta = &MetadataEngine::getInstance();

        int collectionId = m_collectionId;
        int fieldId = m_fieldId;
        QString tableName = meta->getTableName(collectionId);

        if (m_default != -1) {
            //Since default changed, set all NULL values to the old default
            //and new records with NULL (default) will use the new value
            m_pendingDefaultSqlStatement =
                    QString("UPDATE \"%1\" SET \"%2\"=%3 WHERE \"%2\" IS NULL")
                    .arg(tableName).arg(fieldId).arg(m_default);
        }
    }

    m_default = ui->defaultCombo->currentIndex();
}

void ComboboxFieldWizard::clearDefaultItemSlot()
{
    //only if in editing mode
    if (m_currentEditMode == AbstractFieldWizard::ModifyEditMode) {
        MetadataEngine *meta = &MetadataEngine::getInstance();

        int collectionId = m_collectionId;
        int fieldId = m_fieldId;
        QString tableName = meta->getTableName(collectionId);

        if (m_default != -1) {
            //Since default changed, set all NULL values to the old default
            //and new records with NULL (default) will use the new value
            m_pendingDefaultSqlStatement =
                    QString("UPDATE \"%1\" SET \"%2\"=%3 WHERE \"%2\" IS NULL")
                    .arg(tableName).arg(fieldId).arg(m_default);
        }
    }

    m_default = -1;
    ui->defaultCombo->setCurrentIndex(-1);
}

void ComboboxFieldWizard::finishButtonClicked()
{
    //remove all hidden item from list
    //(hidden items are those marked for removal)
    for (int i = 0; i < ui->itemsListWidget->count(); i++) {
        QListWidgetItem *item = ui->itemsListWidget->item(i);
        if (item->isHidden()) {
            item = ui->itemsListWidget->takeItem(i);
            delete item; //delete, see takeItem() docs
            i--; //decrement since count reduced by takeItem()
        }
    }

    //apply pending sql statements
    QSqlDatabase db = DatabaseManager::getInstance().getDatabase();
    if (!m_pendingDefaultSqlStatement.isEmpty())
        m_pendingSqlStatements.prepend(m_pendingDefaultSqlStatement);
    db.transaction();
    foreach (QString s, m_pendingSqlStatements) {
        QSqlQuery query(db);
        query.exec(s);
    }
    db.commit();

    emit finishSignal();
}


//-----------------------------------------------------------------------------
// Private
//-----------------------------------------------------------------------------
