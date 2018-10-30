/**
  * \class UpgradeSuccessDialog
  * \brief This dialog is shown after a software update was installed to inform the user
  * that the software has been update successfully.
  * \author Oirio Joshi (joshirio)
  * \date 2018-10-30
  */

#include "upgradesuccessdialog.h"
#include "ui_upgradesuccessdialog.h"

#include "../utils/definitionholder.h"

UpgradeSuccessDialog::UpgradeSuccessDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::UpgradeSuccessDialog)
{
    ui->setupUi(this);

    ui->buttonBox->setStandardButtons(QDialogButtonBox::Ok);

    //setup info
    ui->newVersionLabel->setText("<b>" + DefinitionHolder::VERSION + "</b>");
}

UpgradeSuccessDialog::~UpgradeSuccessDialog()
{
    delete ui;
}
