/**
  * \class ComboboxFieldWizard
  * \brief Wizard class for creation and configuration of combobox type fields
  * \author Giorgio Wicklein - GIOWISYS Software
  * \date 15/10/2012
  */

#ifndef COMBOBOXFIELDWIZARD_H
#define COMBOBOXFIELDWIZARD_H


//-----------------------------------------------------------------------------
// Headers
//-----------------------------------------------------------------------------

#include "abstractfieldwizard.h"


//-----------------------------------------------------------------------------
// Forward declarations
//-----------------------------------------------------------------------------

namespace Ui {
class ComboboxFieldWizard;
}


//-----------------------------------------------------------------------------
// ComboboxFieldWizard
//-----------------------------------------------------------------------------

class ComboboxFieldWizard : public AbstractFieldWizard
{
    Q_OBJECT

public:
    explicit ComboboxFieldWizard(const QString &fieldName,
                                 QWidget *parent = nullptr,
                                 AbstractFieldWizard::EditMode editMode
                                 = AbstractFieldWizard::NewEditMode);
    ~ComboboxFieldWizard();

    void getFieldProperties(QString &displayProperties,
                            QString &editProperties,
                            QString &triggerProperties);
    void loadField(const int fieldId, const int collectionId);

private slots:
    void updateFinishButton();
    void updateListButtons();
    void addItemButtonClicked();
    void renameItemButtonClicked();
    void removeItemButtonClicked();
    void updateDefaultComboBox();
    void defaultItemChanged();
    void clearDefaultItemSlot();
    void finishButtonClicked();

private:
    Ui::ComboboxFieldWizard *ui;
    int m_default;
    QString m_pendingDefaultSqlStatement;
    QStringList m_pendingSqlStatements;
};

#endif // COMBOBOXFIELDWIZARD_H
