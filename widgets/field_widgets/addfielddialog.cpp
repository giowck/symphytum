/*
 *  Copyright (c) 2012 Giorgio Wicklein <giowckln@gmail.com>
 */

//-----------------------------------------------------------------------------
// Hearders
//-----------------------------------------------------------------------------

#include "addfielddialog.h"
#include "ui_addfielddialog.h"
#include "textfieldwizard.h"
#include "numberfieldwizard.h"
#include "datefieldwizard.h"
#include "creationdatefieldwizard.h"
#include "moddatefieldwizard.h"
#include "checkboxfieldwizard.h"
#include "imagefieldwizard.h"
#include "comboboxfieldwizard.h"
#include "progressfieldwizard.h"
#include "filesfieldwizard.h"
#include "urlfieldwizard.h"
#include "emailfieldwizard.h"


//-----------------------------------------------------------------------------
// Public
//-----------------------------------------------------------------------------

AddFieldDialog::AddFieldDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AddFieldDialog),
    m_currentMode(AbstractFieldWizard::NewEditMode),
    m_currentWizard(0),
    m_fieldType(MetadataEngine::TextType),
    m_fieldId(-1), m_collectionId(-1)
{
    ui->setupUi(this);

    init();
    createConnections();
}

AddFieldDialog::~AddFieldDialog()
{
    delete ui;
}

void AddFieldDialog::setCreationMode(AbstractFieldWizard::EditMode mode,
                                     int fieldId, int collectionId)
{
    switch (mode) {
    case AbstractFieldWizard::NewEditMode:
        ui->fieldIcon->setPixmap(QPixmap(":/images/icons/newfield.png"));
        ui->fieldOperationLabel->setText(tr("Create Field"));
        ui->fieldTypeListWidget->setEnabled(true);
        ui->descriptionLabel->setVisible(true);
        break;
    case AbstractFieldWizard::DuplicateEditMode:
        ui->fieldIcon->setPixmap(QPixmap(":/images/icons/duplicatefield.png"));
        ui->fieldOperationLabel->setText(tr("Duplicate Field"));
        ui->fieldTypeListWidget->setEnabled(false);
        ui->descriptionLabel->setVisible(false);
        loadFieldStartProperties(fieldId, collectionId);
        break;
    case AbstractFieldWizard::ModifyEditMode:
        ui->fieldIcon->setPixmap(QPixmap(":/images/icons/duplicatefield.png"));
        ui->fieldOperationLabel->setText(tr("Modify Field"));
        ui->fieldTypeListWidget->setEnabled(false);
        ui->descriptionLabel->setVisible(false);
        loadFieldStartProperties(fieldId, collectionId);
        break;
    }

    //save args
    m_currentMode = mode;
    m_fieldId = fieldId;
    m_collectionId = collectionId;
}


//-----------------------------------------------------------------------------
// Private slots
//-----------------------------------------------------------------------------

void AddFieldDialog::cancelButtonClicked()
{
    cleanDialog();
    close();
}

void AddFieldDialog::nextButtonClicked()
{
    if (!m_currentWizard) { //only if not already created
        QString fieldName = ui->fieldNameLineEdit->text();
        fieldName.replace('"', " "); //avoid double-quotes

        //create apropriate wizard
        int item = ui->fieldTypeListWidget->currentRow();
        switch (item) {
        case 0:
            m_currentWizard = new TextFieldWizard(fieldName, this, m_currentMode);
            m_fieldType = MetadataEngine::TextType;
            break;
        case 1:
            m_currentWizard = new NumberFieldWizard(fieldName, this, m_currentMode);
            m_fieldType = MetadataEngine::NumericType;
            break;
        case 2:
            m_currentWizard = new DateFieldWizard(fieldName, this, m_currentMode);
            m_fieldType = MetadataEngine::DateType;
            break;
        case 3:
            m_currentWizard = new CreationDateFieldWizard(fieldName, this, m_currentMode);
            m_fieldType = MetadataEngine::CreationDateType;
            break;
        case 4:
            m_currentWizard = new ModDateFieldWizard(fieldName, this, m_currentMode);
            m_fieldType = MetadataEngine::ModDateType;
            break;
        case 5:
            m_currentWizard = new CheckboxFieldWizard(fieldName, this, m_currentMode);
            m_fieldType = MetadataEngine::CheckboxType;
            break;
        case 6:
            m_currentWizard = new ComboboxFieldWizard(fieldName, this, m_currentMode);
            m_fieldType = MetadataEngine::ComboboxType;
            break;
        case 7:
            m_currentWizard = new ProgressFieldWizard(fieldName, this, m_currentMode);
            m_fieldType = MetadataEngine::ProgressType;
            break;
        case 8:
            m_currentWizard = new ImageFieldWizard(fieldName, this, m_currentMode);
            m_fieldType = MetadataEngine::ImageType;
            break;
        case 9:
            m_currentWizard = new FilesFieldWizard(fieldName, this, m_currentMode);
            m_fieldType = MetadataEngine::FilesType;
            break;
        case 10:
            m_currentWizard = new URLFieldWizard(fieldName, this, m_currentMode);
            m_fieldType = MetadataEngine::URLTextType;
            break;
        case 11:
            m_currentWizard = new EmailFieldWizard(fieldName, this, m_currentMode);
            m_fieldType = MetadataEngine::EmailTextType;
            break;
        default:
            return;
        }
        ui->stackedWidget->addWidget(m_currentWizard);

        //connections
        connect(m_currentWizard, SIGNAL(backSignal()),
                this, SLOT(backSlot()));
        connect(m_currentWizard, SIGNAL(finishSignal()),
                this, SLOT(finishSlot()));

        //load field properties into wizard if necessary
        if ((m_currentMode == AbstractFieldWizard::ModifyEditMode) ||
                (m_currentMode == AbstractFieldWizard::DuplicateEditMode))
            m_currentWizard->loadField(m_fieldId, m_collectionId);
    }

    //show wizard
    ui->stackedWidget->setCurrentIndex(1);
}

void AddFieldDialog::updateFieldDescription()
{
    int item = ui->fieldTypeListWidget->currentRow();

    switch (item) {
    case 0:
        ui->descriptionLabel->setText(tr("Generic input field for all kind "
                                         "of data. Text Fields can be resized "
                                         "to create multi-lined text areas."
                                         ));
        break;
    case 1:
        ui->descriptionLabel->setText(tr("Input field for numbers with support"
                                         " for decimal and scientific notation."
                                         ));
        break;
    case 2:
        ui->descriptionLabel->setText(tr("Input and display field for date "
                                         "and time values."
                                         ));
        break;
    case 3:
        ui->descriptionLabel->setText(tr("Display field for record creation date."
                                         ));
        break;
    case 4:
        ui->descriptionLabel->setText(tr("Display field for record modification date."
                                         ));
        break;
    case 5:
        ui->descriptionLabel->setText(tr("Status field with two possible values:"
                                         " checked (yes) and unchecked (no)."
                                         " Checkboxes are generally used to answer"
                                         " simple yes/no questions."
                                         ));
        break;
    case 6:
        ui->descriptionLabel->setText(tr("Drop-down list which allows to choose"
                                         " one value from a predefined list."
                                         ));
        break;
    case 7:
        ui->descriptionLabel->setText(tr("A progressbar to indicate the "
                                         "current progress state of a task."
                                         ));
        break;
    case 8:
        ui->descriptionLabel->setText(tr("Display field for image files."
                                         " Supported file formats: "
                                         "PNG, JPG/JPEG, GIF, TIFF, BMP, SVG."
                                         ));
        break;
    case 9:
        ui->descriptionLabel->setText(tr("File management. "
                                         "This field allows you to link "
                                         "files to a specific record. "
                                         "Added files are copied to the database "
                                         "and can be opened/modified later on."
                                         ));
        break;
    case 10:
        ui->descriptionLabel->setText(tr("URL text links. "
                                         "Input field for web links."
                                         "Each link can be opened with one click "
                                         "on the inline open link button."
                                         ));
        break;
    case 11:
        ui->descriptionLabel->setText(tr("Email address. "
                                         "Input field for email adresses. "
                                         "New email can be written with one click "
                                         "on the inline email button"
                                         ));
        break;
    default:
        ui->descriptionLabel->setText(tr("Select a field type from "
                                         "the list on the left"));
        break;
    }
}

void AddFieldDialog::updateNextButtonState()
{
    bool enable;
    bool hasSelection = ui->fieldTypeListWidget->selectionModel()->hasSelection();
    bool isEmpty = ui->fieldNameLineEdit->text().trimmed().isEmpty();

    enable = hasSelection && (!isEmpty);

    ui->nextButton->setEnabled(enable);
    if (enable)
        ui->nextButton->setDefault(true);
    else
        ui->cancelButton->setDefault(true);

    //reset wizard cause if the user comes back to the start
    //page by using the back button and then selects
    //a different field type which needs a different wizard type
    deleteCurrentWizard();
}

void AddFieldDialog::backSlot()
{
    ui->stackedWidget->setCurrentIndex(ui->stackedWidget->currentIndex() - 1);
}

void AddFieldDialog::finishSlot()
{
    m_currentWizard->createField(m_fieldType, m_currentMode);

    cleanDialog();
    accept();
}

void AddFieldDialog::setDefaultNameSlot()
{
    switch (ui->fieldTypeListWidget->currentRow()) {
    case 3: //creation date type
        ui->fieldNameLineEdit->setText(tr("Created on"));
        break;
    case 4: //modification date type
        ui->fieldNameLineEdit->setText(tr("Modified on"));
        break;
    }
}


//-----------------------------------------------------------------------------
// Private
//-----------------------------------------------------------------------------

void AddFieldDialog::init()
{
    ui->fieldTypeListWidget->addItem(tr("Text"));
    ui->fieldTypeListWidget->addItem(tr("Number"));
    ui->fieldTypeListWidget->addItem(tr("Date"));
    ui->fieldTypeListWidget->addItem(tr("Creation Date"));
    ui->fieldTypeListWidget->addItem(tr("Modification Date"));
    ui->fieldTypeListWidget->addItem(tr("Checkbox"));
    ui->fieldTypeListWidget->addItem(tr("Combobox"));
    ui->fieldTypeListWidget->addItem(tr("Progress"));
    ui->fieldTypeListWidget->addItem(tr("Image"));
    ui->fieldTypeListWidget->addItem(tr("File list"));
    ui->fieldTypeListWidget->addItem(tr("Web Link"));
    ui->fieldTypeListWidget->addItem(tr("Email Address"));
}

void AddFieldDialog::createConnections()
{
    //buttons
    connect(ui->cancelButton, SIGNAL(clicked()),
            this, SLOT(cancelButtonClicked()));
    connect(this, SIGNAL(rejected()),
            this, SLOT(cancelButtonClicked()));
    connect(ui->nextButton, SIGNAL(clicked()),
            this, SLOT(nextButtonClicked()));
    connect(ui->fieldTypeListWidget, SIGNAL(itemSelectionChanged()),
            ui->fieldNameLineEdit, SLOT(setFocus()));

    //description
    connect(ui->fieldTypeListWidget, SIGNAL(itemSelectionChanged()),
            this, SLOT(updateFieldDescription()));

    //state
    connect(ui->fieldTypeListWidget, SIGNAL(itemSelectionChanged()),
            this, SLOT(updateNextButtonState()));
    connect(ui->fieldNameLineEdit, SIGNAL(textChanged(QString)),
            this, SLOT(updateNextButtonState()));

    //default names
    connect(ui->fieldTypeListWidget, SIGNAL(itemSelectionChanged()),
            this, SLOT(setDefaultNameSlot()));

}

void AddFieldDialog::cleanDialog()
{
    ui->stackedWidget->setCurrentIndex(0);
    ui->fieldTypeListWidget->setCurrentRow(-1);
    ui->fieldNameLineEdit->clear();
    ui->stackedWidget->removeWidget(m_currentWizard);

    //reset wizard
    deleteCurrentWizard();

    //set focus for next time dialog is shown
    ui->fieldTypeListWidget->setFocus();
}

void AddFieldDialog::deleteCurrentWizard()
{
    if (m_currentWizard) {
        delete m_currentWizard;
        m_currentWizard = 0;
    }
}

void AddFieldDialog::loadFieldStartProperties(int fieldId, int collectionId)
{
    MetadataEngine *meta = &MetadataEngine::getInstance();
    if (!collectionId)
        collectionId = meta->getCurrentCollectionId();

    QString fieldName = meta->getFieldName(fieldId, collectionId);
    MetadataEngine::FieldType fieldType = meta->getFieldType(fieldId, collectionId);
    int fieldTypeInList = ((int) (fieldType)) - 1; //-1 cause field type starts at 1
                                                   //but list at 0

    ui->fieldTypeListWidget->setCurrentRow(fieldTypeInList);
    ui->fieldNameLineEdit->setText(fieldName);
}
