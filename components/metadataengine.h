/**
  * \class MetadataEngine
  * \brief This class is used to access/modify all metadata-related info.
  *        The content data is separated from metadata. Metadata has information
  *        about column names, collection names, data types and more.
  *        Another method exposed by this class is a factory method for
  *        content data models in accordance with a collection type.
  * \author Giorgio Wicklein - GIOWISYS Software
  * \date 02/06/2012
  */

#ifndef METADATAENGINE_H
#define METADATAENGINE_H


//-----------------------------------------------------------------------------
// Headers
//-----------------------------------------------------------------------------

#include <QtCore/QObject>
#include <QtCore/QDateTime>
#include <QtCore/QHash>


//-----------------------------------------------------------------------------
// Forward declarations
//-----------------------------------------------------------------------------

class QAbstractItemModel;


//-----------------------------------------------------------------------------
// MetadataEngine
//-----------------------------------------------------------------------------

class MetadataEngine : public QObject
{
    Q_OBJECT

public:
    /** This enum holds all supported collection types */
    enum CollectionType {
        StandardCollection = 1 /**< This is the standard data model which
                                    uses the default SQLite storage backend */
    };

    /** This enum presents all supported field (column) data types */
    enum FieldType {
        TextType = 1,       /**< Text type */
        NumericType,        /**< Number type */
        DateType,           /**< Date type */
        CreationDateType,   /**< Creation date type */
        ModDateType,        /**< Modification date type */
        CheckboxType,       /**< Checkbox type */
        ComboboxType,       /**< Combobox (drop-down list) type */
        ProgressType,       /**< Progress type */
        ImageType,          /**< Image type */
        FilesType,          /**< File list type */
        URLTextType,        /**< URL text type */
        EmailTextType       /**< Email text type */
    };

    /**
     * These are metadata classes for field properties.
     * See /stuff/doc/Standard Database Design.
     */
    enum FieldProperty {
        DisplayProperty, /**< Properties that describe how data
                              should be presented */
        EditProperty, /**< Properties that restrict the range of
                           valid values when editing */
        TriggerProperty /**< Properties that need action when a specific
                            condition is met */
    };

    static MetadataEngine& getInstance();
    static void destroy();

    /** Return collection name of specified id */
    QString getCollectionName(const int collectionId) const;

    /** Return current/last used collection id */
    int getCurrentCollectionId();

    /** Set the current active collection to the specified it */
    void setCurrentCollectionId(const int id);

    /** Return current/last used collection name */
    QString getCurrentCollectionName() const;

    /** Returns all collections as list of collection ids */
    QStringList getAllCollections(); //TODO: missing test case

    /** Get the table name for the specified collection id */
    QString getTableName(const int collectionId) const;

    /**
     * Get current field name (column name) for the specified column
     * and collection. If the collectionId is not specified the current
     * active one will be used.
     * @param column - the column/section number
     * @param collectionId - the id of the collection
     * @return column name
     */
    QString getFieldName(const int column,
                         int collectionId = m_currentCollectionId) const;

    /**
     * Set the field/column/section name for the specified column number.
     * This will update the metadata information about column names.
     * If the collection id is not specified the current active one will be used.
     * @param column - the column/section number
     * @param name - the new name for the field
     * @param collectionId - id of the collection
     */
    void setFieldName(const int column, const QString &name,
                      int collectionId = m_currentCollectionId);

    /** Get the column/field count (including _id) of the specified colledtion id */
    int getFieldCount(int collectionId = m_currentCollectionId) const;

    /**
     * Get the field type of a column (field)
     * @param column - the column number
     * @param collectionId - the collection id, if not specified active one is used
     * @return the field type as enum value
     */
    FieldType getFieldType(int column,
                           int collectionId = m_currentCollectionId) const;

    /**
     * Get the form layout coordinates for the specified field.
     * Each field has a coordinate for its position in FormView,
     * xpos is the column and ypos the row in a form layout matrix.
     * @param column - the field/column id of which the coordinates are needed
     * @param collectionId - the collection id, default is current one
     * @param xpos - a reference to a int where the xpos coordinate is saved
     * @param ypos - a reference to a int where the ypos coordinate is saved
     * @return whether the specified column has a valid coordinate. If a field
     *         has not been moved, then the metadata for the coordinates is
     *         empty. FormView will use then a best fit location...
     */
    bool getFieldCoordinate(const int column,
                            int &xpos, int &ypos,
                            int collectionId = m_currentCollectionId) const;

    /**
     * Set the form layout coordinates for the specified field.
     * Each field has a coordinate for its position in FormView,
     * xpos is the column and ypos the row in a form layout matrix.
     * @param column - the field/column id of which the coordinates are set
     * @param collectionId - the collection id, default is current one
     * @param xpos - the xpos coordinate to set
     * @param ypos - the ypos coordinate to set
     */
    void setFieldCoordinate(const int column,
                            const int xpos, const int ypos,
                            int collectionId = m_currentCollectionId);

    /**
     * Get the form layout unit sizes for the specified field.
     * Each field has size units for its size in FormView.
     * @param column - the field/column id of which the size is needed
     * @param collectionId - the collection id, default is current one
     * @param widthUnits - a reference to a int where width units are saved
     * @param heightUnits - a reference to a int where height units are saved
     */
    void getFieldFormLayoutSize(const int column,
                                int &widthUnits, int &heightUnits,
                                int collectionId = m_currentCollectionId) const;

    /**
     * Set the form layout unit sizes for the specified field.
     * Each field has size units for its size in FormView.
     * @param column - the field/column id
     * @param collectionId - the collection id, default is current one
     * @param widthUnits - width units to set
     * @param heightUnits - height units to set
     */
    void setFieldFormLayoutSize(const int column,
                                const int widthUnits, const int heightUnits,
                                int collectionId = m_currentCollectionId) const;

    /**
     * Get metadata field properties of a field of the specified property class.
     * @param propertyType - the type of the property
     * @param column - the field number
     * @param collectionId - if not specified, current collection is used
     * @return string containing the metadata as "key:value;key2:value2;..."
     */
    QString getFieldProperties(FieldProperty propertyType, const int column,
                             int collectionId = m_currentCollectionId) const;

    /**
     * Set metadata field properties of a field of the specified property class.
     * @param propertyType - the type of the property
     * @param column - the field number
     * @param propertyString conatins the metadata as "key:value;key2:value2;..."
     * @param collectionId - if not specified, current collection is used
     */
    void setFieldProperties(FieldProperty propertyType, const int column,
                            const QString &propertyString,
                            int collectionId = m_currentCollectionId);

    /**
     * Factory method for data model creation. Note that the created model
     * has not a parent, so it needs to be deleted explicitly.
     * @param type - CollectionType for which a model is created
     * @param collectionId - if the collection id is specified then the model
     *                       will be initialized so it can be used
     *                       and bound to a view successfully
     * @return pointer to the newly created model
     */
    QAbstractItemModel* createModel(CollectionType type,
                                    const int collectionId = 0);

    /**
     * Create metadata and tables for a new empty already existing collection.
     * The new collection must be added to the collection table before calling this,
     * because the last id of the inserted collection will be used here.
     * @return id - the id of the initialized collection
     */
    int createNewCollection();

    /**
     * Remove the specified collection, including
     * all metadata and tables
     */
    void deleteCollection(int collectionId);

    /** Delete all records from the specified collection */
    void deleteAllRecords(int collectionId);

    /**
     * Create a new field. This method adds a new column
     * to the collection and updates metadata.
     * @param fieldName - the name for the new field
     * @param type - the data type of the field
     * @param displayProperties - the metadata string for display properties
     * @param editProperties - the metadata string for edit properties
     * @param triggerProperties - the metadata string for trigger properties
     * @param collectionId - the collection id, if not specified default is used
     * @return int - the id of the newly created field
     */
    int createField(const QString &fieldName, FieldType type,
                    const QString &displayProperties,
                    const QString &editProperties,
                    const QString &triggerProperties,
                    int collectionId = m_currentCollectionId);

    /**
     * Modify a field
     * @param fieldId - the id of the field which is being edited
     * @param fieldName - the new name for the field
     * @param displayProperties - the metadata string for display properties
     * @param editProperties - the metadata string for edit properties
     * @param triggerProperties - the metadata string for trigger properties
     * @param collectionId - the collection id, if not specified default is used
     */
    void modifyField(const int &fieldId,
                     const QString &fieldName,
                     const QString &displayProperties,
                     const QString &editProperties,
                     const QString &triggerProperties,
                     int collectionId = m_currentCollectionId);

    /** Delete the specified field from collection and update metadata */
    void deleteField(const int fieldId, int collectionId = m_currentCollectionId);

    /**
     * Add file metadata to the database.
     * Since content files are not directly saved in the database
     * only a reference is saved here.
     * @param fileName - the external file name as defined by the user
     * @param hashName - the name given by the FileManager to store the file
     * @param int - the id of the newly added file
     */
    int addContentFile(const QString &fileName, const QString &hashName);

    /** Remove file metadata for the specified file */
    void removeContentFile(int fileId);

    /**
     * Update a content file
     * @param fileId - the id of the file
     * @param fileName - new file name
     * @param hashName - optionally, new hash name for the FileManager storage
     * @param dateAdded - optionally, new date for file add
     */
    void updateContentFile(int fileId, const QString &fileName,
                           const QString &hashName = QString(),
                           const QDateTime &dateAdded = QDateTime::currentDateTime());

    /**
     * Get metadata for a content file
     * @param fileId - the id of the file
     * @param fileName - reference where the file name is saved
     * @param hashName - reference where hash name is saved
     * @param dateAdded - reference where added date is saved
     * @return bool - whether the specified content file exists or not
     */

    bool getContentFile(int fileId, QString &fileName,
                        QString &hashName,
                        QDateTime &dateAdded);

    /**
     * Get a map of all content files
     * @return QHash - pair of fileId,fileHashName
     */
    QHash<int,QString> getAllContentFiles();

    /** Return the id of the specified file hash name */
    int getContentFileId(const QString &hashName);

    /**
     * Set cached current collection id dirty
     * so on next getCurrentCollectionId() call
     * current collection id is read from database
     * and cache updated. This is used after a cloud sync
     * where collection ids may change
     */
    void setDirtyCurrentColleectionId();

signals:
    /**
     * This signal is emitted whenever the currently active collection
     * changes. This signal is caught by MainWindow which creates the
     * appropriate model and sets up the views.
     */
    void currentCollectionIdChanged(int collectionId);

    /** Signal emitted when the structure of the current collection changes */
    void currentCollectionChanged();

private:
    MetadataEngine(QObject *parent = 0);
    MetadataEngine(const MetadataEngine&) : QObject(0) {}
    ~MetadataEngine();

    /**
     * Helper function used by createModel() to build a standard model,
     * if collectionId is != 0, then the model will be initialized with
     * correct table info and select statement
     */
    QAbstractItemModel* createStandardModel(const int collectionId);

    /** Update the list of field names for the current collection */
    void updateFieldNameCache();

    /**
     * Get the field name directly from metadata table in the database
     * @param column - the column/field number
     * @param collectionId - the collection id
     * @return the column/field name
     */
    QString getFieldNameFromDatabase(const int column, int collectionId) const;

    /** Get the column/field count of the specified colledtion id from db */
    int getFieldCountFromDatabase(const int collectionId) const;

    /** Set the column/field count of the specified collection id */
    void setFieldCount(const int collectionId, int columnCount);

    /** Get the SQL column data type name for the specified field type */
    QString dataTypeSqlName(FieldType type);

    static MetadataEngine *m_instance;
    static int m_currentCollectionId; /**< cached current collection id */
    static QStringList *m_currentCollectionFieldNameList; /**< cached list of field
                                                        names for the active
                                                        collection */
};

#endif // METADATAENGINE_H
