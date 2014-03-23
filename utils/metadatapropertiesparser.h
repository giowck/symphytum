/**
  * \class MetadataPropertiesParser
  * \brief This utility is used to parse metadata field properties.
  * \author Giorgio Wicklein - GIOWISYS Software
  * \date 03/07/2012
  */

#ifndef METADATAPROPERTIESPARSER_H
#define METADATAPROPERTIESPARSER_H


//-----------------------------------------------------------------------------
// Headers
//-----------------------------------------------------------------------------

#include <QtCore/QHash>


//-----------------------------------------------------------------------------
// Forward declarations
//-----------------------------------------------------------------------------

class QString;


//-----------------------------------------------------------------------------
// MetadataPropertiesParser
//-----------------------------------------------------------------------------

class MetadataPropertiesParser
{
public:
    /** Construct a metadata parser with the given metadata string */
    MetadataPropertiesParser(const QString &metadataString);
    ~MetadataPropertiesParser();

    /** Return the value for the specified metadata property key */
    QString getValue(const QString &key);

    /** Return the size of the map */
    int size() const;

private:
    QHash<QString, QString> *m_propertiesMap;
};

#endif // METADATAPROPERTIESPARSER_H
