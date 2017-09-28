/*
 *  Copyright (c) 2012 Giorgio Wicklein <giorgio.wicklein@giowisys.com>
 */

//-----------------------------------------------------------------------------
// Hearders
//-----------------------------------------------------------------------------

#include "widgets/mainwindow.h"
#include "utils/definitionholder.h"
#include "utils/qtsingleapplication/qtsingleapplication.h"

#include <QtCore/QTranslator>
#include <QtCore/QLocale>


//-----------------------------------------------------------------------------
// Main
//-----------------------------------------------------------------------------

int main(int argc, char *argv[])
{
    //enable high DPI scaling on Windows and Linux
    //NOTE: don't move, must be called before QApplication initialization
    //or it won't work at all
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

    QtSingleApplication symphytumApp(argc, argv);
    symphytumApp.setApplicationName(DefinitionHolder::NAME);
    symphytumApp.setApplicationVersion(DefinitionHolder::VERSION);
    symphytumApp.setOrganizationName(DefinitionHolder::COMPANY);
    symphytumApp.setOrganizationDomain(DefinitionHolder::DOMAIN_NAME);
    symphytumApp.setWindowIcon(QIcon(":/images/icons/symphytum.png"));

    //only one instance allowed
    if (symphytumApp.sendMessage("Wake up!"))
        return 0;

    //setup translations
    QTranslator qtTranslator;
    qtTranslator.load("qt_" + QLocale::system().name(),
                      ":/languages");
    symphytumApp.installTranslator(&qtTranslator);
    QTranslator myappTranslator;
    myappTranslator.load("symphytum_" + QLocale::system().name(), ":/languages");
    symphytumApp.installTranslator(&myappTranslator);

    //init gui
    MainWindow w;
    w.show();

    //wake up window
    symphytumApp.setActivationWindow(&w);

    return symphytumApp.exec();
}
