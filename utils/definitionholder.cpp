/*
 *  Copyright (c) 2012 Giorgio Wicklein <giowckln@gmail.com>
 */

//-----------------------------------------------------------------------------
// Hearders
//-----------------------------------------------------------------------------

#include "definitionholder.h"

#include <QtCore/QString>
#include <QtCore/QDate>


//-----------------------------------------------------------------------------
// Static variables initialization
//-----------------------------------------------------------------------------

QString DefinitionHolder::VERSION = "2.3";
QString DefinitionHolder::NAME = "Symphytum";
QString DefinitionHolder::COMPANY = "giowisys";
QString DefinitionHolder::DOMAIN_NAME = "giowisys.com";
QString DefinitionHolder::UPDATE_URL = "https://giowck.github.io/symphytum/update/symphytum/updates";
QString DefinitionHolder::DOWNLOAD_URL = "https://giowck.github.io/symphytum/update.html";
int DefinitionHolder::SOFTWARE_BUILD = 8;
int DefinitionHolder::DATABASE_VERSION = 2;
bool DefinitionHolder::APP_STORE = false;
bool DefinitionHolder::APPIMAGE_LINUX = false;
bool DefinitionHolder::WIN_PORTABLE = false;
QString DefinitionHolder::COPYRIGHT =
        QString("Copyright &copy; 2014-%1 Symphytum Developers"
                "<br />Copyright &copy; 2012-2014 GIOWISYS Software UG (haftungsbeschr%2nkt)")
        .arg(QDate::currentDate().toString("yyyy"))
        .arg(QChar(228)); // 228=umlaut a

