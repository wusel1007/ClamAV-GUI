#include "scanlimitstab.h"
#include "ui_scanlimitstab.h"

scanLimitsTab::scanLimitsTab(QWidget *parent) : QWidget(parent), ui(new Ui::scanLimitsTab)
{
    ui->setupUi(this);
    setupFile = new setupFileHandler(QDir::homePath() + "/.clamav-gui/settings.ini");
    if (setupFile->sectionExists("ScanLimitations") == true) updateLimits(); else writeLimits();

    getClamscanParametersProcess = new QProcess(this);
    connect(getClamscanParametersProcess,SIGNAL(readyReadStandardError()),this,SLOT(slot_getClamscanProcessHasOutput()));
    connect(getClamscanParametersProcess,SIGNAL(readyReadStandardOutput()),this,SLOT(slot_getClamscanProcessHasOutput()));
    connect(getClamscanParametersProcess,SIGNAL(finished(int)),this,SLOT(slot_getClamscanProcessFinished()));

    QStringList parameters;
    parameters << "--help";

    getClamscanParametersProcess->start("clamscan",parameters);
}

scanLimitsTab::~scanLimitsTab()
{
    delete ui;
}

void scanLimitsTab::writeLimits(){
QString keyword;
QString value;
QString checked;

    keyword = "Files larger than this will be skipped and assumed clean";
    value = QString::number(ui->filesLargerThanThisSpinBox->value()) + ui->filesLargerThanThisComboBox->currentText();
    ui->filesLargerThanThisCheckBox->isChecked() == true?checked="checked":checked="not checked";
    setupFile->setSectionValue("ScanLimitations",keyword,checked + "|" + value);

    keyword = "The maximum amount of data to scan for each container file";
    value = QString::number(ui->maxAmountForContainerSpinBox->value()) + ui->maxAmountForContainerComboBox->currentText();
    ui->maxAmountForContainerCheckBox->isChecked() == true?checked="checked":checked="not checked";
    setupFile->setSectionValue("ScanLimitations",keyword,checked + "|" + value);

    keyword = "The maximum number of files to scan for each container file";
    value = QString::number(ui->maxNumberForContainerSpinBox->value()) + ui->maxNumberForContainerComboBox->currentText();
    ui->maxNumberForContainerCheckBox->isChecked() == true?checked="checked":checked="not checked";
    setupFile->setSectionValue("ScanLimitations",keyword,checked + "|" + value);

    keyword = "Maximum archive recursion level for container file";
    value = QString::number(ui->maxArchiveRecursionForContainerSpinBox->value()) + ui->maxArchiveRecursionForContainerComboBox->currentText();
    ui->maxArchiveRecursionForContainerCheckBox->isChecked() == true?checked="checked":checked="not checked";
    setupFile->setSectionValue("ScanLimitations",keyword,checked + "|" + value);

    keyword = "Maximum directory recursion level";
    value = QString::number(ui->maxDirRecursionLevelSpinBox->value()) + ui->maxDirRecursionLevelComboBox->currentText();
    ui->maxDirRecursionLevelCheckBox->isChecked() == true?checked="checked":checked="not checked";
    setupFile->setSectionValue("ScanLimitations",keyword,checked + "|" + value);

    keyword = "Maximum size file to check for embedded PE";
    value = QString::number(ui->maxSizeFileForPESpinBox->value()) + ui->maxSizeFileForPEComboBox->currentText();
    ui->maxSizeFileForPECheckBox->isChecked() == true?checked="checked":checked="not checked";
    setupFile->setSectionValue("ScanLimitations",keyword,checked + "|" + value);

    keyword = "Maximum size of HTML file to normalize";
    value = QString::number(ui->maxSizeHTMLFileToNormalizeSpinBox->value()) + ui->maxSizeHTMLFileToNormalizeComboBox->currentText();
    ui->maxSizeHTMLFileToNormalizeCheckBox->isChecked() == true?checked="checked":checked="not checked";
    setupFile->setSectionValue("ScanLimitations",keyword,checked + "|" + value);

    keyword = "Maximum size of normalized HTML file to scan";
    value = QString::number(ui->maxSizeOfNormalizedHTMLFileSpinBox->value()) + ui->maxSizeOfNormalizedHTMLFileComboBox->currentText();
    ui->maxSizeOfNormalizedHTMLFileCheckBox->isChecked() == true?checked="checked":checked="not checked";
    setupFile->setSectionValue("ScanLimitations",keyword,checked + "|" + value);

    keyword = "Maximum size of script file to normalize";
    value = QString::number(ui->maxSizeOfScriptFileToNormalizeSpinBox->value()) + ui->maxSizeOfScriptFileToNormalizeComboBox->currentText();
    ui->maxSizeOfScriptFileToNormalizeCheckBox->isChecked() == true?checked="checked":checked="not checked";
    setupFile->setSectionValue("ScanLimitations",keyword,checked + "|" + value);

    keyword = "Maximum size zip to type reanalyze";
    value = QString::number(ui->maxSizeZipToTypeReanalzeSpinBox->value()) + ui->maxSizeZipToTypeReanalzeComboBox->currentText();
    ui->maxSizeZipToTypeReanalzeCheckBox->isChecked() == true?checked="checked":checked="not checked";
    setupFile->setSectionValue("ScanLimitations",keyword,checked + "|" + value);

    keyword = "Maximum number of partitions in disk image to be scanned";
    value = QString::number(ui->maxNumberOfPartitionsInDiskImageSpinBox->value()) + ui->maxNumberOfPartitionsInDiskImageComboBox->currentText();
    ui->maxNumberOfPartitionsInDiskImageCheckBox->isChecked() == true?checked="checked":checked="not checked";
    setupFile->setSectionValue("ScanLimitations",keyword,checked + "|" + value);

    keyword = "Maximum number of icons in PE file to be scanned";
    value = QString::number(ui->maxNumberOfIconsInPEFileSpinBox->value()) + ui->maxNumberOfIconsInPEFileComboBox->currentText();
    ui->maxNumberOfIconsInPEFileCheckBox->isChecked() == true?checked="checked":checked="not checked";
    setupFile->setSectionValue("ScanLimitations",keyword,checked + "|" + value);

    keyword = "Number of seconds to wait for waiting a response back from the stats server";
    value = QString::number(ui->numberOfSecondsToWaitForResponseSpinBox->value());
    ui->numberOfSecondsToWaitForResponseCheckBox->isChecked() == true?checked="checked":checked="not checked";
    setupFile->setSectionValue("ScanLimitations",keyword,checked + "|" + value);

    keyword = "Bytecode timeout in milliseconds";
    value = QString::number(ui->numberOfSecondsToWaitForByteCodeSpinBox->value());
    ui->numberOfSecondsToWaitForByteCodeCheckBox->isChecked() == true?checked="checked":checked="not checked";
    setupFile->setSectionValue("ScanLimitations",keyword,checked + "|" + value);

    keyword = "Collect and print execution statistics";
    value = QString::number(ui->collectAndPrintExecutionStatisticsComboBox->currentIndex());
    ui->collectAndPrintExecutionStatisticsCheckBox->isChecked() == true?checked="checked":checked="not checked";
    setupFile->setSectionValue("ScanLimitations",keyword,checked + "|" + value);

    keyword = "Structured SSN Format";
    value = QString::number(ui->structuredSSNFormatComboBox->currentIndex());
    ui->structuredSSNFormatCheckBox->isChecked() == true?checked="checked":checked="not checked";
    setupFile->setSectionValue("ScanLimitations",keyword,checked + "|" + value);

    keyword = "Structured SSN Count";
    value = QString::number(ui->structuredSSNCountSpinBox->value());
    ui->structuredSSNCountCheckBox->isChecked() == true?checked="checked":checked="not checked";
    setupFile->setSectionValue("ScanLimitations",keyword,checked + "|" + value);

    keyword = "Structured CC Count";
    value = QString::number(ui->structuredCCCountSpinBox->value());
    ui->structuredCCCountCheckBox->isChecked() == true?checked="checked":checked="not checked";
    setupFile->setSectionValue("ScanLimitations",keyword,checked + "|" + value);

    keyword = "Structured CC Mode";
    value = QString::number(ui->structuredCCModeComboBox->currentIndex());
    ui->structuredCCModeCheckBox->isChecked() == true?checked="checked":checked="not checked";
    setupFile->setSectionValue("ScanLimitations",keyword,checked + "|" + value);

    keyword = "Max Scan-Time";
    value = QString::number(ui->maxScanTimeSpinBox->value());
    ui->maxScanTimeCheckBox->isChecked() == true?checked="checked":checked="not checked";
    setupFile->setSectionValue("ScanLimitations",keyword,checked + "|" + value);

    keyword = "Max recursion to HWP3 parsing function";
    value = QString::number(ui->maxRecursionHWP3SpinBox->value());
    ui->maxRecursionHWP3CheckBox->isChecked() == true?checked="checked":checked="not checked";
    setupFile->setSectionValue("ScanLimitations",keyword,checked + "|" + value);

    keyword = "Max calls to PCRE match function";
    value = QString::number(ui->maxPCREMatchSpinBox->value());
    ui->maxPCREMatchCheckBox->isChecked() == true?checked="checked":checked="not checked";
    setupFile->setSectionValue("ScanLimitations",keyword,checked + "|" + value);

    keyword = "Max recursion calls to the PCRE match function";
    value = QString::number(ui->maxRecursionPCREMatchSpinBox->value());
    ui->maxRecursionPCREMatchCheckBox->isChecked() == true?checked="checked":checked="not checked";
    setupFile->setSectionValue("ScanLimitations",keyword,checked + "|" + value);

    keyword = "Max PCRE file size";
    value = QString::number(ui->maxPCREFileSizeSpinBox->value()) + ui->maxPCREFileSizeComboBox->currentText();
    ui->maxPCREFileSizeCheckBox->isChecked() == true?checked="checked":checked="not checked";
    setupFile->setSectionValue("ScanLimitations",keyword,checked + "|" + value);

    keyword = "Database outdated if older than x days";
    value = QString::number(ui->databaseOutdatedSpinBox->value());
    ui->databaseOutdatedCheckBox->isChecked() == true?checked="checked":checked="not checked";
    setupFile->setSectionValue("ScanLimitations",keyword,checked + "|" + value);

    emit updateClamdConf();
}

void scanLimitsTab::slot_getClamscanProcessHasOutput()
{
    getClamscanProcessOutput = getClamscanProcessOutput + getClamscanParametersProcess->readAll();
}

void scanLimitsTab::slot_getClamscanProcessFinished()
{

    getClamscanProcessOutput = getClamscanProcessOutput + getClamscanParametersProcess->readAll();

    ui->filesLargerThanThisComboBox->setVisible(false);
    ui->filesLargerThanThisCheckBox->setVisible(false);
    ui->filesLargerThanThisSpinBox->setVisible(false);

    ui->maxAmountForContainerCheckBox->setVisible(false);
    ui->maxAmountForContainerComboBox->setVisible(false);
    ui->maxAmountForContainerSpinBox->setVisible(false);

    ui->maxArchiveRecursionForContainerCheckBox->setVisible(false);
    ui->maxArchiveRecursionForContainerComboBox->setVisible(false);
    ui->maxArchiveRecursionForContainerSpinBox->setVisible(false);

    ui->maxDirRecursionLevelCheckBox->setVisible(false);
    ui->maxDirRecursionLevelComboBox->setVisible(false);
    ui->maxDirRecursionLevelSpinBox->setVisible(false);

    ui->maxNumberForContainerCheckBox->setVisible(false);
    ui->maxNumberForContainerComboBox->setVisible(false);
    ui->maxNumberForContainerSpinBox->setVisible(false);

    ui->maxNumberOfIconsInPEFileCheckBox->setVisible(false);
    ui->maxNumberOfIconsInPEFileComboBox->setVisible(false);
    ui->maxNumberOfIconsInPEFileSpinBox->setVisible(false);

    ui->maxNumberOfPartitionsInDiskImageCheckBox->setVisible(false);
    ui->maxNumberOfPartitionsInDiskImageComboBox->setVisible(false);
    ui->maxNumberOfPartitionsInDiskImageSpinBox->setVisible(false);

    ui->maxPCREFileSizeCheckBox->setVisible(false);
    ui->maxPCREFileSizeComboBox->setVisible(false);
    ui->maxPCREFileSizeSpinBox->setVisible(false);

    ui->maxPCREMatchCheckBox->setVisible(false);
    ui->maxPCREMatchSpinBox->setVisible(false);

    ui->maxRecursionHWP3CheckBox->setVisible(false);
    ui->maxRecursionHWP3SpinBox->setVisible(false);

    ui->maxRecursionPCREMatchCheckBox->setVisible(false);
    ui->maxRecursionPCREMatchSpinBox->setVisible(false);

    ui->maxScanTimeCheckBox->setVisible(false);
    ui->maxScanTimeSpinBox->setVisible(false);

    ui->maxSizeFileForPECheckBox->setVisible(false);
    ui->maxSizeFileForPEComboBox->setVisible(false);
    ui->maxSizeFileForPESpinBox->setVisible(false);

    ui->maxSizeHTMLFileToNormalizeCheckBox->setVisible(false);
    ui->maxSizeHTMLFileToNormalizeComboBox->setVisible(false);
    ui->maxSizeHTMLFileToNormalizeSpinBox->setVisible(false);

    ui->maxSizeOfNormalizedHTMLFileCheckBox->setVisible(false);
    ui->maxSizeOfNormalizedHTMLFileComboBox->setVisible(false);
    ui->maxSizeOfNormalizedHTMLFileSpinBox->setVisible(false);

    ui->maxSizeOfScriptFileToNormalizeCheckBox->setVisible(false);
    ui->maxSizeOfScriptFileToNormalizeComboBox->setVisible(false);
    ui->maxSizeOfScriptFileToNormalizeSpinBox->setVisible(false);

    ui->maxSizeZipToTypeReanalzeCheckBox->setVisible(false);
    ui->maxSizeZipToTypeReanalzeComboBox->setVisible(false);
    ui->maxSizeZipToTypeReanalzeSpinBox->setVisible(false);

    ui->numberOfSecondsToWaitForByteCodeCheckBox->setVisible(false);
    ui->numberOfSecondsToWaitForByteCodeSpinBox->setVisible(false);

    ui->numberOfSecondsToWaitForResponseCheckBox->setVisible(false);
    ui->numberOfSecondsToWaitForResponseSpinBox->setVisible(false);

    ui->structuredCCCountCheckBox->setVisible(false);
    ui->structuredCCCountSpinBox->setVisible(false);

    ui->structuredCCModeCheckBox->setVisible(false);
    ui->structuredCCModeComboBox->setVisible(false);

    ui->structuredSSNCountCheckBox->setVisible(false);
    ui->structuredSSNCountSpinBox->setVisible(false);

    ui->structuredSSNFormatCheckBox->setVisible(false);
    ui->structuredSSNFormatComboBox->setVisible(false);

    ui->collectAndPrintExecutionStatisticsCheckBox->setVisible(false);
    ui->collectAndPrintExecutionStatisticsComboBox->setVisible(false);

    ui->databaseOutdatedCheckBox->setVisible(false);
    ui->databaseOutdatedSpinBox->setVisible(false);

    QStringList lines = getClamscanProcessOutput.split("\n");
    QString line;

    for (int x = 0; x < lines.size(); x++) {
        line = lines[x];
        if (line.indexOf("--max-filesize") != -1) {
            ui->filesLargerThanThisComboBox->setVisible(true);
            ui->filesLargerThanThisCheckBox->setVisible(true);
            ui->filesLargerThanThisSpinBox->setVisible(true);
        }

        if (line.indexOf("--max-scansize") != -1) {
            ui->maxAmountForContainerCheckBox->setVisible(true);
            ui->maxAmountForContainerComboBox->setVisible(true);
            ui->maxAmountForContainerSpinBox->setVisible(true);
        }

        if (line.indexOf("--max-recursion") != -1) {
            ui->maxArchiveRecursionForContainerCheckBox->setVisible(true);
            ui->maxArchiveRecursionForContainerComboBox->setVisible(true);
            ui->maxArchiveRecursionForContainerSpinBox->setVisible(true);
        }

        if (line.indexOf("--max-dir-recursion") != -1) {
            ui->maxDirRecursionLevelCheckBox->setVisible(true);
            ui->maxDirRecursionLevelComboBox->setVisible(true);
            ui->maxDirRecursionLevelSpinBox->setVisible(true);
        }
        if (line.indexOf("--max-files") != -1) {
            ui->maxNumberForContainerCheckBox->setVisible(true);
            ui->maxNumberForContainerComboBox->setVisible(true);
            ui->maxNumberForContainerSpinBox->setVisible(true);
        }

        if (line.indexOf("--max-iconspe") != -1) {
            ui->maxNumberOfIconsInPEFileCheckBox->setVisible(true);
            ui->maxNumberOfIconsInPEFileComboBox->setVisible(true);
            ui->maxNumberOfIconsInPEFileSpinBox->setVisible(true);
        }

        if (line.indexOf("--max-partitions") != -1) {
            ui->maxNumberOfPartitionsInDiskImageCheckBox->setVisible(true);
            ui->maxNumberOfPartitionsInDiskImageComboBox->setVisible(true);
            ui->maxNumberOfPartitionsInDiskImageSpinBox->setVisible(true);
        }

        if (line.indexOf("--pcre-max-filesize") != -1) {
            ui->maxPCREFileSizeCheckBox->setVisible(true);
            ui->maxPCREFileSizeComboBox->setVisible(true);
            ui->maxPCREFileSizeSpinBox->setVisible(true);
        }

        if (line.indexOf("--pcre-match-limit") != -1) {
            ui->maxPCREMatchCheckBox->setVisible(true);
            ui->maxPCREMatchSpinBox->setVisible(true);
        }

        if (line.indexOf("--max-rechwp3") != -1) {
            ui->maxRecursionHWP3CheckBox->setVisible(true);
            ui->maxRecursionHWP3SpinBox->setVisible(true);
        }

        if (line.indexOf("--pcre-recmatch-limit") != -1) {
            ui->maxRecursionPCREMatchCheckBox->setVisible(true);
            ui->maxRecursionPCREMatchSpinBox->setVisible(true);
        }

        if (line.indexOf("--max-scantime") != -1) {
            ui->maxScanTimeCheckBox->setVisible(true);
            ui->maxScanTimeSpinBox->setVisible(true);
        }

        if (line.indexOf("--max-embeddedpe") != -1) {
            ui->maxSizeFileForPECheckBox->setVisible(true);
            ui->maxSizeFileForPEComboBox->setVisible(true);
            ui->maxSizeFileForPESpinBox->setVisible(true);
        }

        if (line.indexOf("--max-htmlnormalize") != -1) {
            ui->maxSizeHTMLFileToNormalizeCheckBox->setVisible(true);
            ui->maxSizeHTMLFileToNormalizeComboBox->setVisible(true);
            ui->maxSizeHTMLFileToNormalizeSpinBox->setVisible(true);
        }

        if (line.indexOf("--max-htmlnotags") != -1) {
            ui->maxSizeOfNormalizedHTMLFileCheckBox->setVisible(true);
            ui->maxSizeOfNormalizedHTMLFileComboBox->setVisible(true);
            ui->maxSizeOfNormalizedHTMLFileSpinBox->setVisible(true);
        }

        if (line.indexOf("--max-scriptnormalize") != -1) {
            ui->maxSizeOfScriptFileToNormalizeCheckBox->setVisible(true);
            ui->maxSizeOfScriptFileToNormalizeComboBox->setVisible(true);
            ui->maxSizeOfScriptFileToNormalizeSpinBox->setVisible(true);
        }

        if (line.indexOf("--max-scriptnormalize") != -1) {
            ui->maxSizeZipToTypeReanalzeCheckBox->setVisible(true);
            ui->maxSizeZipToTypeReanalzeComboBox->setVisible(true);
            ui->maxSizeZipToTypeReanalzeSpinBox->setVisible(true);
        }

        if (line.indexOf("--bytecode-timeout") != -1) {
            ui->numberOfSecondsToWaitForByteCodeCheckBox->setVisible(true);
            ui->numberOfSecondsToWaitForByteCodeSpinBox->setVisible(true);
        }

        if (line.indexOf("--stats-timeout") != -1) {
            ui->numberOfSecondsToWaitForResponseCheckBox->setVisible(true);
            ui->numberOfSecondsToWaitForResponseSpinBox->setVisible(true);
        }

        if (line.indexOf("--structured-cc-count") != -1) {
            ui->structuredCCCountCheckBox->setVisible(true);
            ui->structuredCCCountSpinBox->setVisible(true);
        }

        if (line.indexOf("--structured-cc-mode") != -1) {
            ui->structuredCCModeCheckBox->setVisible(true);
            ui->structuredCCModeComboBox->setVisible(true);
        }

        if (line.indexOf("--structured-ssn-count") != -1) {
            ui->structuredSSNCountCheckBox->setVisible(true);
            ui->structuredSSNCountSpinBox->setVisible(true);
        }

        if (line.indexOf("--structured-ssn-format") != -1) {
            ui->structuredSSNFormatCheckBox->setVisible(true);
            ui->structuredSSNFormatComboBox->setVisible(true);
        }

        if (line.indexOf("--statistics") != -1) {
            ui->collectAndPrintExecutionStatisticsCheckBox->setVisible(true);
            ui->collectAndPrintExecutionStatisticsComboBox->setVisible(true);
        }

        if (line.indexOf("--fail-if-cvd-older-than") != -1) {
            ui->databaseOutdatedCheckBox->setVisible(true);
            ui->databaseOutdatedSpinBox->setVisible(true);
        }
    }
}

void scanLimitsTab::updateLimits(){
QString value;
QString checked;

    value = setupFile->getSectionValue("ScanLimitations","Files larger than this will be skipped and assumed clean");
    checked = value.left(value.indexOf("|"));
    value = value.mid(value.indexOf("|") + 1);
    if ((value.right(1) == "K") || (value.right(1) == "M")){
        ui->filesLargerThanThisComboBox->setCurrentIndex(ui->filesLargerThanThisComboBox->findText(value.right(1)));
        value = value.left(value.length() - 1);
    }
    checked=="checked"?ui->filesLargerThanThisCheckBox->setChecked(true):ui->filesLargerThanThisCheckBox->setChecked(false);
    ui->filesLargerThanThisSpinBox->setValue(value.toInt());

    value = setupFile->getSectionValue("ScanLimitations","The maximum amount of data to scan for each container file");
    checked = value.left(value.indexOf("|"));
    value = value.mid(value.indexOf("|") + 1);
    if ((value.right(1) == "K") || (value.right(1) == "M")){
        ui->maxAmountForContainerComboBox->setCurrentIndex(ui->maxAmountForContainerComboBox->findText(value.right(1)));
        value = value.left(value.length() - 1);
    }
    checked=="checked"?ui->maxAmountForContainerCheckBox->setChecked(true):ui->maxAmountForContainerCheckBox->setChecked(false);
    ui->maxAmountForContainerSpinBox->setValue(value.toInt());

    value = setupFile->getSectionValue("ScanLimitations","The maximum number of files to scan for each container file");
    checked = value.left(value.indexOf("|"));
    value = value.mid(value.indexOf("|") + 1);
    if ((value.right(1) == "K") || (value.right(1) == "M")){
        ui->maxNumberForContainerComboBox->setCurrentIndex(ui->maxNumberForContainerComboBox->findText(value.right(1)));
        value = value.left(value.length() - 1);
    }
    checked=="checked"?ui->maxNumberForContainerCheckBox->setChecked(true):ui->maxNumberForContainerCheckBox->setChecked(false);
    ui->maxNumberForContainerSpinBox->setValue(value.toInt());

    value = setupFile->getSectionValue("ScanLimitations","Maximum archive recursion level for container file");
    checked = value.left(value.indexOf("|"));
    value = value.mid(value.indexOf("|") + 1);
    if ((value.right(1) == "K") || (value.right(1) == "M")){
        ui->maxArchiveRecursionForContainerComboBox->setCurrentIndex(ui->maxArchiveRecursionForContainerComboBox->findText(value.right(1)));
        value = value.left(value.length() - 1);
    }
    checked=="checked"?ui->maxArchiveRecursionForContainerCheckBox->setChecked(true):ui->maxArchiveRecursionForContainerCheckBox->setChecked(false);
    ui->maxArchiveRecursionForContainerSpinBox->setValue(value.toInt());

    value = setupFile->getSectionValue("ScanLimitations","Maximum directory recursion level");
    checked = value.left(value.indexOf("|"));
    value = value.mid(value.indexOf("|") + 1);
    if ((value.right(1) == "K") || (value.right(1) == "M")){
        ui->maxDirRecursionLevelComboBox->setCurrentIndex(ui->maxDirRecursionLevelComboBox->findText(value.right(1)));
        value = value.left(value.length() - 1);
    }
    checked=="checked"?ui->maxDirRecursionLevelCheckBox->setChecked(true):ui->maxDirRecursionLevelCheckBox->setChecked(false);
    ui->maxDirRecursionLevelSpinBox->setValue(value.toInt());

    value = setupFile->getSectionValue("ScanLimitations","Maximum size file to check for embedded PE");
    checked = value.left(value.indexOf("|"));
    value = value.mid(value.indexOf("|") + 1);
    if ((value.right(1) == "K") || (value.right(1) == "M")){
        ui->maxSizeFileForPEComboBox->setCurrentIndex(ui->maxSizeFileForPEComboBox->findText(value.right(1)));
        value = value.left(value.length() - 1);
    }
    checked=="checked"?ui->maxSizeFileForPECheckBox->setChecked(true):ui->maxSizeFileForPECheckBox->setChecked(false);
    ui->maxSizeFileForPESpinBox->setValue(value.toInt());

    value = setupFile->getSectionValue("ScanLimitations","Maximum size of HTML file to normalize");
    checked = value.left(value.indexOf("|"));
    value = value.mid(value.indexOf("|") + 1);
    if ((value.right(1) == "K") || (value.right(1) == "M")){
        ui->maxSizeHTMLFileToNormalizeComboBox->setCurrentIndex(ui->maxSizeHTMLFileToNormalizeComboBox->findText(value.right(1)));
        value = value.left(value.length() - 1);
    }
    checked=="checked"?ui->maxSizeHTMLFileToNormalizeCheckBox->setChecked(true):ui->maxSizeHTMLFileToNormalizeCheckBox->setChecked(false);
    ui->maxSizeHTMLFileToNormalizeSpinBox->setValue(value.toInt());

    value = setupFile->getSectionValue("ScanLimitations","Maximum size of normalized HTML file to scan");
    checked = value.left(value.indexOf("|"));
    value = value.mid(value.indexOf("|") + 1);
    if ((value.right(1) == "K") || (value.right(1) == "M")){
        ui->maxSizeOfNormalizedHTMLFileComboBox->setCurrentIndex(ui->maxSizeOfNormalizedHTMLFileComboBox->findText(value.right(1)));
        value = value.left(value.length() - 1);
    }
    checked=="checked"?ui->maxSizeOfNormalizedHTMLFileCheckBox->setChecked(true):ui->maxSizeOfNormalizedHTMLFileCheckBox->setChecked(false);
    ui->maxSizeOfNormalizedHTMLFileSpinBox->setValue(value.toInt());

    value = setupFile->getSectionValue("ScanLimitations","Maximum size of script file to normalize");
    checked = value.left(value.indexOf("|"));
    value = value.mid(value.indexOf("|") + 1);
    if ((value.right(1) == "K") || (value.right(1) == "M")){
        ui->maxSizeOfScriptFileToNormalizeComboBox->setCurrentIndex(ui->maxSizeOfScriptFileToNormalizeComboBox->findText(value.right(1)));
        value = value.left(value.length() - 1);
    }
    checked=="checked"?ui->maxSizeOfScriptFileToNormalizeCheckBox->setChecked(true):ui->maxSizeOfScriptFileToNormalizeCheckBox->setChecked(false);
    ui->maxSizeOfScriptFileToNormalizeSpinBox->setValue(value.toInt());

    value = setupFile->getSectionValue("ScanLimitations","Maximum size zip to type reanalyze");
    checked = value.left(value.indexOf("|"));
    value = value.mid(value.indexOf("|") + 1);
    if ((value.right(1) == "K") || (value.right(1) == "M")){
        ui->maxSizeZipToTypeReanalzeComboBox->setCurrentIndex(ui->maxSizeZipToTypeReanalzeComboBox->findText(value.right(1)));
        value = value.left(value.length() - 1);
    }
    checked=="checked"?ui->maxSizeZipToTypeReanalzeCheckBox->setChecked(true):ui->maxSizeZipToTypeReanalzeCheckBox->setChecked(false);
    ui->maxSizeZipToTypeReanalzeSpinBox->setValue(value.toInt());

    value = setupFile->getSectionValue("ScanLimitations","Maximum number of partitions in disk image to be scanned");
    checked = value.left(value.indexOf("|"));
    value = value.mid(value.indexOf("|") + 1);
    if ((value.right(1) == "K") || (value.right(1) == "M")){
        ui->maxNumberOfPartitionsInDiskImageComboBox->setCurrentIndex(ui->maxNumberOfPartitionsInDiskImageComboBox->findText(value.right(1)));
        value = value.left(value.length() - 1);
    }
    checked=="checked"?ui->maxNumberOfPartitionsInDiskImageCheckBox->setChecked(true):ui->maxNumberOfPartitionsInDiskImageCheckBox->setChecked(false);
    ui->maxNumberOfPartitionsInDiskImageSpinBox->setValue(value.toInt());

    value = setupFile->getSectionValue("ScanLimitations","Maximum number of icons in PE file to be scanned");
    checked = value.left(value.indexOf("|"));
    value = value.mid(value.indexOf("|") + 1);
    if ((value.right(1) == "K") || (value.right(1) == "M")){
        ui->maxNumberOfIconsInPEFileComboBox->setCurrentIndex(ui->maxNumberOfIconsInPEFileComboBox->findText(value.right(1)));
        value = value.left(value.length() - 1);
    }
    checked=="checked"?ui->maxNumberOfIconsInPEFileCheckBox->setChecked(true):ui->maxNumberOfIconsInPEFileCheckBox->setChecked(false);
    ui->maxNumberOfIconsInPEFileSpinBox->setValue(value.toInt());

    value = setupFile->getSectionValue("ScanLimitations","Number of seconds to wait for waiting a response back from the stats server");
    checked = value.left(value.indexOf("|"));
    value = value.mid(value.indexOf("|") + 1);
    checked=="checked"?ui->numberOfSecondsToWaitForResponseCheckBox->setChecked(true):ui->numberOfSecondsToWaitForResponseCheckBox->setChecked(false);
    ui->numberOfSecondsToWaitForResponseSpinBox->setValue(value.toInt());

    value = setupFile->getSectionValue("ScanLimitations","Bytecode timeout in milliseconds");
    checked = value.left(value.indexOf("|"));
    value = value.mid(value.indexOf("|") + 1);
    checked=="checked"?ui->numberOfSecondsToWaitForByteCodeCheckBox->setChecked(true):ui->numberOfSecondsToWaitForByteCodeCheckBox->setChecked(false);
    ui->numberOfSecondsToWaitForByteCodeSpinBox->setValue(value.toInt());

    value = setupFile->getSectionValue("ScanLimitations","Collect and print execution statistics");
    checked = value.left(value.indexOf("|"));
    value = value.mid(value.indexOf("|") + 1);
    checked=="checked"?ui->collectAndPrintExecutionStatisticsCheckBox->setChecked(true):ui->collectAndPrintExecutionStatisticsCheckBox->setChecked(false);
    ui->collectAndPrintExecutionStatisticsComboBox->setCurrentIndex(value.toInt());

    value = setupFile->getSectionValue("ScanLimitations","Structured SSN Format");
    checked = value.left(value.indexOf("|"));
    value = value.mid(value.indexOf("|") + 1);
    checked=="checked"?ui->structuredSSNFormatCheckBox->setChecked(true):ui->structuredSSNFormatCheckBox->setChecked(false);
    ui->structuredSSNFormatComboBox->setCurrentIndex(value.toInt());

    value = setupFile->getSectionValue("ScanLimitations","Structured SSN Count");
    checked = value.left(value.indexOf("|"));
    value = value.mid(value.indexOf("|") + 1);
    checked=="checked"?ui->structuredSSNCountCheckBox->setChecked(true):ui->structuredSSNCountCheckBox->setChecked(false);
    ui->structuredSSNCountSpinBox->setValue(value.toInt());

    value = setupFile->getSectionValue("ScanLimitations","Structured CC Count");
    checked = value.left(value.indexOf("|"));
    value = value.mid(value.indexOf("|") + 1);
    checked=="checked"?ui->structuredCCCountCheckBox->setChecked(true):ui->structuredCCCountCheckBox->setChecked(false);
    ui->structuredCCCountSpinBox->setValue(value.toInt());

    value = setupFile->getSectionValue("ScanLimitations","Structured CC Mode");
    checked = value.left(value.indexOf("|"));
    value = value.mid(value.indexOf("|") + 1);
    checked=="checked"?ui->structuredCCModeCheckBox->setChecked(true):ui->structuredCCModeCheckBox->setChecked(false);
    ui->structuredCCModeComboBox->setCurrentIndex(value.toInt());

    value = setupFile->getSectionValue("ScanLimitations","Max Scan-Time");
    checked = value.left(value.indexOf("|"));
    value = value.mid(value.indexOf("|") + 1);
    checked=="checked"?ui->maxScanTimeCheckBox->setChecked(true):ui->maxScanTimeCheckBox->setChecked(false);
    ui->maxScanTimeSpinBox->setValue(value.toInt());

    value = setupFile->getSectionValue("ScanLimitations","Max recursion to HWP3 parsing function");
    checked = value.left(value.indexOf("|"));
    value = value.mid(value.indexOf("|") + 1);
    checked=="checked"?ui->maxRecursionHWP3CheckBox->setChecked(true):ui->maxRecursionHWP3CheckBox->setChecked(false);
    ui->maxRecursionHWP3SpinBox->setValue(value.toInt());

    value = setupFile->getSectionValue("ScanLimitations","Max calls to PCRE match function");
    checked = value.left(value.indexOf("|"));
    value = value.mid(value.indexOf("|") + 1);
    checked=="checked"?ui->maxPCREMatchCheckBox->setChecked(true):ui->maxPCREMatchCheckBox->setChecked(false);
    ui->maxPCREMatchSpinBox->setValue(value.toInt());

    value = setupFile->getSectionValue("ScanLimitations","Max recursion calls to the PCRE match function");
    checked = value.left(value.indexOf("|"));
    value = value.mid(value.indexOf("|") + 1);
    checked=="checked"?ui->maxRecursionPCREMatchCheckBox->setChecked(true):ui->maxRecursionPCREMatchCheckBox->setChecked(false);
    ui->maxRecursionPCREMatchSpinBox->setValue(value.toInt());

    value = setupFile->getSectionValue("ScanLimitations","Max PCRE file size");
    checked = value.left(value.indexOf("|"));
    value = value.mid(value.indexOf("|") + 1);
    if ((value.right(1) == "K") || (value.right(1) == "M")){
        ui->maxPCREFileSizeComboBox->setCurrentIndex(ui->maxPCREFileSizeComboBox->findText(value.right(1)));
        value = value.left(value.length() - 1);
    }
    checked=="checked"?ui->maxPCREFileSizeCheckBox->setChecked(true):ui->maxPCREFileSizeCheckBox->setChecked(false);
    ui->maxPCREFileSizeSpinBox->setValue(value.toInt());

    value = setupFile->getSectionValue("ScanLimitations","Database outdated if older than x days");
    checked = value.left(value.indexOf("|"));
    value = value.mid(value.indexOf("|") + 1);
    checked=="checked"?ui->databaseOutdatedCheckBox->setChecked(true):ui->databaseOutdatedCheckBox->setChecked(false);
    ui->databaseOutdatedSpinBox->setValue(value.toInt());
}
