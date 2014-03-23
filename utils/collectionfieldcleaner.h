/**
  * \class CollectionFieldCleaner
  * \brief This utility is used to clean collection fields
  *        before they are deleted. Some field types such as
  *        image, file list and date don't contain the data directly
  *        but only references (file ids, alarm ids...). So those
  *        fields need to be cleaned before deletion and this utility
  *        handles that deletion triggers.
  * \author Giorgio Wicklein - GIOWISYS Software
  * \date 02/11/2012
  */

#ifndef COLLECTIONFIELDCLEANER_H
#define COLLECTIONFIELDCLEANER_H


//-----------------------------------------------------------------------------
// Headers
//-----------------------------------------------------------------------------

#include <QtCore/QObject>


//-----------------------------------------------------------------------------
// Forward declarations
//-----------------------------------------------------------------------------

class MetadataEngine;


//-----------------------------------------------------------------------------
// CollectionFieldCleaner
//-----------------------------------------------------------------------------

class CollectionFieldCleaner : public QObject
{
    Q_OBJECT

public:
    explicit CollectionFieldCleaner(QObject *parent = 0);

    /** Clean the specified field */
    void cleanField(int collectionId, int fieldId);

    /** Clean all fields of the specified collection */
    void cleanCollection(int collectionId);

private:
    MetadataEngine *m_metadataEngine;
};

#endif // COLLECTIONFIELDCLEANER_H
