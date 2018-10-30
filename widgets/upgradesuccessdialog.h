#ifndef UPGRADESUCCESSDIALOG_H
#define UPGRADESUCCESSDIALOG_H

#include <QDialog>

namespace Ui {
class UpgradeSuccessDialog;
}

class UpgradeSuccessDialog : public QDialog
{
    Q_OBJECT

public:
    explicit UpgradeSuccessDialog(QWidget *parent = nullptr);
    ~UpgradeSuccessDialog();

private:
    Ui::UpgradeSuccessDialog *ui;
};

#endif // UPGRADESUCCESSDIALOG_H
