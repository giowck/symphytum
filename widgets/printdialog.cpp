/*
 *  Copyright (c) 2012 Giorgio Wicklein <giowckln@gmail.com>
 */

//-----------------------------------------------------------------------------
// Hearders
//-----------------------------------------------------------------------------

#include "printdialog.h"
#include "ui_printdialog.h"

#include "../components/metadataengine.h"
#include "../components/databasemanager.h"
#include "../components/filemanager.h"
#include "../utils/metadatapropertiesparser.h"
#include "../utils/definitionholder.h"

#include <QtSql/QSqlQuery>
#include <QtWidgets/QFileDialog>
#include <QtGui/QDesktopServices>
#include <QPrintDialog>
#include <QPrinter>
#include <QtGui/QTextDocument>
#include <QtCore/QDateTime>


//-----------------------------------------------------------------------------
// Public
//-----------------------------------------------------------------------------

PrintDialog::PrintDialog(const int collectionId,
                         QList<int> &selectionRecordIdList,
                         QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PrintDialog),
    m_recordList(selectionRecordIdList),
    m_collectionId(collectionId),
    m_printCancelled(false)
{
    ui->setupUi(this);

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
    connect(ui->pdfButton, SIGNAL(clicked()),
            this, SLOT(pdfButtonClicked()));
    connect(ui->printButton, SIGNAL(clicked()),
            this, SLOT(printButtonClicked()));
    connect(ui->cancelPrintButton, SIGNAL(clicked()),
            this, SLOT(cancelPrintButtonClicked()));
}

PrintDialog::~PrintDialog()
{
    delete ui;
}


//-----------------------------------------------------------------------------
// Private slots
//-----------------------------------------------------------------------------

void PrintDialog::pdfButtonClicked()
{
    ui->stackedWidget->setCurrentIndex(1);
    qApp->processEvents();

    //get output file
    QString documentsDir = QStandardPaths::standardLocations(
                QStandardPaths::DocumentsLocation).at(0);
    QString fileName = QFileDialog::getSaveFileName(this,
                                                    tr("Save PDF file"),
                                                    documentsDir,
                                                    tr("PDF file(*.pdf)"));
    if (fileName.isEmpty()) {
        ui->stackedWidget->setCurrentIndex(0);
        return;
    }
    if (!fileName.contains(".pdf", Qt::CaseInsensitive))
        fileName.append(".pdf");

    initRecordListToPrint();
    print(true, fileName);
}

void PrintDialog::printButtonClicked()
{
    ui->stackedWidget->setCurrentIndex(1);
    qApp->processEvents();

    initRecordListToPrint();
    print();
}

void PrintDialog::cancelPrintButtonClicked()
{
    m_printCancelled = true;
}


//-----------------------------------------------------------------------------
// Private
//-----------------------------------------------------------------------------

void PrintDialog::initRecordListToPrint()
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

void PrintDialog::print(bool pdf, QString pdfOutputPath)
{
    QPrinter printer;
    QTextDocument document(this);
    QString htmlString;
    QString tableName = m_metadataEngine->getTableName(m_collectionId);
    QSqlDatabase db = DatabaseManager::getInstance().getDatabase();
    QSqlQuery query(db);

    if (pdf) {
        printer.setOutputFileName(pdfOutputPath);
        printer.setOutputFormat(QPrinter::PdfFormat);
    } else {
        QPrintDialog printDialog(&printer, this);
        if (printDialog.exec() != QDialog::Accepted) {
            ui->stackedWidget->setCurrentIndex(0);
            return;
        }
    }

    //init html
    htmlString.append(QString(
                          "<html>"
                          "<head>"
                          "<style type=\"text/css\">"
                          "table"
                          "{"
                          "border-collapse:collapse;"
                          "}"
                          "table, th, td"
                          "{"
                          "border: 1px solid #D3D3D3;"
                          "padding: 5px;"
                          "font-size: 11pt;"
                          "}"
                          "</style>"
                          "</head>"
                          "<body>"
                          "<p>Generated by %1 on %2</p><br />"
                          )
                      .arg(DefinitionHolder::NAME)
                      .arg(QDateTime::currentDateTime().toString()));

    //get all record to print
    QString recordListString;
    foreach (int id, m_recordList) {
        recordListString.append(QString::number(id) + ",");
    }
    //remove last ','
    recordListString.remove(recordListString.size()-1, 1);
    QString sql = QString("SELECT * FROM %1 WHERE _id IN (%2)")
            .arg(tableName).arg(recordListString);
    query.exec(sql);

    //generate text
    while (query.next()) {
        htmlString.append("<table>");

        int fieldCount = m_metadataEngine->getFieldCount(m_collectionId);
        for (int i = 1; i < fieldCount; i++) { // 1 because of _id
            htmlString.append("<tr>");
            htmlString.append(QString("<td>%1:</td>")
                              .arg(m_metadataEngine->getFieldName(i, m_collectionId)));
            htmlString.append("<td>");

            MetadataEngine::FieldType type = m_metadataEngine->getFieldType(i, m_collectionId);
            switch (type) {
            case MetadataEngine::TextType:
            case MetadataEngine::URLTextType:
            case MetadataEngine::EmailTextType:
                htmlString.append(textTypeItemHtml(query.value(i), i));
                break;
            case MetadataEngine::NumericType:
                htmlString.append(numericTypeItemHtml(query.value(i), i));
                break;
            case MetadataEngine::CreationDateType:
            case MetadataEngine::ModDateType:
            case MetadataEngine::DateType:
                htmlString.append(dateTypeItemHtml(query.value(i), i));
                break;
            case MetadataEngine::CheckboxType:
                htmlString.append(checkboxTypeItemHtml(query.value(i), i));
                break;
            case MetadataEngine::ComboboxType:
                htmlString.append(comboboxTypeItemHtml(query.value(i), i));
                break;
            case MetadataEngine::ProgressType:
                htmlString.append(progressTypeItemHtml(query.value(i), i));
                break;
            case MetadataEngine::ImageType:
                htmlString.append(imageTypeItemHtml(query.value(i), i));
                break;
            case MetadataEngine::FilesType:
                htmlString.append(filesTypeItemHtml(query.value(i), i));
                break;
            default:
                htmlString.append(textTypeItemHtml(query.value(i), i));
                break;
            }

            htmlString.append("</td>");
            htmlString.append("</tr>");
        }

        htmlString.append("</table><br /><hr />");

        ui->progressBar->setValue(ui->progressBar->value()+1);
        qApp->processEvents();
        if (m_printCancelled) {
            reject();
            return;
        }
    }

    //finalize html
    htmlString.append("</body></html>");

    //print
    document.setHtml(htmlString);
    document.print(&printer);

    accept(); //close
}

QString PrintDialog::textTypeItemHtml(const QVariant &data, int fieldId)
{
    Q_UNUSED(fieldId);

    return data.toString();
}

QString PrintDialog::numericTypeItemHtml(const QVariant &data, int fieldId)
{
    QString html;
    html.append("<span style=\"");

    QString dataString;
    bool empty = data.toString().isEmpty();

    //adapt to display properties
    QString metadata =
            m_metadataEngine->getFieldProperties(MetadataEngine::DisplayProperty,
                                                 fieldId);
    MetadataPropertiesParser parser(metadata);
    QString v;

    v = parser.getValue("markNegative");
    if ((v == "1") && (data.toDouble() < 0.0)) {
        html.append("color:red;");
    }

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

    html.append("\">");
    html.append(dataString);
    html.append("</span>");

    return html;
}

QString PrintDialog::dateTypeItemHtml(const QVariant &data, int fieldId)
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

QString PrintDialog::checkboxTypeItemHtml(const QVariant &data, int fieldId)
{
    Q_UNUSED(fieldId);

    QString html;
    bool checked = data.toInt();

    if(checked)
        html.append(tr("Yes"));
    else
        html.append(tr("No"));

    return html;
}

QString PrintDialog::comboboxTypeItemHtml(const QVariant &data, int fieldId)
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
    }

    return itemString;
}

QString PrintDialog::progressTypeItemHtml(const QVariant &data, int fieldId)
{
    QString html;

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
    html.append(QString::number(percentage) + "%");

    return html;
}

QString PrintDialog::imageTypeItemHtml(const QVariant &data, int fieldId)
{
    Q_UNUSED(fieldId);

    QString html;
    int fileId = data.toInt();
    if (fileId) {
        QString filePath;
        QString fileName;
        QString fileHash;
        QDateTime addedDateTime;
        FileManager fm;

        m_metadataEngine->getContentFile(fileId,
                                         fileName,
                                         fileHash,
                                         addedDateTime);

        filePath = fm.getFilesDirectory() + fileHash;

        //calc size
        QPixmap pixmap(filePath);
        QSize pixmapSize = pixmap.size();
        QSize newSize(256, 256);
        QRect imageRect = pixmap.scaled(QSize(256, 256),
                                        Qt::KeepAspectRatio,
                                        Qt::FastTransformation).rect();
        if ((newSize.width() > pixmapSize.width()) &&
                (newSize.height() > pixmapSize.height())) {
            newSize = pixmapSize;
        } else {
            newSize = QSize(imageRect.width(), imageRect.height());
        }

        html.append(QString("<img border=\"0\" src=\"%1\" width=\"%2\" height=\"%3\">")
                    .arg(filePath).arg(newSize.width()).arg(newSize.height()));
    }

    return html;
}

QString PrintDialog::filesTypeItemHtml(const QVariant &data, int fieldId)
{
    Q_UNUSED(fieldId);

    QString dataString = data.toString();

    int fileCount = dataString.split(',', QString::SkipEmptyParts).size();
    //opt.text = tr("%n file(s)", "", fileCount);
    //BUG workaround: investigate why it doesn't work (http://qt-project.org/doc/qt-4.8/i18n-source-translation.html#handling-plurals)
    QString countString = (fileCount == 1 )? tr("%1 file").arg(fileCount) :
                                             tr("%1 files").arg(fileCount);

    return countString;
}
