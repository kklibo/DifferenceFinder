#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>
#include <QComboBox>

#include "usersettings.h"
#include "defensivecoding.h"

namespace Ui {
class SettingsDialog;
}

class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsDialog(QWidget *parent = 0);
    ~SettingsDialog();

    void initUserSettings(UserSettings &settings);
    UserSettings getUserSettings();

private slots:
    void on_CB_ColumnCount_currentTextChanged(const QString &arg1);

    void on_SB_ColumnCountN_editingFinished();

private:

    //get the enum data we stored in the combo box item's Qt::UserRole QVariant
    template <typename T>
    bool getUserDataFromCurrentItem(const QComboBox& CB, T& selected)
    {
        const QVariant userData = CB.currentData(Qt::UserRole);
        if ( !userData.canConvert<T>() ) {
            LOG.Error("invalid combobox item");
            return false;
        }
        selected = userData.value<T>();
        return true;
    }

    Ui::SettingsDialog *ui;
    UserSettings m_userSettings;

};

#endif // SETTINGSDIALOG_H
