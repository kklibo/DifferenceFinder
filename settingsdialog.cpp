#include "settingsdialog.h"
#include "ui_settingsdialog.h"

SettingsDialog::SettingsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SettingsDialog)
{
    ui->setupUi(this);

    //populate the combo boxes, with enums as userData for easy referencing

    ui->CB_ColumnCount->insertItem(0, "Fill (default)",                 QVariant::fromValue(dataSetView::ByteGridColumnMode::Fill));
    ui->CB_ColumnCount->insertItem(1, "Largest multiple of",            QVariant::fromValue(dataSetView::ByteGridColumnMode::LargestMultipleOfN));
    ui->CB_ColumnCount->insertItem(2, "Largest power of 2",             QVariant::fromValue(dataSetView::ByteGridColumnMode::LargestPowerOf2));
    ui->CB_ColumnCount->insertItem(3, "Up to",                          QVariant::fromValue(dataSetView::ByteGridColumnMode::UpToN));
    ui->CB_ColumnCount->insertItem(4, "Largest power of 2 *(1 or 1.5)", QVariant::fromValue(dataSetView::ByteGridColumnMode::LargestPowerOf2Extra));
    //note: on_CB_ColumnCount_currentTextChanged seems to be automatically called once after these calls to insertItem

    ui->CB_Scrolling->insertItem(0, "Fixed Rows (default)", QVariant::fromValue(dataSetView::ByteGridScrollingMode::FixedRows));
    ui->CB_Scrolling->insertItem(1, "Free",                 QVariant::fromValue(dataSetView::ByteGridScrollingMode::Free));

}

SettingsDialog::~SettingsDialog()
{
    delete ui;
}

void SettingsDialog::initUserSettings(UserSettings &settings)
{
    m_userSettings = settings;

    //use the enums stored in the combobox items to find the current settings and select them

    //Column Count combobox
    int index;
    index = ui->CB_ColumnCount->findData(QVariant::fromValue(m_userSettings.byteGridColumnMode));
    if (index >= 0){
        ui->CB_ColumnCount->setCurrentIndex(index);
    } else {
        LOG.Error("invalid Column Count setting");
    }

    //Scrolling combobox
    index = ui->CB_Scrolling->findData(QVariant::fromValue(m_userSettings.byteGridScrollingMode));
    if (index >= 0){
        ui->CB_Scrolling->setCurrentIndex(index);
    } else {
        LOG.Error("invalid Scrolling setting");
    }
}

UserSettings SettingsDialog::getUserSettings()
{
    //update m_userSettings
    // set combo box selections
    dataSetView::ByteGridColumnMode byteGridColumnMode;
    if (getUserDataFromCurrentItem<dataSetView::ByteGridColumnMode>(*ui->CB_ColumnCount, byteGridColumnMode)) {
        m_userSettings.byteGridColumnMode = byteGridColumnMode;
    }

    dataSetView::ByteGridScrollingMode byteGridScrollingMode;
    if (getUserDataFromCurrentItem<dataSetView::ByteGridScrollingMode>(*ui->CB_Scrolling, byteGridScrollingMode)) {
        m_userSettings.byteGridScrollingMode = byteGridScrollingMode;
    }

    //N values are already set (when edited)


    return m_userSettings;
}

void SettingsDialog::on_CB_ColumnCount_currentTextChanged(const QString &/*arg1*/)
{
    //get the enum data we stored in the combo box item
    dataSetView::ByteGridColumnMode byteGridColumnMode;
    if (!getUserDataFromCurrentItem<dataSetView::ByteGridColumnMode>(*ui->CB_ColumnCount, byteGridColumnMode)) {
        return;
    }

    //set the multi-mode text entry Spin Box for the current column setting
    if (byteGridColumnMode == dataSetView::ByteGridColumnMode::LargestMultipleOfN) {
        ui->SB_ColumnCountN->setEnabled(true);
        ASSERT_LE_INT_MAX(m_userSettings.byteGridColumn_LargestMultipleOf_N);
        ui->SB_ColumnCountN->setValue(static_cast<int>(m_userSettings.byteGridColumn_LargestMultipleOf_N));
        ui->SB_ColumnCountN->setMinimum(1);
    } else
    if (byteGridColumnMode == dataSetView::ByteGridColumnMode::UpToN) {
        ui->SB_ColumnCountN->setEnabled(true);
        ASSERT_LE_INT_MAX(m_userSettings.byteGridColumn_UpTo_N);
        ui->SB_ColumnCountN->setValue(static_cast<int>(m_userSettings.byteGridColumn_UpTo_N));
        ui->SB_ColumnCountN->setMinimum(1);
    } else {
        ui->SB_ColumnCountN->setEnabled(false);
        ui->SB_ColumnCountN->clear();
    }
}

void SettingsDialog::on_SB_ColumnCountN_editingFinished()
{
    //get the enum data we stored in the combo box item
    dataSetView::ByteGridColumnMode selected;
    if (!getUserDataFromCurrentItem<dataSetView::ByteGridColumnMode>(*ui->CB_ColumnCount, selected)) {
        return;
    }

    //apply the value of the multi-mode Spin Box to the current column setting, if applicable
    QString arg1 = ui->CB_ColumnCount->currentText();
    if (selected == dataSetView::ByteGridColumnMode::LargestMultipleOfN) {
        int val = ui->SB_ColumnCountN->value();
        ASSERT(1 <= val);
        m_userSettings.byteGridColumn_LargestMultipleOf_N = static_cast<unsigned int>(val);
    } else
    if (selected == dataSetView::ByteGridColumnMode::UpToN) {
        int val = ui->SB_ColumnCountN->value();
        ASSERT(1 <= val);
        m_userSettings.byteGridColumn_UpTo_N = static_cast<unsigned int>(val);
    }
}
