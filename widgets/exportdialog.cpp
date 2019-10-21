/*
 *  Copyright (c) 2012 Giorgio Wicklein <giowckln@gmail.com>
 */

//-----------------------------------------------------------------------------
// Hearders
//-----------------------------------------------------------------------------

#include "exportdialog.h"
#include "ui_exportdialog.h"

#include "../components/metadataengine.h"
#include "../components/databasemanager.h"
#include "../utils/metadatapropertiesparser.h"

#include <QtSql/QSqlQuery>
#include <QtWidgets/QFileDialog>
#include <QtGui/QDesktopServices>
#include <QtCore/QDateTime>
#include <QtCore/QFile>
#include <QtWidgets/QMessageBox>
#include <QtCore/QTextStream>


//-----------------------------------------------------------------------------
// Public
//-----------------------------------------------------------------------------

ExportDialog::ExportDialog(const int collectionId,
                           QList<int> &selectionRecordIdList,
                           QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ExportDialog),
    m_recordList(selectionRecordIdList),
    m_collectionId(collectionId),
    m_exportCancelled(false)
{
    ui->setupUi(this);

    //init
    initUiCSV();

    //setup ui text
    if (!selectionRecordIdList.isEmpty()) {
        ui->selectedRecordsRadio->setChecked(true);
        int count = selectionRecordIdList.size();
        //QString text = tr("%n file(s)", "", fileCount);
        //BUG workaround: investigate why it doesn't work (http://qt-project.org/doc/qt-4.8/i18n-source-translation.html#handling-plurals)
        QString itemText = (count == 1 )? tr("item") : tr("items");
        QString text = tr("Selected records (%1 %2)")
                .arg(QString::number(count))
                .arg(itemText);
        ui->selectedRecordsRadio->setText(text);
    }

    //components
    m_metadataEngine = &MetadataEngine::getInstance();

    //connections
    connect(ui->cancelButton, SIGNAL(clicked()),
            this, SLOT(reject()));
    connect(ui->nextButton, SIGNAL(clicked()),
            this, SLOT(nextButtonClicked()));
    connect(ui->backCSVButton, SIGNAL(clicked()),
            this, SLOT(backButtonClicked()));
    connect(ui->cancelProgressButton, SIGNAL(clicked()),
            this, SLOT(cancelProgressButtonClicked()));
    connect(ui->exportCSVButton, SIGNAL(clicked()),
            this, SLOT(exportCSVButtonClicked()));
}

ExportDialog::~ExportDialog()
{
    delete ui;
}


//-----------------------------------------------------------------------------
// Private slots
//-----------------------------------------------------------------------------

void ExportDialog::nextButtonClicked()
{
    initRecordListToExport();

    switch (ui->formatCombo->currentIndex()) {
    case 0:
        ui->stackedWidget->setCurrentIndex(1);
        break;
    default:
        //nothing
        break;
    }
}

void ExportDialog::backButtonClicked()
{
    int current = ui->stackedWidget->currentIndex();
    ui->stackedWidget->setCurrentIndex(current - 1);
}

void ExportDialog::cancelProgressButtonClicked()
{
    m_exportCancelled = true;
}

void ExportDialog::exportCSVButtonClicked()
{
    ui->stackedWidget->setCurrentIndex(2);
    qApp->processEvents();

    //get output file
    QString documentsDir = QStandardPaths::standardLocations(
                QStandardPaths::DocumentsLocation).at(0);
    QString fileName = QFileDialog::getSaveFileName(this,
                                                    tr("Save CSV file"),
                                                    documentsDir,
                                                    tr("CSV file(*.csv)"));
    if (fileName.isEmpty()) {
        ui->stackedWidget->setCurrentIndex(1);
        return;
    }
    if (!fileName.contains(".csv", Qt::CaseInsensitive))
        fileName.append(".csv");

    //init field separator
    QChar separator;
    if (ui->separatorCombo->currentIndex() == 0)
        separator = ',';
    else if (ui->separatorCombo->currentIndex() == 1)
        separator = ';';

    //init embedded escape
    QChar embeddedEscape('"');

    //init terminate line
    QString terminateLine = "\r\n";

    //open file and set encoding
    QFile fileCvs(fileName);
    if (!fileCvs.open(QFile::WriteOnly)) {
        QMessageBox::critical(this, QObject::tr("Error in CSV export"),
                              QObject::tr("Unable to create CSV file!"));
        reject();
        return;
    }
    QTextStream out(&fileCvs);
    out.setCodec(ui->encodingCombo->itemData(
                     ui->encodingCombo->currentIndex())
                 .toString().toStdString().c_str());

    //get all records to export
    QString tableName = m_metadataEngine->getTableName(m_collectionId);
    QSqlDatabase db = DatabaseManager::getInstance().getDatabase();
    QSqlQuery query(db);

    QString recordListString;
    foreach (int id, m_recordList) {
        recordListString.append(QString::number(id) + ",");
    }
    //remove last ','
    recordListString.remove(recordListString.size()-1, 1);
    QString sql = QString("SELECT * FROM %1 WHERE _id IN (%2)")
            .arg(tableName).arg(recordListString);
    query.exec(sql);

    //generate header
    int fieldCount = m_metadataEngine->getFieldCount(m_collectionId);
    for (int i = 1; i < fieldCount; i++) { // 1 because of _id
        QString csvString = m_metadataEngine->getFieldName(i, m_collectionId);
        //if name contains " or , (for example) handle CSV escape
        if (csvString.contains(embeddedEscape) || csvString.contains(separator)) {
            csvString.replace(embeddedEscape, QString(2, embeddedEscape));
            csvString.prepend(embeddedEscape);
            csvString.append(embeddedEscape);
        }
        out << csvString;
        if ((i + 1) < fieldCount)
            out << separator;
    }
    out << terminateLine;

    //generate text
    while (query.next()) {
        for (int i = 1; i < fieldCount; i++) { // 1 because of _id
            QString csvString;

            MetadataEngine::FieldType type = m_metadataEngine->getFieldType(i, m_collectionId);
            switch (type) {
            case MetadataEngine::TextType:
            case MetadataEngine::URLTextType:
            case MetadataEngine::EmailTextType:
                csvString.append(textTypeItemText(query.value(i), i));
                break;
            case MetadataEngine::NumericType:
                csvString.append(numericTypeItemText(query.value(i), i));
                break;
            case MetadataEngine::CreationDateType:
            case MetadataEngine::ModDateType:
            case MetadataEngine::DateType:
                csvString.append(dateTypeItemText(query.value(i), i));
                break;
            case MetadataEngine::CheckboxType:
                csvString.append(checkboxTypeItemText(query.value(i), i));
                break;
            case MetadataEngine::ComboboxType:
                csvString.append(comboboxTypeItemText(query.value(i), i));
                break;
            case MetadataEngine::ProgressType:
                csvString.append(progressTypeItemText(query.value(i), i));
                break;
            case MetadataEngine::ImageType:
                csvString.append(imageTypeItemText(query.value(i), i));
                break;
            case MetadataEngine::FilesType:
                csvString.append(filesTypeItemText(query.value(i), i));
                break;
            default:
                csvString.append(textTypeItemText(query.value(i), i));
                break;
            }

            //if name contains " or , (for example) handle CSV escape and if \n (newline)
            if (csvString.contains(embeddedEscape) || csvString.contains(separator) || csvString.contains("\n")) {
                csvString.replace(embeddedEscape, QString(2, embeddedEscape));
                csvString.prepend(embeddedEscape);
                csvString.append(embeddedEscape);
            }
            out << csvString;
            if ((i + 1) < fieldCount)
                out << separator;
        }
        out << terminateLine;

        ui->progressBar->setValue(ui->progressBar->value()+1);
        qApp->processEvents();
        if (m_exportCancelled) {
            fileCvs.close();
            QFile::remove(fileName);
            reject();
            return;
        }
    }

    fileCvs.close();
    accept();
}


//-----------------------------------------------------------------------------
// Private
//-----------------------------------------------------------------------------

void ExportDialog::initUiCSV()
{
    ui->encodingCombo->addItem("Unicode (UTF-8)",
                               QString("UTF-8"));
    ui->encodingCombo->addItem("Western Europe (Euro sign) (ISO 8859-15)",
                               QString("ISO 8859-15"));
    ui->encodingCombo->addItem("Western Europe (Apple Roman)",
                               QString("Apple Roman"));
    ui->encodingCombo->insertSeparator(3);
    ui->encodingCombo->addItem("Arabic (ISO 8859-6)",
                               QString("ISO 8859-6"));
    ui->encodingCombo->addItem("Baltic languages (ISO 8859-13)",
                               QString("ISO 8859-13"));
    ui->encodingCombo->addItem("Celtic languages (Irish Gaelic, Scottish, Welsh) (ISO 8859-14)",
                               QString("ISO 8859-14"));
    ui->encodingCombo->addItem("Central, Eastern and Southern Europe (ISO 8859-16)",
                               QString("ISO 8859-16"));
    ui->encodingCombo->addItem("Chinese National Standard (GB18030-0)",
                               QString("GB18030-0"));
    ui->encodingCombo->addItem("Traditional Chinese (Big5)",
                               QString("Big5"));
    ui->encodingCombo->addItem("Traditional Chinese (Big5-HKSCS)",
                               QString("Big5-HKSCS"));
    ui->encodingCombo->addItem("Cyrillic Alphabet (ISO 8859-5)",
                               QString("ISO 8859-5"));
    ui->encodingCombo->addItem("Greek (ISO 8859-7)",
                               QString("ISO 8859-7"));
    ui->encodingCombo->addItem("Hebrew (ISO 8859-8)",
                               QString("ISO 8859-8"));
    ui->encodingCombo->addItem("Indian Standard Code for Information Interchange (Iscii-Bng)",
                               QString("Iscii-Bng"));
    ui->encodingCombo->addItem("Indian Standard Code for Information Interchange (Iscii-Dev)",
                               QString("Iscii-Dev"));
    ui->encodingCombo->addItem("Indian Standard Code for Information Interchange (Iscii-Gjr)",
                               QString("Iscii-Gjr"));
    ui->encodingCombo->addItem("Indian Standard Code for Information Interchange (Iscii-Knd)",
                               QString("Iscii-Knd"));
    ui->encodingCombo->addItem("Indian Standard Code for Information Interchange (Iscii-Mlm)",
                               QString("Iscii-Mlm"));
    ui->encodingCombo->addItem("Indian Standard Code for Information Interchange (Iscii-Ori)",
                               QString("Iscii-Ori"));
    ui->encodingCombo->addItem("Indian Standard Code for Information Interchange (Iscii-Pnj)",
                               QString("Iscii-Pnj"));
    ui->encodingCombo->addItem("Indian Standard Code for Information Interchange (Iscii-Tlg)",
                               QString("Iscii-Tlg"));
    ui->encodingCombo->addItem("Indian Standard Code for Information Interchange (Iscii-Tml)",
                               QString("Iscii-Tml"));
    ui->encodingCombo->addItem("Japanese (EUC-JP)",
                               QString("EUC-JP"));
    ui->encodingCombo->addItem("Japanese (ISO 2022-JP)",
                               QString("ISO 2022-JP"));
    ui->encodingCombo->addItem("Japanese (JIS X 0201)",
                               QString("JIS X 0201"));
    ui->encodingCombo->addItem("Japanese (JIS X 0208)",
                               QString("JIS X 0208"));
    ui->encodingCombo->addItem("Japanese (Shift-JIS)",
                               QString("Shift-JIS"));
    ui->encodingCombo->addItem("Korean (EUC-KR)",
                               QString("EUC-KR"));
    ui->encodingCombo->addItem("Russian (KOI8-R)",
                               QString("KOI8-R"));
    ui->encodingCombo->addItem("Tamil Script Code for Information Interchange (TSCII)",
                               QString("TSCII"));
    ui->encodingCombo->addItem("Thai Industrial Standard (TIS-620)",
                               QString("TIS-620"));
    ui->encodingCombo->addItem("Turkish (ISO 8859-9)",
                               QString("ISO 8859-9"));
    ui->encodingCombo->addItem("Ukrainian (KOI8-U)",
                               QString("KOI8-U"));
    ui->encodingCombo->addItem("Unicode (UTF-16)",
                               QString("UTF-16"));
    ui->encodingCombo->addItem("Unicode (UTF-16BE)",
                               QString("UTF-16BE"));
    ui->encodingCombo->addItem("Unicode (UTF-16LE)",
                               QString("UTF-16LE"));
    ui->encodingCombo->addItem("Unicode (UTF-32)",
                               QString("UTF-32"));
    ui->encodingCombo->addItem("Unicode (UTF-32BE)",
                               QString("UTF-32BE"));
    ui->encodingCombo->addItem("Unicode (UTF-32LE)",
                               QString("UTF-32LE"));
    ui->encodingCombo->addItem("Western and Central Europe (ISO 8859-2)",
                               QString("ISO 8859-2"));
    ui->encodingCombo->addItem("Western and South Europe (Turkish, Maltese, Esperanto) (ISO 8859-3)",
                               QString("ISO 8859-3"));
    ui->encodingCombo->addItem("Western Europe (ISO 8859-1)",
                               QString("ISO 8859-1"));
    ui->encodingCombo->addItem("Western Europe and Baltics (ISO 8859-4)",
                               QString("ISO 8859-4"));
    ui->encodingCombo->addItem("Western Europe and Nordic languages (ISO 8859-10)",
                               QString("ISO 8859-10"));

    //set default
    int defaultCodec = 0;
#if defined(Q_OS_WIN)
    defaultCodec = 1;
#elif defined(Q_OS_OSX)
    defaultCodec = 2;
#elif defined(Q_OS_LINUX)
    defaultCodec = 0;
#endif
    ui->encodingCombo->setCurrentIndex(defaultCodec);
}

void ExportDialog::initRecordListToExport()
{
    //if all records selected init list
    if (ui->allRecordsRadio->isChecked()) {
        m_recordList.clear();

        QString tableName = m_metadataEngine->getTableName(m_collectionId);
        QSqlDatabase db = DatabaseManager::getInstance().getDatabase();
        QSqlQuery query(db);

        //get all record ids for current collection
        QString sql = QString("SELECT _id FROM %1").arg(tableName);
        query.exec(sql);

        while (query.next()) {
            bool ok;
            int id = query.value(0).toInt(&ok);
            if (ok)
                m_recordList.append(id);
        }
    }

    ui->progressBar->setRange(1, m_recordList.size());
    ui->progressBar->setValue(1);
}

QString ExportDialog::textTypeItemText(const QVariant &data, int fieldId)
{
    Q_UNUSED(fieldId)

    return data.toString();
}

QString ExportDialog::numericTypeItemText(const QVariant &data, int fieldId)
{
    QString dataString;
    bool empty = data.toString().isEmpty();

    //adapt to display properties
    QString metadata =
            m_metadataEngine->getFieldProperties(MetadataEngine::DisplayProperty,
                                                 fieldId);
    MetadataPropertiesParser parser(metadata);
    QString v;

    int precision;
    v = parser.getValue("precision");
    precision = v.toInt();

    v = parser.getValue("displayMode");
    if (v == "auto") {
        if (!empty)
            dataString = data.toString();
    } else if (v == "decimal") {
        if (!empty)
            dataString = QString::number(data.toDouble(), 'f', precision);
    } else if (v == "scientific") {
        if (!empty)
            dataString = QString::number(data.toDouble(), 'e', precision);
    } else {
        if (!empty)
            dataString = data.toString(); //if display mode not specified
    }

    //make use of the correct decimal point char
    QLocale locale;
    dataString.replace(".", locale.decimalPoint());

    return dataString;
}

QString ExportDialog::dateTypeItemText(const QVariant &data, int fieldId)
{
    QLocale locale;
    QString dateFormat = locale.dateTimeFormat(QLocale::ShortFormat);

    //adapt to display properties
    QString metadata =
            m_metadataEngine->getFieldProperties(MetadataEngine::DisplayProperty,
                                                 fieldId);
    MetadataPropertiesParser parser(metadata);
    if (metadata.size() > 0) {
        QString v;

        v = parser.getValue("dateFormat");
        if (v == "1")
            dateFormat = locale.dateTimeFormat(QLocale::ShortFormat);
        else if (v == "2")
            dateFormat = locale.dateFormat(QLocale::ShortFormat);
        else if (v == "3")
            dateFormat = "ddd MMM d hh:mm yyyy";
        else if (v == "4")
            dateFormat = "ddd MMM d yyyy";
        else if (v == "5")
            dateFormat = "yyyy-MM-dd hh:mm";
        else if (v == "6")
            dateFormat = "yyyy-MM-dd";
    }

    return data.toDateTime().toString(dateFormat);
}

QString ExportDialog::checkboxTypeItemText(const QVariant &data, int fieldId)
{
    Q_UNUSED(fieldId)

    QString text;
    bool checked = data.toInt();

    if(checked)
        text.append(tr("Yes"));
    else
        text.append(tr("No"));

    return text;
}

QString ExportDialog::comboboxTypeItemText(const QVariant &data, int fieldId)
{
    bool ok;
    int itemId = data.toInt(&ok);
    if (!ok) itemId = -1;
    QString itemString;

    //adapt to display properties
    QString metadata =
            m_metadataEngine->getFieldProperties(MetadataEngine::DisplayProperty,
                                                 fieldId);
    MetadataPropertiesParser parser(metadata);
    if (metadata.size() > 0) {
        QString v;

        //load items from display properties
        QStringList items = parser.getValue("items")
                .split(',', QString::SkipEmptyParts);

        //handle default
        v = parser.getValue("default");
        if ((!v.isEmpty()) && (itemId == -1)) {
            bool ok;
            itemId = v.toInt(&ok);
            if (!ok) itemId = -1;
        }

        if ((itemId != -1) && (itemId < items.size()))
            itemString = items.at(itemId);

        //replace some escape codes
        itemString.replace("\\comma", ",");
        itemString.replace("\\colon", ":");
        itemString.replace("\\semicolon", ";");
        itemString.replace("\\doublequote", "\"");
        itemString.replace("\\singlequote", "'");
    }

    return itemString;
}

QString ExportDialog::progressTypeItemText(const QVariant &data, int fieldId)
{
    QString text;

    int max = 100;
    int value = data.toInt();

    //display properties
    QString metadata =
            m_metadataEngine->getFieldProperties(MetadataEngine::DisplayProperty,
                                                 fieldId);
    MetadataPropertiesParser parser(metadata);
    if (metadata.size() > 0) {
        max = parser.getValue("max").toInt();
    }

    //calc percentage
    int percentage = (((double) value) / max) * 100.0;
    text.append(QString::number(percentage) + "%");

    return text;
}

QString ExportDialog::imageTypeItemText(const QVariant &data, int fieldId)
{
    Q_UNUSED(fieldId)

    QString fileName, hashName, origDirPath;
    QDateTime dateTime;
    m_metadataEngine->getContentFile(data.toInt(),
                                     fileName,
                                     hashName, dateTime, origDirPath);
    return fileName + "/" + hashName;
}

QString ExportDialog::filesTypeItemText(const QVariant &data, int fieldId)
{
    Q_UNUSED(fieldId)

    QString dataString = data.toString();

    int fileCount = dataString.split(',', QString::SkipEmptyParts).size();
    //opt.text = tr("%n file(s)", "", fileCount);
    //BUG workaround: investigate why it doesn't work (http://qt-project.org/doc/qt-4.8/i18n-source-translation.html#handling-plurals)
    QString countString = (fileCount == 1 )? tr("%1 file").arg(fileCount) :
                                             tr("%1 files").arg(fileCount);

    return countString;
}
