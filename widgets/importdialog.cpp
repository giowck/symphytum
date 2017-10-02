/*
 *  Copyright (c) 2012 Giorgio Wicklein <giowckln@gmail.com>
 */

//-----------------------------------------------------------------------------
// Hearders
//-----------------------------------------------------------------------------

#include "importdialog.h"
#include "ui_importdialog.h"

#include "../components/metadataengine.h"
#include "../components/databasemanager.h"
#include "../components/sync_framework/syncsession.h"

#include <QtSql/QSqlQuery>
#include <QtWidgets/QFileDialog>
#include <QtGui/QDesktopServices>
#include <QtCore/QFile>
#include <QtWidgets/QMessageBox>
#include <QtCore/QTextStream>


//-----------------------------------------------------------------------------
// Public
//-----------------------------------------------------------------------------

ImportDialog::ImportDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ImportDialog),
    m_importCancelled(false)
{
    ui->setupUi(this);

    //init
    initUiCSV();

    //components
    m_metadataEngine = &MetadataEngine::getInstance();

    //connections
    connect(ui->nextButton, SIGNAL(clicked()),
            this, SLOT(nextButtonClicked()));
    connect(ui->backCSVButton, SIGNAL(clicked()),
            this, SLOT(backButtonClicked()));
    connect(ui->cancelButton, SIGNAL(clicked()),
            this, SLOT(reject()));
    connect(ui->importCancelButton, SIGNAL(clicked()),
            this, SLOT(cancelImportButtonClicked()));
    connect(ui->importCSVButton, SIGNAL(clicked()),
            this, SLOT(importCSVButtonClicked()));
    connect(ui->collectionNameLineEdit, SIGNAL(textEdited(QString)),
            this, SLOT(updateImportCSVButton()));
}

ImportDialog::~ImportDialog()
{
    delete ui;
}


//-----------------------------------------------------------------------------
// Private slots
//-----------------------------------------------------------------------------

void ImportDialog::nextButtonClicked()
{
    switch (ui->formatCombo->currentIndex()) {
    case 0:
        ui->stackedWidget->setCurrentIndex(1);
        break;
    default:
        //nothing
        break;
    }
}

void ImportDialog::backButtonClicked()
{
    int current = ui->stackedWidget->currentIndex();
    ui->stackedWidget->setCurrentIndex(current - 1);
}

void ImportDialog::cancelImportButtonClicked()
{
    m_importCancelled = true;
}

void ImportDialog::updateImportCSVButton()
{
    bool enabled = !ui->collectionNameLineEdit->text().trimmed().isEmpty();
    ui->importCSVButton->setEnabled(enabled);
}

void ImportDialog::importCSVButtonClicked()
{
    ui->stackedWidget->setCurrentIndex(2);
    qApp->processEvents();

    //get output file
    QString documentsDir = QStandardPaths::standardLocations(
                QStandardPaths::DocumentsLocation).at(0);
    QString fileName = QFileDialog::getOpenFileName(this,
                                                    tr("Open CSV file"),
                                                    documentsDir,
                                                    tr("CSV file(*.csv)"));
    if (fileName.isEmpty()) {
        ui->stackedWidget->setCurrentIndex(1);
        return;
    }

    //init field separator
    QChar separator;
    if (ui->separatorCombo->currentIndex() == 0)
        separator = ',';
    else if (ui->separatorCombo->currentIndex() == 1)
        separator = ';';

    //init embedded escape
    QChar embeddedEscape('"');

    //open file and set encoding
    QFile fileCvs(fileName);
    if (!fileCvs.open(QFile::ReadOnly)) {
        QMessageBox::critical(this, QObject::tr("Error in CSV import"),
                              QObject::tr("Unable to open CSV file!"));
        reject();
        return;
    }
    QTextStream in(&fileCvs);
    in.setCodec(ui->encodingCombo->itemData(
                    ui->encodingCombo->currentIndex())
                .toString().toStdString().c_str());

    //import
    QSqlDatabase db = DatabaseManager::getInstance().getDatabase();
    QSqlQuery query(db);

    //import header to generate columns
    QStringList headers = parseCSVLine(readCSVLine(&in, embeddedEscape),
                                       separator,
                                       embeddedEscape);
    if (headers.isEmpty()) {
        QMessageBox::critical(this, QObject::tr("Error in CSV import"),
                              QObject::tr("Invalid CSV file!"));
        fileCvs.close();
        reject();
        return;
    }

    //create new collection
    int collectionId = -1;
    QString collectionName = ui->collectionNameLineEdit->text();
    collectionName.replace('"', " "); //replace double quotes
    QString sql = QString("INSERT INTO \"collections\" (\"name\")"
                          " VALUES (\"%1\")").arg(collectionName);
    if (query.exec(sql))
        collectionId = m_metadataEngine->createNewCollection();
    else
        return;

    //set local data changed
    SyncSession::LOCAL_DATA_CHANGED = true;

    //create new fields from headers
    db.transaction();
    foreach (QString s, headers) {
        s.replace('"', " "); //replace double quotes
        m_metadataEngine->createField(s,
                                      MetadataEngine::TextType,
                                      "",
                                      "",
                                      "",
                                      collectionId);

        qApp->processEvents();
        if (m_importCancelled) {
            fileCvs.close();
            reject();
            return;
        }
    }
    QString tableName = m_metadataEngine->getTableName(collectionId);
    QString columnList;
    int headersSize = headers.size();
    for (int i = 0; i < headersSize; i++) {
        columnList.append(QString("\"%1\"").arg(i+1)); //+1 because of _id
        if ((i+1) < headersSize)
            columnList.append(",");
    }

    //import data
    while (!in.atEnd()) {
        QStringList fields = parseCSVLine(readCSVLine(&in, embeddedEscape),
                                          separator,
                                          embeddedEscape);
        int fieldsSize = fields.size();
        if (fieldsSize == headersSize) {
            QString valueList;
            for (int i = 0; i < fieldsSize; i++) {
                QString fieldValue = fields.at(i);
                fieldValue.replace('"', "\"\""); //replace double-quotes for sql (escape)
                valueList.append(QString("\"%1\"").arg(fieldValue));
                if ((i+1) < fieldsSize)
                    valueList.append(",");
            }
            sql = QString("INSERT INTO \"%1\" (%2) VALUES (%3)")
                    .arg(tableName).arg(columnList).arg(valueList);
            query.prepare(sql);
            query.exec();
        }

        qApp->processEvents();
        if (m_importCancelled) {
            fileCvs.close();
            reject();
            return;
        }
    }
    db.commit();

    fileCvs.close();
    accept();
}


//-----------------------------------------------------------------------------
// Private
//-----------------------------------------------------------------------------

void ImportDialog::initUiCSV()
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

QString ImportDialog::readCSVLine(QTextStream *in,
                                  const QChar &embeddedEscape)
{
    QString line = in->readLine();

    //if double-quote (ex. for embeddedEscape) count is odd
    //ignore line break, because of embeddedEscape (part of data)
    if (line.count(embeddedEscape) % 2) {
        do { //read next line until even again, meaning end of line
            line.append("\n"); //newline is part of data
            line.append(in->readLine());
        } while (line.count(embeddedEscape) % 2);
    }

    return line;
}

QStringList ImportDialog::parseCSVLine(const QString &csvData,
                                       const QChar &separator,
                                       const QChar &embeddedEscape)
{
    QStringList fields;

    QString tmpField;
    bool escapeNext = false;
    int size = csvData.size();
    QChar c;
    for (int i = 0; i < size; i++) {
        c = csvData.at(i);

        if (c == embeddedEscape) { // ex. double-quote found
            if ((i + 1) < size) { //if has next
                if (csvData.at(i+1) != embeddedEscape) { //single -> escape
                    escapeNext = !escapeNext; //invert escape status
                } else { //double -> double-quote escape, add it
                    tmpField.append(embeddedEscape);
                    i++; //increment since next was already parsed 2 * " = single "
                }
            }
        } else if (c == separator) {
            if (escapeNext) { //separator in a doube-quoted string, escape
                tmpField.append(separator);
            } else { //terminate field
                fields.append(tmpField);
                escapeNext = false;
                tmpField.clear();
            }
        } else { //alphanum -> add
            tmpField.append(c);
        }
    }
    //add last field when last field empty (last char separator)
    //or end of line without separator
    if ((c == separator) || (!tmpField.isEmpty())) {
        fields.append(tmpField);
    }

    return fields;
}
