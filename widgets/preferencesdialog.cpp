/*
 *  Copyright (c) 2012 Giorgio Wicklein <giowckln@gmail.com>
 */

//-----------------------------------------------------------------------------
// Hearders
//-----------------------------------------------------------------------------

#include "preferencesdialog.h"
#include "ui_preferencesdialog.h"
#include "../components/settingsmanager.h"
#include "../components/databasemanager.h"
#include "../components/sync_framework/syncengine.h"
#include "../components/sync_framework/syncsession.h"
#include "../components/filemanager.h"
#include "../utils/definitionholder.h"

#include <QtWidgets/QSpinBox>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QFontComboBox>


//-----------------------------------------------------------------------------
// Public
//-----------------------------------------------------------------------------

PreferencesDialog::PreferencesDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PreferencesDialog),
    m_settingsManager(nullptr),
    m_cloudChanged(false),
    m_softwareReset(false),
    m_appearanceChanged(false),
    m_databasePathChanged(false)
{
    ui->setupUi(this);

    //init
    ui->listWidget->setCurrentRow(0);
    m_settingsManager = new SettingsManager;

    //setup widgets
    initSettings();

    //load settings
    loadSettings();

    //connections
    connect(ui->closeButton, SIGNAL(clicked()),
            this, SLOT(accept()));
    connect(ui->listWidget, SIGNAL(currentRowChanged(int)),
            this, SLOT(currentCategoryChanged()));
    connect(ui->updatesComboBox, SIGNAL(activated(int)),
            this, SLOT(updatesComboBoxChanged()));
    connect(ui->cloudStatusComboBox, SIGNAL(activated(int)),
            this, SLOT(cloudStateComboBoxChanged()));
    connect(ui->cloudUnlinkButton, SIGNAL(clicked()),
            this, SLOT(cloundUnlinkButtonClicked()));
    connect(ui->softwareResetButton, SIGNAL(clicked()),
            this, SLOT(softwareResetButtonClicked()));
    connect(ui->formViewColorCombo, SIGNAL(activated(int)),
            this, SLOT(formViewColorComboChanged()));
    connect(ui->formViewFontSizeComboBox, SIGNAL(activated(int)),
            this, SLOT(formViewFontSizeComboChanged()));
    connect(ui->formViewFontCombo, &QFontComboBox::currentTextChanged,
            this, &PreferencesDialog::formViewFontComboChanged);
    connect(ui->tableRowSizeSpinBox, SIGNAL(editingFinished()),
            this, SLOT(tableViewRowSizeSpinChanged()));
    connect(ui->cacheImagesTableViewCheckBox, SIGNAL(stateChanged(int)),
            this, SLOT(cacheImagesTableViewCheckBoxChanged()));
    connect(ui->hideImagesTableViewCheckBox, &QCheckBox::stateChanged,
            this, &PreferencesDialog::hideImagesTableViewCheckBoxChanged);
    connect(ui->browseDbPathButton, SIGNAL(clicked(bool)),
            this, SLOT(browseDbPathButtonClicked()));
    connect(ui->resetDbPathButton, SIGNAL(clicked(bool)),
            this, SLOT(resetDbPathButtonClicked()));

    if (DefinitionHolder::APP_STORE) {
        //disable updates
        ui->updateGroupBox->setEnabled(false);
    }
}

PreferencesDialog::~PreferencesDialog()
{
    delete ui;

    if (m_settingsManager)
        delete m_settingsManager;
}

bool PreferencesDialog::cloudSyncChanged()
{
    return m_cloudChanged;
}

bool PreferencesDialog::softwareResetActivated()
{
    return m_softwareReset;
}

bool PreferencesDialog::appearanceChanged()
{
    return m_appearanceChanged;
}

bool PreferencesDialog::databasePathChanged()
{
    return m_databasePathChanged;
}


//-----------------------------------------------------------------------------
// Private slots
//-----------------------------------------------------------------------------

void PreferencesDialog::currentCategoryChanged()
{
    ui->stackedWidget->setCurrentIndex(ui->listWidget->currentRow());
}

void PreferencesDialog::updatesComboBoxChanged()
{
    m_settingsManager->saveCheckUpdates(
                ui->updatesComboBox->currentIndex() == 0);
}

void PreferencesDialog::cloudStateComboBoxChanged()
{
    bool enabled = ui->cloudStatusComboBox->currentIndex();

    if (enabled) {
        if (!m_settingsManager->isCloudSyncActive()) {
            m_settingsManager->setCloudSyncActive(true);
            //cause conflict
            SyncSession::LOCAL_DATA_CHANGED = true;
            m_cloudChanged = true;
        }
    } else {
        if ((!SyncSession::IS_READ_ONLY) && SyncSession::IS_ONLINE) {
            SyncEngine::getInstance().startCloseCloudSession();
        }
        SyncSession::IS_ONLINE = false;
        SyncSession::IS_ENABLED = false;
        SyncSession::IS_READ_ONLY = false;
        m_settingsManager->setCloudSyncActive(false);
        //cause conflict
        m_settingsManager->saveCloudLocalDataChanged(true);
        m_settingsManager->saveCloudSessionKey("invalid");
        m_cloudChanged = true;
    }
}

void PreferencesDialog::cloundUnlinkButtonClicked()
{
    bool close = (!SyncSession::IS_READ_ONLY) && SyncSession::IS_ONLINE;

    //set state
    SyncSession::IS_ONLINE = false;
    SyncSession::IS_ENABLED = false;
    SyncSession::IS_READ_ONLY = false;

    //close session if open
    if (close) {
        SyncEngine::getInstance().startCloseCloudSession();
    }

    //delete all cloud related settings
    m_settingsManager->deleteObjectProperties("cloudSync");

    //clear file manager's lists
    FileManager fm(this);
    fm.clearAllLists();

    //set status disabled
    ui->cloudStatusComboBox->setCurrentIndex(0);
}

void PreferencesDialog::softwareResetButtonClicked()
{
    m_softwareReset = true;
    accept();
}

void PreferencesDialog::formViewColorComboChanged()
{
    int colorIndex = ui->formViewColorCombo->currentIndex();
    QString colorCode = ui->formViewColorCombo->itemData(colorIndex).toString();

    m_settingsManager->saveProperty("backgroundColor", "formView", colorCode);
    m_settingsManager->saveProperty("backgroundColorIndex", "formView", colorIndex);

    m_appearanceChanged = true;

#ifdef Q_OS_WIN
    static bool messageShown = false;
    if (!messageShown) { //show only once until restart
        QMessageBox::information(this, tr("Restart Required"),
                                 tr("Please restart %1 to apply the new background color")
                                 .arg(DefinitionHolder::NAME));
        messageShown = true;
    }
#endif
}

void PreferencesDialog::formViewFontSizeComboChanged()
{
    int fontSizeIndex = ui->formViewFontSizeComboBox->currentIndex();
    QString fontSizeString = ui->formViewFontSizeComboBox->currentText();

    m_settingsManager->saveProperty("fontSize", "formView", fontSizeString);
    m_settingsManager->saveProperty("fontSizeIndex", "formView", fontSizeIndex);

    m_appearanceChanged = true;
}

void PreferencesDialog::formViewFontComboChanged()
{
    QString fontString = ui->formViewFontCombo->currentText();
    m_settingsManager->saveProperty("fontFamily", "formView", fontString);

    m_appearanceChanged = true;
}

void PreferencesDialog::tableViewRowSizeSpinChanged()
{
    int rows = ui->tableRowSizeSpinBox->value();

    m_settingsManager->saveProperty("rowSize", "tableView", rows);

    m_appearanceChanged = true;
}

void PreferencesDialog::cacheImagesTableViewCheckBoxChanged()
{
    bool b = ui->cacheImagesTableViewCheckBox->isChecked();

    m_settingsManager->saveProperty("cacheImages", "tableView", b);

    QMessageBox::information(this, tr("Restart required!"),
                             tr("A restart is required for this setting to take effect."),
                             QMessageBox::Ok);
}

void PreferencesDialog::hideImagesTableViewCheckBoxChanged()
{
    bool b = ui->hideImagesTableViewCheckBox->isChecked();

    m_settingsManager->saveProperty("hideImages", "tableView", b);

    QMessageBox::information(this, tr("Restart required!"),
                             tr("A restart is required for this setting to take effect."),
                             QMessageBox::Ok);
}

void PreferencesDialog::browseDbPathButtonClicked()
{
    QString currentDbPath;
    if (!ui->dbPathLineEdit->text().isEmpty()) {
        currentDbPath = ui->dbPathLineEdit->text();
    } else {
        currentDbPath = QFileInfo(DatabaseManager::getInstance()
                                  .getDatabasePath()).dir().path();
    }

    QString newPath = QFileDialog::getExistingDirectory(this,
                                                        tr("Select database directory"),
                                                        currentDbPath,
                                                        QFileDialog::ShowDirsOnly
                                                        | QFileDialog::DontResolveSymlinks);
    if (!newPath.isEmpty()) {
        ui->dbPathLineEdit->setText(newPath);
        ui->dbPathLineEdit->setEnabled(true);
        updateDatabasePath();
    }
}

void PreferencesDialog::resetDbPathButtonClicked()
{
    ui->dbPathLineEdit->clear();
    ui->dbPathLineEdit->setEnabled(false);
    updateDatabasePath();
}


//-----------------------------------------------------------------------------
// Private
//-----------------------------------------------------------------------------

void PreferencesDialog::initSettings()
{
    //form view background color
    QMap<QString, QString> colorMap;
    colorMap.insert("AliceBlue", "#F0F8FF");
    colorMap.insert("AntiqueWhite", "#FAEBD7");
    colorMap.insert("Beige", "#F5F5DC");
    colorMap.insert("Gainsboro", "#DCDCDC");
    colorMap.insert("Lavender", "#E6E6FA");
    colorMap.insert("LightCyan", "#E0FFFF");
    colorMap.insert("LightGoldenRodYellow", "#FAFAD2");
    colorMap.insert("LightGrey", "#D3D3D3");
    colorMap.insert("LightGrey2", "#E0E0E0");
    colorMap.insert("LightSkyBlue", "#87CEFA");
    colorMap.insert("LightSteelBlue", "#B0C4DE");
    colorMap.insert("LightYellow", "#FFFFE0");
    colorMap.insert("MistyRose", "#FFE4E1");
    colorMap.insert("Plum", "#DDA0DD");
    colorMap.insert("PowderBlue", "#B0E0E6");
    colorMap.insert("SeaShell", "#FFF5EE");
    colorMap.insert("SkyBlue", "#87CEEB");
    colorMap.insert("Violet", "#EE82EE");

    QMap<QString, QString>::const_iterator i = colorMap.constBegin();
    while (i != colorMap.constEnd()) {
        QPixmap pixmap(64, 64);
        pixmap.fill(QColor(i.value()));
        ui->formViewColorCombo->addItem(QIcon(pixmap),
                                        i.key(),
                                        i.value());
        ++i;
    }

    ui->formViewFontCombo->setCurrentText("Default");
}

void PreferencesDialog::loadSettings()
{
    if (!m_settingsManager->restoreCheckUpdates())
        ui->updatesComboBox->setCurrentIndex(1);
    if (m_settingsManager->isCloudSyncActive())
        ui->cloudStatusComboBox->setCurrentIndex(1);

    //database path
    QString dbPath = m_settingsManager->restoreCustomDatabaseDir();
    if (dbPath.isEmpty()) {
        dbPath = QFileInfo(DatabaseManager::getInstance()
                                          .getDatabasePath()).dir().path();
    }
    ui->dbPathLineEdit->setText(dbPath);

    //form view background color
    int colorCodeIndex = m_settingsManager->restoreProperty(
                "backgroundColorIndex", "formView").toInt();
    ui->formViewColorCombo->setCurrentIndex(colorCodeIndex);

    //form view font size
    int fontSizeIndex = m_settingsManager->restoreProperty(
                "fontSizeIndex", "formView").toInt();
    ui->formViewFontSizeComboBox->setCurrentIndex(fontSizeIndex);

    //form view font family
    QString fontFamily = m_settingsManager->restoreProperty(
                "fontFamily", "formView").toString();
    if (!fontFamily.isEmpty()) {
        ui->formViewFontCombo->setCurrentText(fontFamily);
    }

    //table view row size
    int tableRowSize =  m_settingsManager->restoreProperty(
                "rowSize", "tableView").toInt();
    if (tableRowSize) {
        ui->tableRowSizeSpinBox->setValue(tableRowSize);
    }

    //table view img caching
    bool cacheImg =  m_settingsManager->restoreProperty(
                "cacheImages", "tableView").toBool();
    ui->cacheImagesTableViewCheckBox->setChecked(cacheImg);

    //table view img hiding
    bool hideImg =  m_settingsManager->restoreProperty(
                "hideImages", "tableView").toBool();
    ui->hideImagesTableViewCheckBox->setChecked(hideImg);
}

void PreferencesDialog::updateDatabasePath()
{
    m_settingsManager->saveCustomDatabaseDir(ui->dbPathLineEdit->text());
    m_databasePathChanged = true;
}
