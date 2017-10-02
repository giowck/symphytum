/*
 *  Copyright (c) 2012 Giorgio Wicklein <giowckln@gmail.com>
 */

//-----------------------------------------------------------------------------
// Hearders
//-----------------------------------------------------------------------------

#include "metadatapropertiesparser.h"

#include <QtCore/QString>
#include <QtCore/QStringList>


//-----------------------------------------------------------------------------
// Public
//-----------------------------------------------------------------------------

MetadataPropertiesParser::MetadataPropertiesParser(const QString &metadataString) :
    m_propertiesMap(0)
{
    m_propertiesMap = new QHash<QString, QString>();

    //parse
    QString s;
    QStringList properties = metadataString.split(";", QString::SkipEmptyParts);
    QStringList pair;

    foreach (s, properties) {
        pair = s.split(":", QString::SkipEmptyParts);
        if (pair.size() == 2) {
            //add key-value pair to map
            m_propertiesMap->insert(pair.at(0), pair.at(1));
        }
    }
}

MetadataPropertiesParser::~MetadataPropertiesParser()
{
    if (m_propertiesMap) {
        m_propertiesMap->clear();
        delete m_propertiesMap;
    }
}

QString MetadataPropertiesParser::getValue(const QString &key)
{
    return m_propertiesMap->value(key, ""); //empty string means not found
}

int MetadataPropertiesParser::size() const
{
    return m_propertiesMap->size();
}
