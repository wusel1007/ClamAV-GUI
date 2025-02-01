#include "includeexcludeoptions.h"
#include "ui_includeexcludeoptions.h"

includeExcludeOptions::includeExcludeOptions(QWidget *parent) : QWidget(parent), ui(new Ui::includeExcludeOptions)
{
    ui->setupUi(this);
    setupFile = new setupFileHandler(QDir::homePath() + "/.clamav-gui/settings.ini");
    if (setupFile->sectionExists("REGEXP_and_IncludeExclude") == true){
        readSettingsInProgress = true;
        readSettings();
    }
}

includeExcludeOptions::~includeExcludeOptions()
{
    delete ui;
}

void includeExcludeOptions::slot_writeSettings(){
QString section = "REGEXP_and_IncludeExclude";
QString checked;
QString keyword;
QString value;
    if (readSettingsInProgress == false) {
        keyword = "DontScanFileNamesMatchingRegExp";
        value = ui->dontScanFileNameLineEdit->text();
        ui->dontScanFileNameCheckBox->isChecked()?checked="checked":checked="not checked";
        setupFile->setSectionValue(section,keyword,checked + "|" + value);

        keyword = "DontScanDiretoriesMatchingRegExp";
        value = ui->dontScanDirLineEdit->text();
        ui->dontScanDirCheckBox->isChecked()?checked="checked":checked="not checked";
        setupFile->setSectionValue(section,keyword,checked + "|" + value);

        keyword = "OnlyScanFileNamesMatchingRegExp";
        value = ui->onlyScanFileNameLineEdit->text();
        ui->onlyScanFileNameCheckBox->isChecked()?checked="checked":checked="not checked";
        setupFile->setSectionValue(section,keyword,checked + "|" + value);

        keyword = "OnlyScanDiretoriesMatchingRegExp";
        value = ui->onlyScanDirLineEdit->text();
        ui->onlyScanDirCheckBox->isChecked()?checked="checked":checked="not checked";
        setupFile->setSectionValue(section,keyword,checked + "|" + value);

        keyword = "EnablePUAOptions";
        setupFile->setSectionValue(section,keyword,ui->enablePUACheckBox->isChecked());

        keyword = "LoadPUAPacked";
        setupFile->setSectionValue(section,keyword,ui->loadPUAPackedRadioButon->isChecked());

        keyword = "LoadPUAPWTool";
        setupFile->setSectionValue(section,keyword,ui->loadPUAPWToolRadioButton->isChecked());

        keyword = "LoadPUANetTool";
        setupFile->setSectionValue(section,keyword,ui->loadPUANetToolRadioButton->isChecked());

        keyword = "LoadPUAP2P";
        setupFile->setSectionValue(section,keyword,ui->loadPUAP2PRadioButton->isChecked());

        keyword = "LoadPUAIRC";
        setupFile->setSectionValue(section,keyword,ui->loadPUAIRCRadioButton->isChecked());

        keyword = "LoadPUARAT";
        setupFile->setSectionValue(section,keyword,ui->loadPUARATRadioButton->isChecked());

        keyword = "LoadPUANetToolSpy";
        setupFile->setSectionValue(section,keyword,ui->loadPUANetToolSpyRadioButton->isChecked());

        keyword = "LoadPUAServer";
        setupFile->setSectionValue(section,keyword,ui->loadPUAServerRadioButton->isChecked());

        keyword = "LoadPUAScript";
        setupFile->setSectionValue(section,keyword,ui->loadPUAScriptRadioButton->isChecked());

        keyword = "LoadPUAAndr";
        setupFile->setSectionValue(section,keyword,ui->loadPUAAndrRadioButton->isChecked());

        keyword = "LoadPUAJava";
        setupFile->setSectionValue(section,keyword,ui->loadPUAJavaRadioButton->isChecked());

        keyword = "LoadPUAOsx";
        setupFile->setSectionValue(section,keyword,ui->loadPUAOsxRadioButton->isChecked());

        keyword = "LoadPUATool";
        setupFile->setSectionValue(section,keyword,ui->loadPUAToolRadioButton->isChecked());

        keyword = "LoadPUAUnix";
        setupFile->setSectionValue(section,keyword,ui->loadPUAUnixRadioButton->isChecked());

        keyword = "LoadPUAWin";
        setupFile->setSectionValue(section,keyword,ui->loadPUAWinRadioButton->isChecked());

        emit updateClamdConf();
    }
}

void includeExcludeOptions::readSettings(){
QString section = "REGEXP_and_IncludeExclude";
QString checked;
QString keyword;
QString value;

    keyword = "DontScanFileNamesMatchingRegExp";
    value = setupFile->getSectionValue(section,keyword);
    checked = value.left(value.indexOf("|"));
    value = value.mid(value.indexOf("|") + 1);
    checked=="checked"?ui->dontScanFileNameCheckBox->setChecked(true):ui->dontScanFileNameCheckBox->setChecked(false);
    ui->dontScanFileNameLineEdit->setText(value);

    keyword = "DontScanDiretoriesMatchingRegExp";
    value = setupFile->getSectionValue(section,keyword);
    checked = value.left(value.indexOf("|"));
    value = value.mid(value.indexOf("|") + 1);
    checked=="checked"?ui->dontScanDirCheckBox->setChecked(true):ui->dontScanDirCheckBox->setChecked(false);
    ui->dontScanDirLineEdit->setText(value);

    keyword = "OnlyScanFileNamesMatchingRegExp";
    value = setupFile->getSectionValue(section,keyword);
    checked = value.left(value.indexOf("|"));
    value = value.mid(value.indexOf("|") + 1);
    checked=="checked"?ui->onlyScanFileNameCheckBox->setChecked(true):ui->onlyScanFileNameCheckBox->setChecked(false);
    ui->onlyScanFileNameLineEdit->setText(value);

    keyword = "OnlyScanDiretoriesMatchingRegExp";
    value = setupFile->getSectionValue(section,keyword);
    checked = value.left(value.indexOf("|"));
    value = value.mid(value.indexOf("|") + 1);
    checked=="checked"?ui->onlyScanDirCheckBox->setChecked(true):ui->onlyScanDirCheckBox->setChecked(false);
    ui->onlyScanDirLineEdit->setText(value);

    keyword = "EnablePUAOptions";
    if (setupFile->getSectionBoolValue(section,keyword) == true){
        ui->enablePUACheckBox->setChecked(true);
        ui->PUAFrame->setEnabled(true);
    } else {
        ui->enablePUACheckBox->setChecked(false);
        ui->PUAFrame->setEnabled(false);
    }

    keyword = "LoadPUAPacked";
    ui->loadPUAPackedRadioButon->setChecked(setupFile->getSectionBoolValue(section,keyword));

    keyword = "LoadPUAPWTool";
    ui->loadPUAPWToolRadioButton->setChecked(setupFile->getSectionBoolValue(section,keyword));

    keyword = "LoadPUANetTool";
    ui->loadPUANetToolRadioButton->setChecked(setupFile->getSectionBoolValue(section,keyword));

    keyword = "LoadPUAP2P";
    ui->loadPUAP2PRadioButton->setChecked(setupFile->getSectionBoolValue(section,keyword));

    keyword = "LoadPUAIRC";
    ui->loadPUAIRCRadioButton->setChecked(setupFile->getSectionBoolValue(section,keyword));

    keyword = "LoadPUARAT";
    ui->loadPUARATRadioButton->setChecked(setupFile->getSectionBoolValue(section,keyword));

    keyword = "LoadPUANetToolSpy";
    ui->loadPUANetToolSpyRadioButton->setChecked(setupFile->getSectionBoolValue(section,keyword));

    keyword = "LoadPUAServer";
    ui->loadPUAServerRadioButton->setChecked(setupFile->getSectionBoolValue(section,keyword));

    keyword = "LoadPUAScript";
    ui->loadPUAScriptRadioButton->setChecked(setupFile->getSectionBoolValue(section,keyword));

    keyword = "LoadPUAAndr";
    ui->loadPUAAndrRadioButton->setChecked(setupFile->getSectionBoolValue(section,keyword));

    keyword = "LoadPUAJava";
    ui->loadPUAJavaRadioButton->setChecked(setupFile->getSectionBoolValue(section,keyword));

    keyword = "LoadPUAOsx";
    ui->loadPUAOsxRadioButton->setChecked(setupFile->getSectionBoolValue(section,keyword));

    keyword = "LoadPUATool";
    ui->loadPUAToolRadioButton->setChecked(setupFile->getSectionBoolValue(section,keyword));

    keyword = "LoadPUAUnix";
    ui->loadPUAUnixRadioButton->setChecked(setupFile->getSectionBoolValue(section,keyword));

    keyword = "LoadPUAWin";
    ui->loadPUAWinRadioButton->setChecked(setupFile->getSectionBoolValue(section,keyword));

    readSettingsInProgress = false;
}

void includeExcludeOptions::slot_enablePUACheckBoxClicked(){
    ui->PUAFrame->setEnabled(ui->enablePUACheckBox->isChecked());
    slot_writeSettings();
}
