/*
 *  Copyright (c) 2012 Giorgio Wicklein <giowckln@gmail.com>
 *  Copyright (c) 2014 Giorgio Wicklein <giowckln@gmail.com>
 */

//-----------------------------------------------------------------------------
// Hearders
//-----------------------------------------------------------------------------

#include "tableviewdelegate.h"
#include "editors/imagetypeeditor.h"
#include "editors/filestypeeditor.h"
#include "../../utils/formwidgetvalidator.h"
#include "../../components/metadataengine.h"
#include "../../utils/metadatapropertiesparser.h"
#include "../../widgets/mainwindow.h"
#include "../../components/undocommands.h"
#include "../../components/sync_framework/syncsession.h"
#include "../../components/filemanager.h"
#include "tableview.h"
#include "../../components/alarmmanager.h"
#include "../../components/settingsmanager.h"

#include <QtWidgets/QLineEdit>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QComboBox>
#include <QtCore/QLocale>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QUndoCommand>
#include <QtWidgets/QUndoStack>
#include <QtWidgets/QApplication>
#include <QtGui/QPixmap>
#include <QtGui/QPainter>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QDateTimeEdit>
#include <QtGui/QPixmapCache>


//-----------------------------------------------------------------------------
// Public
//-----------------------------------------------------------------------------

TableViewDelegate::TableViewDelegate(QObject *parent) :
    QStyledItemDelegate(parent), m_metadataEngine(nullptr)
{
    m_metadataEngine = &MetadataEngine::getInstance();

    //check if cache images enabled
    SettingsManager sm;
    m_cacheImages = sm.restoreProperty("cacheImages", "tableView").toBool();
    if (m_cacheImages) {
        QPixmapCache::setCacheLimit(10240 * 200); //200 * 10MB ~ 2GB RAM needed
    }

    //load property: images should be hidden for better scrolling performance on weak devices
    m_hideImages = sm.restoreProperty("hideImages", "tableView").toBool();
}

void TableViewDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
                              const QModelIndex &index) const
{
    MetadataEngine::FieldType type = m_metadataEngine->getFieldType(index.column());

    switch (type) {
    case MetadataEngine::TextType:
    case MetadataEngine::URLTextType:
    case MetadataEngine::EmailTextType:
        paintTextType(painter, option, index);
        break;
    case MetadataEngine::NumericType:
        paintNumericType(painter, option, index);
        break;
    case MetadataEngine::CreationDateType:
    case MetadataEngine::ModDateType:
    case MetadataEngine::DateType:
        paintCreateDateType(painter, option, index);
        break;
    case MetadataEngine::CheckboxType:
        paintCheckboxType(painter, option, index);
        break;
    case MetadataEngine::ComboboxType:
        paintComboboxType(painter, option, index);
        break;
    case MetadataEngine::ProgressType:
        paintProgressType(painter, option, index);
        break;
    case MetadataEngine::ImageType:
        paintImageType(painter, option, index);
        break;
    case MetadataEngine::FilesType:
        paintFilesType(painter, option, index);
        break;
    default:
        QStyledItemDelegate::paint(painter, option, index);
        break;
    }
}

QSize TableViewDelegate::sizeHint(const QStyleOptionViewItem &option,
                                  const QModelIndex &index) const
{
    if (false /* if custom data type */) {
        //see example http://qt-project.org/doc/qt-4.8/itemviews-stardelegate.html
    } else {
        return QStyledItemDelegate::sizeHint(option, index);
    }
}

QWidget* TableViewDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                                         const QModelIndex &index) const
{
    int column = index.column();
    MetadataEngine::FieldType fieldType =  m_metadataEngine->getFieldType(column);
    QWidget *e;

    switch (fieldType) {
    case MetadataEngine::TextType:
    case MetadataEngine::URLTextType:
    case MetadataEngine::EmailTextType:
        e = new QLineEdit(parent);
        break;
    case MetadataEngine::NumericType:
    {
        QLineEdit *lineEdit;
        lineEdit = new QLineEdit(parent);
        //accept only double values, support locale dependent decimal point
        lineEdit->setValidator(new QDoubleValidator(parent));
        e = lineEdit;
    }
        break;
    case MetadataEngine::CheckboxType:
    {
        //on OS X checkbox editors are not clickable (only with spacebar)
        //this is a bug, so here we use QComboBox instead of QCheckbox
#ifdef Q_OS_OSX
        QComboBox *c = new QComboBox(parent);
        c->addItem(tr("No"));
        c->addItem(tr("Yes"));
        e = c;
#else
        e = new QCheckBox(parent);

#endif // Q_OS_OSX
    }
        break;
    case MetadataEngine::ComboboxType:
    {
        QComboBox *c = new QComboBox(parent);

        //load items from display properties
        MetadataEngine *meta = m_metadataEngine;
        QString displayProperties = meta->getFieldProperties(meta->DisplayProperty,
                                                             index.column(),
                                                             meta->getCurrentCollectionId());
        MetadataPropertiesParser displayParser(displayProperties);
        if (displayProperties.size()) {
            QStringList items = displayParser.getValue("items")
                    .split(',', QString::SkipEmptyParts);
            foreach (QString s, items) {
                //replace some escape codes
                s.replace("\\comma", ",");
                s.replace("\\colon", ":");
                s.replace("\\semicolon", ";");
                s.replace("\\doublequote", "\"");
                s.replace("\\singlequote", "'");
                c->addItem(s);
            }
        }

        e = c;
    }
        break;
    case MetadataEngine::ProgressType:
    {
        QSpinBox *c = new QSpinBox(parent);

        //load items from display properties
        MetadataEngine *meta = m_metadataEngine;
        QString displayProperties = meta->getFieldProperties(meta->DisplayProperty,
                                                             index.column(),
                                                             meta->getCurrentCollectionId());
        MetadataPropertiesParser displayParser(displayProperties);
        if (displayProperties.size()) {
            bool ok;
            int max = displayParser.getValue("max").toInt(&ok);
            if (!ok) max = 100;
            c->setButtonSymbols(QAbstractSpinBox::PlusMinus);
            c->setRange(0, max);
            c->setSuffix(tr(" of %1").arg(max));
        }

        e = c;
    }
        break;
    case MetadataEngine::ImageType:
    {
        ImageTypeEditor *i = new ImageTypeEditor(parent);
        connect(i, SIGNAL(editingFinished()),
                this, SLOT(commitAndCloseCustomEditor()));
        e = i;
    }
        break;
    case MetadataEngine::FilesType:
    {
        FilesTypeEditor *f = new FilesTypeEditor(parent);
        connect(f, SIGNAL(editingFinished()),
                this, SLOT(commitAndCloseCustomEditor()));
        e = f;
    }
        break;
    case MetadataEngine::DateType:
    {
        QDateTimeEdit *t = new QDateTimeEdit(parent);
        t->setMinimumDate(QDate(100, 1, 1));

        //load date format from display properties
        QLocale locale;
        QString dateFormat = locale.dateTimeFormat(QLocale::ShortFormat);
        MetadataEngine *meta = m_metadataEngine;
        QString displayProperties = meta->getFieldProperties(meta->DisplayProperty,
                                                             index.column(),
                                                             meta->getCurrentCollectionId());
        MetadataPropertiesParser parser(displayProperties);
        if (displayProperties.size()) {
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

        //setup date time edit
        t->setCalendarPopup(true);
        t->setDisplayFormat(dateFormat);
        t->setDateTime(QDateTime::currentDateTime());

        e = t;
    }
        break;
    case MetadataEngine::CreationDateType:
    case MetadataEngine::ModDateType:
        //editing not supported
        e = new QWidget(parent);
        break;
    default:
        e = QStyledItemDelegate::createEditor(parent, option, index);
    }

    return e;
}

void TableViewDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    int column = index.column();
    MetadataEngine::FieldType fieldType =  m_metadataEngine->getFieldType(column);

    switch (fieldType) {
    case MetadataEngine::TextType:
    case MetadataEngine::URLTextType:
    case MetadataEngine::EmailTextType:
        setTextTypeEditorData(editor, index);
        break;
    case MetadataEngine::NumericType:
        setNumericTypeEditorData(editor, index);
        break;
    case MetadataEngine::CheckboxType:
        setCheckboxTypeEditorData(editor, index);
        break;
    case MetadataEngine::ComboboxType:
        setComboboxTypeEditorData(editor, index);
        break;
    case MetadataEngine::ProgressType:
        setProgressTypeEditorData(editor, index);
        break;
    case MetadataEngine::ImageType:
        setImageTypeEditorData(editor, index);
        break;
    case MetadataEngine::FilesType:
        setFilesTypeEditorData(editor, index);
        break;
    case MetadataEngine::DateType:
        setDateTypeEditorData(editor, index);
        break;
    case MetadataEngine::CreationDateType:
    case MetadataEngine::ModDateType:
        //editing not supported
        break;
    default:
        QStyledItemDelegate::setEditorData(editor, index);
    }
}

void TableViewDelegate::updateEditorGeometry(QWidget *editor,
                                             const QStyleOptionViewItem &option,
                                             const QModelIndex &index) const
{
    int column = index.column();
    MetadataEngine::FieldType fieldType =  m_metadataEngine->getFieldType(column);

    switch (fieldType) {
    case MetadataEngine::TextType:
    case MetadataEngine::NumericType:
    case MetadataEngine::DateType:
    case MetadataEngine::CreationDateType:
    case MetadataEngine::ModDateType:
    case MetadataEngine::ComboboxType:
    case MetadataEngine::ProgressType:
    case MetadataEngine::ImageType:
    case MetadataEngine::FilesType:
    case MetadataEngine::URLTextType:
    case MetadataEngine::EmailTextType:
        editor->setGeometry(option.rect);
        break;
    case MetadataEngine::CheckboxType:
    {
        //because of the bug on Qt 5.6 for OS X where the checkbox (display) in tableview
        //is drawn at the wrong position here we use QComboBox instead of QCheckbox
        //to match better the simple text display instead of the checkbox display
        //see paintCheckboxType()
#ifdef Q_OS_OSX
        editor->setGeometry(option.rect);
#else
        //center checkbox
        QStyleOptionButton checkboxstyle;
        QRect checkbox_rect = QApplication::style()->subElementRect(QStyle::SE_CheckBoxIndicator, &checkboxstyle);
        checkboxstyle.rect = option.rect;
        checkboxstyle.rect.setLeft(option.rect.x() +
                                   option.rect.width()/2 - checkbox_rect.width()/2);
        editor->setGeometry(checkboxstyle.rect);
#endif // Q_OS_OSX
    }
        break;
    default:
        QStyledItemDelegate::updateEditorGeometry(editor, option, index);
    }
}

void TableViewDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
                                     const QModelIndex &index) const
{
    int column = index.column();
    MetadataEngine::FieldType fieldType =
            m_metadataEngine->getFieldType(column);
    QString metadataEdit =
            m_metadataEngine->getFieldProperties(MetadataEngine::EditProperty,
                                                 column);
    FormWidgetValidator validator(metadataEdit, fieldType);
    QVariant data;

    //get data from editor
    switch (fieldType) {
    case MetadataEngine::NumericType:
    {
        QLineEdit *l;

        //localization of decimal point
        QChar notDecimal;
        QLocale locale;

        l = qobject_cast<QLineEdit*>(editor);
        if (l) {
            //check if text data is empty
            if (l->text().isEmpty()) break;

            //determine which of '.' or ',' is decimal point
            if (locale.decimalPoint() == '.')
                notDecimal = ',';
            else
                notDecimal = '.';
            //create data depending on locale for decimal point
            data = locale.toDouble(l->text().remove(notDecimal));
        }
    }
        break;
    case MetadataEngine::TextType:
    case MetadataEngine::URLTextType:
    case MetadataEngine::EmailTextType:
    {
        QLineEdit *l;
        l = qobject_cast<QLineEdit*>(editor);
        if (l)
            data = l->text();
    }
        break;
    case MetadataEngine::CheckboxType:
    {
        //because of the bug on Qt 5.6 for OS X where the checkbox (display) in tableview
        //is drawn at the wrong position here we use QComboBox instead of QCheckbox
        //to match better the simple text display instead of the checkbox display
        //see paintCheckboxType()
#ifdef Q_OS_OSX
        QComboBox *c;
        c = qobject_cast<QComboBox*>(editor);
        if (c) {
            if (c->currentIndex())
                data = 1; //no/unchecked
        }
            else
                data = 0; //yes//checked
#else
        QCheckBox *c;
        c = qobject_cast<QCheckBox*>(editor);
        if (c)
            data = ((int) c->isChecked());
#endif // Q_OS_OSX
    }
        break;
    case MetadataEngine::ComboboxType:
    {
        QComboBox *c;
        c = qobject_cast<QComboBox*>(editor);
        if (c)
            data = c->currentIndex();
    }
        break;
    case MetadataEngine::ProgressType:
    {
        QSpinBox *c;
        c = qobject_cast<QSpinBox*>(editor);
        if (c)
            data = c->value();
    }
        break;
    case MetadataEngine::ImageType:
    {
        ImageTypeEditor *e;
        e = qobject_cast<ImageTypeEditor*>(editor);
        if (e)
            data = e->getImage();
    }
        break;
    case MetadataEngine::FilesType:
    {
        FilesTypeEditor *e;
        e = qobject_cast<FilesTypeEditor*>(editor);
        if (e)
            data = e->getFiles();
    }
        break;
    case MetadataEngine::DateType:
    {
        QDateTimeEdit *t;
        t = qobject_cast<QDateTimeEdit*>(editor);
        if (t)
            data = t->dateTime();

        //get current edit record and field ids
        int recordId = -1;
        int fieldId = -1;
        TableView *view = qobject_cast<TableView*>(this->parent());
        if (view) {
            int row = view->getLastEditRow();
            fieldId = view->getLastEditColumn();
            if (row != -1) {
                bool ok;
                int i = view->model()->data(view->model()->index(row, 0)).toInt(&ok);
                if (ok)
                    recordId = i;
            }
        }

        //check if alarm trigger property is enabled
        //if it is, update alarm trigger in alarms table
        if (fieldId != -1) {
            MetadataPropertiesParser parser(m_metadataEngine->
                                            getFieldProperties(m_metadataEngine->TriggerProperty,
                                                               fieldId));
            if (parser.size() > 0) {
                bool alarmTrigger = parser.getValue("alarmOnDate") == "1";

                //add or update alarm
                if ((recordId != -1) && (alarmTrigger)) {
                    AlarmManager a;
                    QDateTime dateTime = data.toDateTime();
                    a.addOrUpdateAlarm(m_metadataEngine->getCurrentCollectionId(),
                                       fieldId, recordId, dateTime);
                }
            }
        }
    }
        break;
    case MetadataEngine::CreationDateType:
    case MetadataEngine::ModDateType:
        //editing not supported
        return;
    }

    //validate
    QString errorMessage;
    bool valid = validator.validate(data, errorMessage);

    if (valid) {
        //create undo action
        QVariant oldData = index.data();
        QVariant newData = data;
        if (oldData != newData) {
            QUndoStack *stack = MainWindow::getUndoStack();
            if (stack) {
                ModRecordCommand *cmd = new ModRecordCommand(
                            m_metadataEngine->getCurrentCollectionId(),
                            index.row(), index.column(),
                            oldData, newData);
                stack->push(cmd);
            }

            //sync set local data changed
            SyncSession::LOCAL_DATA_CHANGED = true;
        }

        model->setData(index, data);
    } else {
        //on invalid input show message
        QWidget *parent = qobject_cast<QWidget*>(this->parent());
        QMessageBox box(QMessageBox::Warning, tr("Invalid Input"),
                        tr("The entered data is not valid!<br>"
                           "%1").arg(errorMessage),
                        QMessageBox::NoButton,
                        parent);
        box.setWindowModality(Qt::WindowModal);
        box.exec();
    }
}


//-----------------------------------------------------------------------------
// Private slots
//-----------------------------------------------------------------------------

void TableViewDelegate::commitAndCloseCustomEditor()
{
    QWidget *editor = qobject_cast<QWidget *>(sender());
    if (editor) {
        emit commitData(editor);
        emit closeEditor(editor);
    }
}


//-----------------------------------------------------------------------------
// Private
//-----------------------------------------------------------------------------

void TableViewDelegate::paintTextType(QPainter *painter,
                                      const QStyleOptionViewItem &option,
                                      const QModelIndex &index) const
{
    QString dataString = index.data().toString();
    QStyleOptionViewItem opt(option);
    opt.text = dataString;

    //adapt to display properties
    QString metadata =
            m_metadataEngine->getFieldProperties(MetadataEngine::DisplayProperty,
                                                 index.column());
    MetadataPropertiesParser parser(metadata);
    if (metadata.size() > 0) {
        QString v;

        v = parser.getValue("markEmpty");
        if ((v == "1") && dataString.isEmpty()) {
            //make background red style
            opt.backgroundBrush.setStyle(Qt::SolidPattern);
            opt.backgroundBrush.setColor(QColor(255,223,223));
        }
    }

    opt.widget->style()->drawControl(QStyle::CE_ItemViewItem, &opt, painter);
}

void TableViewDelegate::paintNumericType(QPainter *painter,
                                         const QStyleOptionViewItem &option,
                                         const QModelIndex &index) const
{
    QStyleOptionViewItem opt(option);
    QString dataString;
    bool empty = index.data().toString().isEmpty();

    //adapt to display properties
    QString metadata =
            m_metadataEngine->getFieldProperties(MetadataEngine::DisplayProperty,
                                                 index.column());
    MetadataPropertiesParser parser(metadata);
    QString v;

    v = parser.getValue("markEmpty");
    if ((v == "1") && empty) {
        //make background red style
        opt.backgroundBrush.setStyle(Qt::SolidPattern);
        opt.backgroundBrush.setColor(QColor(255,223,223));
    }

    v = parser.getValue("markNegative");
    if ((v == "1") && (index.data().toDouble() < 0.0)) {
        opt.palette.setColor(QPalette::Text, Qt::red);
    }

    int precision;
    v = parser.getValue("precision");
    precision = v.toInt();

    v = parser.getValue("displayMode");
    if (v == "auto") {
        if (!empty)
            dataString = index.data().toString();
    } else if (v == "decimal") {
        if (!empty)
            dataString = QString::number(index.data().toDouble(), 'f', precision);
    } else if (v == "scientific") {
        if (!empty)
            dataString = QString::number(index.data().toDouble(), 'e', precision);
    } else {
        if (!empty)
            dataString = index.data().toString(); //if display mode not specified
    }

    //make use of the correct decimal point char
    QLocale locale;
    dataString.replace(".", locale.decimalPoint());

    opt.text = dataString;
    opt.widget->style()->drawControl(QStyle::CE_ItemViewItem, &opt, painter);
}

void TableViewDelegate::paintCreateDateType(QPainter *painter,
                                            const QStyleOptionViewItem &option,
                                            const QModelIndex &index) const
{
    QStyleOptionViewItem opt(option);
    QLocale locale;
    QString dateFormat = locale.dateTimeFormat(QLocale::ShortFormat);

    //adapt to display properties
    QString metadata =
            m_metadataEngine->getFieldProperties(MetadataEngine::DisplayProperty,
                                                 index.column());
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

    //date as string
    opt.text = index.data().toDateTime().toString(dateFormat);

    opt.widget->style()->drawControl(QStyle::CE_ItemViewItem, &opt, painter);
}

void TableViewDelegate::paintCheckboxType(QPainter *painter,
                                      const QStyleOptionViewItem &option,
                                      const QModelIndex &index) const
{
    bool checked = index.model()->data(index).toInt();

#ifdef Q_OS_OSX
    //there is a bug in Qt 5.6 for OS X where the checkbox position is wrong
    //so instead on OS X use a simple combobox yes/no display as a workaround
    QString dataString = checked ? tr("Yes") : tr("No");
    QStyleOptionViewItem opt(option);
    opt.text = dataString;
    opt.widget->style()->drawControl(QStyle::CE_ItemViewItem, &opt, painter);
#else
    QStyleOptionButton checkboxstyle;
    //center checkbox
    QRect checkbox_rect = QApplication::style()->subElementRect(QStyle::SE_CheckBoxIndicator,
                                                                &checkboxstyle);
    checkboxstyle.rect = option.rect;
    checkboxstyle.rect.setLeft(option.rect.x() +
                               option.rect.width()/2 - checkbox_rect.width()/2);
    if(checked)
        checkboxstyle.state = QStyle::State_On|QStyle::State_Enabled;
    else
        checkboxstyle.state = QStyle::State_Off|QStyle::State_Enabled;

    QApplication::style()->drawControl(QStyle::CE_CheckBox, &checkboxstyle, painter);
#endif //Q_OS_OSX
}

void TableViewDelegate::paintComboboxType(QPainter *painter,
                                          const QStyleOptionViewItem &option,
                                          const QModelIndex &index) const
{
    bool ok;
    int itemId = index.data().toInt(&ok);
    if (!ok) itemId = -1;
    QString itemString;

    QStyleOptionViewItem opt(option);

    //adapt to display properties
    QString metadata =
            m_metadataEngine->getFieldProperties(MetadataEngine::DisplayProperty,
                                                 index.column());
    MetadataPropertiesParser parser(metadata);
    if (metadata.size() > 0) {
        QString v;

        v = parser.getValue("markEmpty");
        if ((v == "1") && (itemId == -1)) {
            //make background red style
            opt.backgroundBrush.setStyle(Qt::SolidPattern);
            opt.backgroundBrush.setColor(QColor(255,223,223));
        }

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

    opt.text = itemString;

    opt.widget->style()->drawControl(QStyle::CE_ItemViewItem, &opt, painter);
}

void TableViewDelegate::paintProgressType(QPainter *painter,
                                          const QStyleOptionViewItem &option,
                                          const QModelIndex &index) const
{
    int max = 100;
    int value = index.data().toInt();

    //display properties
    QString metadata =
            m_metadataEngine->getFieldProperties(MetadataEngine::DisplayProperty,
                                                 index.column());
    MetadataPropertiesParser parser(metadata);
    if (metadata.size() > 0) {
        max = parser.getValue("max").toInt();
    }

    QStyleOptionProgressBar progressBarOption;

    //center progress bar
    //use checkbox height because progress indicator returns 0
    int progressBarHeight = QApplication::style()->subElementRect(QStyle::SE_CheckBoxIndicator,
                                                                  &progressBarOption).height();
    progressBarOption.rect = option.rect;
    progressBarOption.rect.setTop(option.rect.bottom() -
                                  option.rect.height()/2 - progressBarHeight/2);
    progressBarOption.rect.setHeight(progressBarHeight);

    progressBarOption.state = QStyle::State_Enabled;
    progressBarOption.direction = QApplication::layoutDirection();
    progressBarOption.fontMetrics = QApplication::fontMetrics();
    progressBarOption.minimum = 0;
    progressBarOption.maximum = max;
    progressBarOption.textAlignment = Qt::AlignCenter;
    progressBarOption.textVisible = true;
    progressBarOption.progress = value;
    int progressPercent = ((double) value) / ((double) max) * 100.0;
    progressBarOption.text = QString().sprintf("%d%%", progressPercent);

    QApplication::style()->drawControl(QStyle::CE_ProgressBar, &progressBarOption, painter);
}

void TableViewDelegate::paintImageType(QPainter *painter,
                                       const QStyleOptionViewItem &option,
                                       const QModelIndex &index) const
{
    int fileId = index.data().toInt();
    if (!fileId) return;

    //check if images should be hidden for better scrolling performance
    if (m_hideImages) { //show hidden as standard text
        QStyleOptionViewItem opt(option);
        opt.text = tr("hidden");
        opt.font.setStyle(QFont::StyleItalic);
        opt.widget->style()->drawControl(QStyle::CE_ItemViewItem, &opt, painter);
    } else { // show image
        QStyleOptionViewItem opt(option);

        QString filePath;
        QString fileName;
        QString fileHash;
        QString origDirPath;
        QDateTime addedDateTime;
        FileManager fm;

        m_metadataEngine->getContentFile(fileId,
                                         fileName,
                                         fileHash,
                                         addedDateTime,
                                         origDirPath);

        //if file was not found
        if (fileHash.isEmpty()) return;

        filePath = fm.getFilesDirectory() + fileHash;
        QPixmap pixmap;

        //use caching if enabled
        if (m_cacheImages) {
            if (!QPixmapCache::find(fileHash, &pixmap)) {
                pixmap.load(filePath);
                pixmap = pixmap.scaled(QSize(opt.rect.width(),
                                             opt.rect.height()),
                                       Qt::KeepAspectRatio,
                                       Qt::FastTransformation);
                QPixmapCache::insert(fileHash, pixmap);
            }
        } else {
            pixmap.load(filePath);
        }

        QRect imageRect = pixmap.scaled(QSize(opt.rect.width(),
                                              opt.rect.height()),
                                        Qt::KeepAspectRatio,
                                        Qt::FastTransformation).rect();
        QRect drawRect(opt.rect);

        //center
        drawRect.moveLeft(opt.rect.center().x() - (imageRect.width() / 2));
        drawRect.moveTop(opt.rect.center().y() - (imageRect.height() / 2));

        drawRect.setWidth(imageRect.width());
        drawRect.setHeight(imageRect.height());
        painter->drawPixmap(drawRect, pixmap);
    }
}

void TableViewDelegate::paintFilesType(QPainter *painter,
                                       const QStyleOptionViewItem &option,
                                       const QModelIndex &index) const
{
    QString dataString = index.data().toString();
    QStyleOptionViewItem opt(option);

    int fileCount = dataString.split(',', QString::SkipEmptyParts).size();
    //opt.text = tr("%n file(s)", "", fileCount);
    //BUG workaround: investigate why it doesn't work (http://qt-project.org/doc/qt-4.8/i18n-source-translation.html#handling-plurals)
    opt.text = (fileCount == 1 )? tr("%1 file").arg(fileCount) :
                                  tr("%1 files").arg(fileCount);

    opt.widget->style()->drawControl(QStyle::CE_ItemViewItem, &opt, painter);
}

void TableViewDelegate::setTextTypeEditorData(QWidget *editor,
                                              const QModelIndex &index) const
{
    QLineEdit *lineEdit;

    lineEdit = qobject_cast<QLineEdit*>(editor);
    if (lineEdit) lineEdit->setText(index.data().toString());
}

void TableViewDelegate::setNumericTypeEditorData(QWidget *editor,
                                                 const QModelIndex &index) const
{
    QString dataString;
    bool empty = index.data().toString().isEmpty();
    QLineEdit *lineEdit;

    //adapt to display properties
    QString metadata =
            m_metadataEngine->getFieldProperties(MetadataEngine::DisplayProperty,
                                                 index.column());
    MetadataPropertiesParser parser(metadata);
    QString v;

    int precision;
    v = parser.getValue("precision");
    precision = v.toInt();

    v = parser.getValue("displayMode");
    if (v == "auto") {
        if (!empty)
            dataString = index.data().toString();
    } else if (v == "decimal") {
        if (!empty)
            dataString = QString::number(index.data().toDouble(), 'f', precision);
    } else if (v == "scientific") {
        if (!empty)
            dataString = QString::number(index.data().toDouble(), 'e', precision);
    } else {
        if (!empty)
            dataString = index.data().toString(); //if display mode not specified
    }

    //make use of the correct decimal point char
    QLocale locale;
    dataString.replace(".", locale.decimalPoint());

    lineEdit = qobject_cast<QLineEdit*>(editor);
    if (lineEdit) lineEdit->setText(dataString);
}

void TableViewDelegate::setCheckboxTypeEditorData(QWidget *editor,
                                                  const QModelIndex &index) const
{
    //because of the bug on Qt 5.6 for OS X where the checkbox (display) in tableview
    //is drawn at the wrong position here we use QComboBox instead of QCheckbox
    //to match better the simple text display instead of the checkbox display
    //see paintCheckboxType()
#ifdef Q_OS_OSX
    QComboBox *comboBox;
    comboBox = qobject_cast<QComboBox*>(editor);
    if (comboBox) {
        if (index.data().toInt())
            comboBox->setCurrentIndex(1);
        else
            comboBox->setCurrentIndex(0);
    }
#else
    QCheckBox *checkBox;

    checkBox = qobject_cast<QCheckBox*>(editor);
    if (checkBox) checkBox->setChecked(index.data().toInt());
#endif // Q_OS_OSX
}

void TableViewDelegate::setComboboxTypeEditorData(QWidget *editor,
                                                  const QModelIndex &index) const
{
    QComboBox *comboBox;
    comboBox = qobject_cast<QComboBox*>(editor);
    if (comboBox) {
        bool ok;
        int value = index.data().toInt(&ok);
        if (!ok) value = -1;

        //handle default
        {
            QString metadata =
                    m_metadataEngine->getFieldProperties(MetadataEngine::DisplayProperty,
                                                         index.column());
            MetadataPropertiesParser parser(metadata);
            if (metadata.size() > 0) {
                QString v = parser.getValue("default");
                if ((!v.isEmpty()) && (value == -1)) {
                    bool ok;
                    value = v.toInt(&ok);
                    if (!ok) value = -1;
                }
            }
        }

        comboBox->setCurrentIndex(value);
    }
}

void TableViewDelegate::setProgressTypeEditorData(QWidget *editor,
                                                  const QModelIndex &index) const
{
    QSpinBox *spinBox;

    spinBox = qobject_cast<QSpinBox*>(editor);
    if (spinBox) spinBox->setValue(index.data().toInt());
}

void TableViewDelegate::setImageTypeEditorData(QWidget *editor,
                                               const QModelIndex &index) const
{
    ImageTypeEditor *e;

    e = qobject_cast<ImageTypeEditor*>(editor);
    if (e) e->setImage(index.data().toInt());
}

void TableViewDelegate::setFilesTypeEditorData(QWidget *editor,
                                               const QModelIndex &index) const
{
    FilesTypeEditor *e;

    e = qobject_cast<FilesTypeEditor*>(editor);
    if (e) e->setFiles(index.data().toString());
}

void TableViewDelegate::setDateTypeEditorData(QWidget *editor,
                                              const QModelIndex &index) const
{
    QDateTimeEdit *dateTimeEdit;

    dateTimeEdit = qobject_cast<QDateTimeEdit*>(editor);
    if (dateTimeEdit) dateTimeEdit->setDateTime(index.data().toDateTime());
}
