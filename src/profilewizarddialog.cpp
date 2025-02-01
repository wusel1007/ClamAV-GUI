#include "profilewizarddialog.h"
#include "ui_profilewizarddialog.h"

ProfileWizardDialog::ProfileWizardDialog(QWidget *parent, QString name) : QDialog(parent), profileName(name), ui(new Ui::ProfileWizardDialog)
{
    ui->setupUi(this);

    if (profileName == ""){
        setupFileFilename = QDir::homePath() + "/.clamav-gui/settings.ini";
        setupFile = new setupFileHandler(setupFileFilename);
        newProfile = true;
    } else {
        setupFileFilename = QDir::homePath() + "/.clamav-gui/profiles/" + profileName + ".ini";
        setupFile = new setupFileHandler(setupFileFilename);
        ui->profileNameLineEdit->setText(profileName);
        ui->profileNameLineEdit->setReadOnly(true);
        ui->createButton->setText(tr("Save"));
        newProfile = false;
    }


    model = new CFileSystemModel;
    model->setFilter(QDir::Dirs|QDir::NoDotAndDotDot );
    model->setRootPath("/");
    model->unCheckAll();

    ui->treeView->setModel(model);
    ui->treeView->hideColumn(1);
    ui->treeView->hideColumn(2);
    ui->treeView->hideColumn(3);
    ui->treeView->collapseAll();

    profileName==""?ui->stackedWidget->setCurrentIndex(0):ui->stackedWidget->setCurrentIndex(3);
    if (profileName != "") readSettings();

    slot_previousButtonClicked();

    getClamscanParametersProcess = new QProcess(this);
    connect(getClamscanParametersProcess,SIGNAL(readyReadStandardError()),this,SLOT(slot_getClamscanProcessHasOutput()));
    connect(getClamscanParametersProcess,SIGNAL(readyReadStandardOutput()),this,SLOT(slot_getClamscanProcessHasOutput()));
    connect(getClamscanParametersProcess,SIGNAL(finished(int)),this,SLOT(slot_getClamscanProcessFinished()));


    QStringList parameters;
    parameters << "--help";

    getClamscanParametersProcess->start("clamscan",parameters);
}

ProfileWizardDialog::~ProfileWizardDialog()
{
    delete ui;
}

void ProfileWizardDialog::closeEvent(QCloseEvent *) {
    QString profileFile = QDir::homePath()+"/.clamav-gui/profiles/" + ui->profileNameLineEdit->text()+".ini";
    if ((newProfile == true) && (profileFile != QDir::homePath()+"/.clamav-gui/settings.ini")){
        QFile killfile(profileFile);
        if (killfile.exists() == true) killfile.remove();
    }
}

void ProfileWizardDialog::readSettings(){
QString section = "REGEXP_and_IncludeExclude";
QString keyword;
QString value;
QString checked;
QString optionText;
QString tooltipText;
QStringList directories = setupFile->getSectionValue(ui->profileNameLineEdit->text(),"Directories").split("\n");
QStringList availableOptions = setupFile->getKeywords("AvailableOptions");

// DIRTREE
    ui->treeView->collapseAll();
    ui->recursivCheckBox->setChecked(setupFile->getSectionBoolValue(ui->profileNameLineEdit->text(),"Recursion"));
    model->unCheckAll();
    foreach(QString dir,directories){
        if (dir != ""){
            model->setChecked(dir,true);
            QModelIndex index = model->index(dir);
            ui->treeView->scrollTo(index);
            ui->treeView->setCurrentIndex(index);
        }
    }

// DIRECTORIES
    value=setupFile->getSectionValue("Directories","LoadSupportedDBFiles");
    checked = value.left(value.indexOf("|"));
    value = value.mid(value.indexOf("|") + 1);
    checked=="checked"?ui->loadVirusDatabaseCheckBox->setChecked(true):ui->loadVirusDatabaseCheckBox->setChecked(false);
    ui->loadVirusDatabaseLineEdit->setText(value);

    value=setupFile->getSectionValue("Directories","ScanReportToFile");
    checked = value.left(value.indexOf("|"));
    value = value.mid(value.indexOf("|") + 1);
    checked=="checked"?ui->scanReportToFileCheckBox->setChecked(true):ui->scanReportToFileCheckBox->setChecked(false);
    ui->scanReportToFileLineEdit->setText(value);

    value=setupFile->getSectionValue("Directories","ScanFilesFromFile");
    checked = value.left(value.indexOf("|"));
    value = value.mid(value.indexOf("|") + 1);
    checked=="checked"?ui->scanFilesFromFileCheckBox->setChecked(true):ui->scanFilesFromFileCheckBox->setChecked(false);
    ui->scanFilesFromFileLineEdit->setText(value);

    value=setupFile->getSectionValue("Directories","TmpFile");
    checked = value.left(value.indexOf("|"));
    value = value.mid(value.indexOf("|") + 1);
    checked=="checked"?ui->tempFileCheckBox->setChecked(true):ui->tempFileCheckBox->setChecked(false);
    ui->tempFilesLineEdit->setText(value);

    value=setupFile->getSectionValue("Directories","MoveInfectedFiles");
    checked = value.left(value.indexOf("|"));
    value = value.mid(value.indexOf("|") + 1);
    checked=="checked"?ui->moveDirectoryCheckBox->setChecked(true):ui->moveDirectoryCheckBox->setChecked(false);
    ui->moveDirectoryLineEdit->setText(value);

    value=setupFile->getSectionValue("Directories","CopyInfectedFiles");
    checked = value.left(value.indexOf("|"));
    value = value.mid(value.indexOf("|") + 1);
    checked=="checked"?ui->copyDirectoryCheckBox->setChecked(true):ui->copyDirectoryCheckBox->setChecked(false);
    ui->copyDirectoryLineEdit->setText(value);

    value=setupFile->getSectionValue("Directories","FollowDirectorySymLinks");
    checked = value.left(value.indexOf("|"));
    value = value.mid(value.indexOf("|") + 1);
    checked=="checked"?ui->followDirectorySymlinksCheckBox->setChecked(true):ui->followDirectorySymlinksCheckBox->setChecked(false);
    ui->followDirectorySymlinksComboBox->setCurrentIndex(value.toInt());

    value=setupFile->getSectionValue("Directories","FollowFileSymLinks");
    checked = value.left(value.indexOf("|"));
    value = value.mid(value.indexOf("|") + 1);
    checked=="checked"?ui->followFileSymlinksCheckBox->setChecked(true):ui->followFileSymlinksCheckBox->setChecked(false);
    ui->followFileSymlinksComboBox->setCurrentIndex(value.toInt());

// INCLUDE / EXCLUDE
    keyword = "DontScanFileNamesMatchingRegExp";
    value = setupFile->getSectionValue(section,keyword);
    checked = value.left(value.indexOf("|"));
    value = value.mid(value.indexOf("|") + 1);
    checked=="checked"?ui->pwdontScanFileNameCheckBox->setChecked(true):ui->pwdontScanFileNameCheckBox->setChecked(false);
    ui->pwdontScanFileNameLineEdit->setText(value);

    keyword = "DontScanDiretoriesMatchingRegExp";
    value = setupFile->getSectionValue(section,keyword);
    checked = value.left(value.indexOf("|"));
    value = value.mid(value.indexOf("|") + 1);
    checked=="checked"?ui->pwdontScanDirCheckBox->setChecked(true):ui->pwdontScanDirCheckBox->setChecked(false);
    ui->pwdontScanDirLineEdit->setText(value);

    keyword = "OnlyScanFileNamesMatchingRegExp";
    value = setupFile->getSectionValue(section,keyword);
    checked = value.left(value.indexOf("|"));
    value = value.mid(value.indexOf("|") + 1);
    checked=="checked"?ui->pwonlyScanFileNameCheckBox->setChecked(true):ui->pwonlyScanFileNameCheckBox->setChecked(false);
    ui->pwonlyScanFileNameLineEdit->setText(value);

    keyword = "OnlyScanDiretoriesMatchingRegExp";
    value = setupFile->getSectionValue(section,keyword);
    checked = value.left(value.indexOf("|"));
    value = value.mid(value.indexOf("|") + 1);
    checked=="checked"?ui->pwonlyScanDirCheckBox->setChecked(true):ui->pwonlyScanDirCheckBox->setChecked(false);
    ui->pwonlyScanDirLineEdit->setText(value);

// PUA
    keyword = "EnablePUAOptions";
    if (setupFile->getSectionBoolValue(section,keyword) == true){
        ui->pwenablePUACheckBox->setChecked(true);
        ui->pwPUAFrame->setEnabled(true);
    } else {
        ui->pwenablePUACheckBox->setChecked(false);
        ui->pwPUAFrame->setEnabled(false);
    }

    keyword = "LoadPUAPacked";
    ui->pwloadPUAPackedRadioButon->setChecked(setupFile->getSectionBoolValue(section,keyword));

    keyword = "LoadPUAPWTool";
    ui->pwloadPUAPWToolRadioButton->setChecked(setupFile->getSectionBoolValue(section,keyword));

    keyword = "LoadPUANetTool";
    ui->pwloadPUANetToolRadioButton->setChecked(setupFile->getSectionBoolValue(section,keyword));

    keyword = "LoadPUAP2P";
    ui->pwloadPUAP2PRadioButton->setChecked(setupFile->getSectionBoolValue(section,keyword));

    keyword = "LoadPUAIRC";
    ui->pwloadPUAIRCRadioButton->setChecked(setupFile->getSectionBoolValue(section,keyword));

    keyword = "LoadPUARAT";
    ui->pwloadPUARATRadioButton->setChecked(setupFile->getSectionBoolValue(section,keyword));

    keyword = "LoadPUANetToolSpy";
    ui->pwloadPUANetToolSpyRadioButton->setChecked(setupFile->getSectionBoolValue(section,keyword));

    keyword = "LoadPUAServer";
    ui->pwloadPUAServerRadioButton->setChecked(setupFile->getSectionBoolValue(section,keyword));

    keyword = "LoadPUAScript";
    ui->pwloadPUAScriptRadioButton->setChecked(setupFile->getSectionBoolValue(section,keyword));

    keyword = "LoadPUAAndr";
    ui->pwloadPUAAndrRadioButton->setChecked(setupFile->getSectionBoolValue(section,keyword));

    keyword = "LoadPUAJava";
    ui->pwloadPUAJavaRadioButton->setChecked(setupFile->getSectionBoolValue(section,keyword));

    keyword = "LoadPUAOsx";
    ui->pwloadPUAOsxRadioButton->setChecked(setupFile->getSectionBoolValue(section,keyword));

    keyword = "LoadPUATool";
    ui->pwloadPUAToolRadioButton->setChecked(setupFile->getSectionBoolValue(section,keyword));

    keyword = "LoadPUAUnix";
    ui->pwloadPUAUnixRadioButton->setChecked(setupFile->getSectionBoolValue(section,keyword));

    keyword = "LoadPUAWin";
    ui->pwloadPUAWinRadioButton->setChecked(setupFile->getSectionBoolValue(section,keyword));

// SCANLIMITS
    value = setupFile->getSectionValue("ScanLimitations","Files larger than this will be skipped and assumed clean");
    checked = value.left(value.indexOf("|"));
    value = value.mid(value.indexOf("|") + 1);
    if ((value.right(1) == "K") || (value.right(1) == "M")){
        ui->pwfilesLargerThanThisComboBox->setCurrentIndex(ui->pwfilesLargerThanThisComboBox->findText(value.right(1)));
        value = value.left(value.length() - 1);
    } else {
        ui->pwfilesLargerThanThisComboBox->setCurrentIndex(0);
    }
    checked=="checked"?ui->pwfilesLargerThanThisCheckBox->setChecked(true):ui->pwfilesLargerThanThisCheckBox->setChecked(false);
    ui->pwfilesLargerThanThisSpinBox->setValue(value.toInt());

    value = setupFile->getSectionValue("ScanLimitations","The maximum amount of data to scan for each container file");
    checked = value.left(value.indexOf("|"));
    value = value.mid(value.indexOf("|") + 1);
    if ((value.right(1) == "K") || (value.right(1) == "M")){
        ui->pwmaxAmountForContainerComboBox->setCurrentIndex(ui->pwmaxAmountForContainerComboBox->findText(value.right(1)));
        value = value.left(value.length() - 1);
    } else {
       ui->pwmaxAmountForContainerComboBox->setCurrentIndex(0);
    }
    checked=="checked"?ui->pwmaxAmountForContainerCheckBox->setChecked(true):ui->pwmaxAmountForContainerCheckBox->setChecked(false);
    ui->pwmaxAmountForContainerSpinBox->setValue(value.toInt());

    value = setupFile->getSectionValue("ScanLimitations","The maximum number of files to scan for each container file");
    checked = value.left(value.indexOf("|"));
    value = value.mid(value.indexOf("|") + 1);
    if ((value.right(1) == "K") || (value.right(1) == "M")){
        ui->pwmaxNumberForContainerComboBox->setCurrentIndex(ui->pwmaxNumberForContainerComboBox->findText(value.right(1)));
        value = value.left(value.length() - 1);
    } else {
        ui->pwmaxNumberForContainerComboBox->setCurrentIndex(0);
    }
    checked=="checked"?ui->pwmaxNumberForContainerCheckBox->setChecked(true):ui->pwmaxNumberForContainerCheckBox->setChecked(false);
    ui->pwmaxNumberForContainerSpinBox->setValue(value.toInt());

    value = setupFile->getSectionValue("ScanLimitations","Maximum archive recursion level for container file");
    checked = value.left(value.indexOf("|"));
    value = value.mid(value.indexOf("|") + 1);
    if ((value.right(1) == "K") || (value.right(1) == "M")){
        ui->pwmaxArchiveRecursionForContainerComboBox->setCurrentIndex(ui->pwmaxArchiveRecursionForContainerComboBox->findText(value.right(1)));
        value = value.left(value.length() - 1);
    } else {
        ui->pwmaxArchiveRecursionForContainerComboBox->setCurrentIndex(0);
    }
    checked=="checked"?ui->pwmaxArchiveRecursionForContainerCheckBox->setChecked(true):ui->pwmaxArchiveRecursionForContainerCheckBox->setChecked(false);
    ui->pwmaxArchiveRecursionForContainerSpinBox->setValue(value.toInt());

    value = setupFile->getSectionValue("ScanLimitations","Maximum directory recursion level");
    checked = value.left(value.indexOf("|"));
    value = value.mid(value.indexOf("|") + 1);
    if ((value.right(1) == "K") || (value.right(1) == "M")){
        ui->pwmaxDirRecursionLevelComboBox->setCurrentIndex(ui->pwmaxDirRecursionLevelComboBox->findText(value.right(1)));
        value = value.left(value.length() - 1);
    } else {
        ui->pwmaxDirRecursionLevelComboBox->setCurrentIndex(0);
    }
    checked=="checked"?ui->pwmaxDirRecursionLevelCheckBox->setChecked(true):ui->pwmaxDirRecursionLevelCheckBox->setChecked(false);
    ui->pwmaxDirRecursionLevelSpinBox->setValue(value.toInt());

    value = setupFile->getSectionValue("ScanLimitations","Maximum size file to check for embedded PE");
    checked = value.left(value.indexOf("|"));
    value = value.mid(value.indexOf("|") + 1);
    if ((value.right(1) == "K") || (value.right(1) == "M")){
        ui->pwmaxSizeFileForPEComboBox->setCurrentIndex(ui->pwmaxSizeFileForPEComboBox->findText(value.right(1)));
        value = value.left(value.length() - 1);
    } else {
        ui->pwmaxSizeFileForPEComboBox->setCurrentIndex(0);
    }
    checked=="checked"?ui->pwmaxSizeFileForPECheckBox->setChecked(true):ui->pwmaxSizeFileForPECheckBox->setChecked(false);
    ui->pwmaxSizeFileForPESpinBox->setValue(value.toInt());

    value = setupFile->getSectionValue("ScanLimitations","Maximum size of HTML file to normalize");
    checked = value.left(value.indexOf("|"));
    value = value.mid(value.indexOf("|") + 1);
    if ((value.right(1) == "K") || (value.right(1) == "M")){
        ui->pwmaxSizeHTMLFileToNormalizeComboBox->setCurrentIndex(ui->pwmaxSizeHTMLFileToNormalizeComboBox->findText(value.right(1)));
        value = value.left(value.length() - 1);
    } else {
        ui->pwmaxSizeHTMLFileToNormalizeComboBox->setCurrentIndex(0);
    }
    checked=="checked"?ui->pwmaxSizeHTMLFileToNormalizeCheckBox->setChecked(true):ui->pwmaxSizeHTMLFileToNormalizeCheckBox->setChecked(false);
    ui->pwmaxSizeHTMLFileToNormalizeSpinBox->setValue(value.toInt());

    value = setupFile->getSectionValue("ScanLimitations","Maximum size of normalized HTML file to scan");
    checked = value.left(value.indexOf("|"));
    value = value.mid(value.indexOf("|") + 1);
    if ((value.right(1) == "K") || (value.right(1) == "M")){
        ui->pwmaxSizeOfNormalizedHTMLFileComboBox->setCurrentIndex(ui->pwmaxSizeOfNormalizedHTMLFileComboBox->findText(value.right(1)));
        value = value.left(value.length() - 1);
    } else {
        ui->pwmaxSizeOfNormalizedHTMLFileComboBox->setCurrentIndex(0);
    }
    checked=="checked"?ui->pwmaxSizeOfNormalizedHTMLFileCheckBox->setChecked(true):ui->pwmaxSizeOfNormalizedHTMLFileCheckBox->setChecked(false);
    ui->pwmaxSizeOfNormalizedHTMLFileSpinBox->setValue(value.toInt());

    value = setupFile->getSectionValue("ScanLimitations","Maximum size of script file to normalize");
    checked = value.left(value.indexOf("|"));
    value = value.mid(value.indexOf("|") + 1);
    if ((value.right(1) == "K") || (value.right(1) == "M")){
        ui->pwmaxSizeOfScriptFileToNormalizeComboBox->setCurrentIndex(ui->pwmaxSizeOfScriptFileToNormalizeComboBox->findText(value.right(1)));
        value = value.left(value.length() - 1);
    } else {
        ui->pwmaxSizeOfScriptFileToNormalizeComboBox->setCurrentIndex(0);
    }
    checked=="checked"?ui->pwmaxSizeOfScriptFileToNormalizeCheckBox->setChecked(true):ui->pwmaxSizeOfScriptFileToNormalizeCheckBox->setChecked(false);
    ui->pwmaxSizeOfScriptFileToNormalizeSpinBox->setValue(value.toInt());

    value = setupFile->getSectionValue("ScanLimitations","Maximum size zip to type reanalyze");
    checked = value.left(value.indexOf("|"));
    value = value.mid(value.indexOf("|") + 1);
    if ((value.right(1) == "K") || (value.right(1) == "M")){
        ui->pwmaxSizeZipToTypeReanalzeComboBox->setCurrentIndex(ui->pwmaxSizeZipToTypeReanalzeComboBox->findText(value.right(1)));
        value = value.left(value.length() - 1);
    } else {
        ui->pwmaxSizeZipToTypeReanalzeComboBox->setCurrentIndex(0);
    }
    checked=="checked"?ui->pwmaxSizeZipToTypeReanalzeCheckBox->setChecked(true):ui->pwmaxSizeZipToTypeReanalzeCheckBox->setChecked(false);
    ui->pwmaxSizeZipToTypeReanalzeSpinBox->setValue(value.toInt());

    value = setupFile->getSectionValue("ScanLimitations","Maximum number of partitions in disk image to be scanned");
    checked = value.left(value.indexOf("|"));
    value = value.mid(value.indexOf("|") + 1);
    if ((value.right(1) == "K") || (value.right(1) == "M")){
        ui->pwmaxNumberOfPartitionsInDiskImageComboBox->setCurrentIndex(ui->pwmaxNumberOfPartitionsInDiskImageComboBox->findText(value.right(1)));
        value = value.left(value.length() - 1);
    } else {
        ui->pwmaxNumberOfPartitionsInDiskImageComboBox->setCurrentIndex(0);
    }
    checked=="checked"?ui->pwmaxNumberOfPartitionsInDiskImageCheckBox->setChecked(true):ui->pwmaxNumberOfPartitionsInDiskImageCheckBox->setChecked(false);
    ui->pwmaxNumberOfPartitionsInDiskImageSpinBox->setValue(value.toInt());

    value = setupFile->getSectionValue("ScanLimitations","Maximum number of icons in PE file to be scanned");
    checked = value.left(value.indexOf("|"));
    value = value.mid(value.indexOf("|") + 1);
    if ((value.right(1) == "K") || (value.right(1) == "M")){
        ui->pwmaxNumberOfIconsInPEFileComboBox->setCurrentIndex(ui->pwmaxNumberOfIconsInPEFileComboBox->findText(value.right(1)));
        value = value.left(value.length() - 1);
    } else {
        ui->pwmaxNumberOfIconsInPEFileComboBox->setCurrentIndex(0);
    }
    checked=="checked"?ui->pwmaxNumberOfIconsInPEFileCheckBox->setChecked(true):ui->pwmaxNumberOfIconsInPEFileCheckBox->setChecked(false);
    ui->pwmaxNumberOfIconsInPEFileSpinBox->setValue(value.toInt());

    value = setupFile->getSectionValue("ScanLimitations","Number of seconds to wait for waiting a response back from the stats server");
    checked = value.left(value.indexOf("|"));
    value = value.mid(value.indexOf("|") + 1);
    checked=="checked"?ui->pwnumberOfSecondsToWaitForResponseCheckBox->setChecked(true):ui->pwnumberOfSecondsToWaitForResponseCheckBox->setChecked(false);
    ui->pwnumberOfSecondsToWaitForResponseSpinBox->setValue(value.toInt());

    value = setupFile->getSectionValue("ScanLimitations","Bytecode timeout in milliseconds");
    checked = value.left(value.indexOf("|"));
    value = value.mid(value.indexOf("|") + 1);
    checked=="checked"?ui->pwBytecodeTimeoutCheckBox->setChecked(true):ui->pwBytecodeTimeoutCheckBox->setChecked(false);
    ui->pwBytecodeTimeoutSpinBox->setValue(value.toInt());

    value = setupFile->getSectionValue("ScanLimitations","Collect and print execution statistics");
    checked = value.left(value.indexOf("|"));
    value = value.mid(value.indexOf("|") + 1);
    checked=="checked"?ui->pwExecutionStatisticsCheckBox->setChecked(true):ui->pwExecutionStatisticsCheckBox->setChecked(false);
    ui->pwExecutionStatisticsComboBox->setCurrentIndex(value.toInt());

    value = setupFile->getSectionValue("ScanLimitations","Structured SSN Format");
    checked = value.left(value.indexOf("|"));
    value = value.mid(value.indexOf("|") + 1);
    checked=="checked"?ui->pwstructuredSSNFormatCheckBox->setChecked(true):ui->pwstructuredSSNFormatCheckBox->setChecked(false);
    ui->pwStructuredSSNFormatComboBox->setCurrentIndex(value.toInt());

    value = setupFile->getSectionValue("ScanLimitations","Structured SSN Count");
    checked = value.left(value.indexOf("|"));
    value = value.mid(value.indexOf("|") + 1);
    checked=="checked"?ui->pwStructuredSSNCountCheckBox->setChecked(true):ui->pwStructuredSSNCountCheckBox->setChecked(false);
    ui->pwStructuredSSNCountSpinBox->setValue(value.toInt());

    value = setupFile->getSectionValue("ScanLimitations","Structured CC Count");
    checked = value.left(value.indexOf("|"));
    value = value.mid(value.indexOf("|") + 1);
    checked=="checked"?ui->pwStructuredCCCountCheckBox->setChecked(true):ui->pwStructuredCCCountCheckBox->setChecked(false);
    ui->pwStructuredCCCountSpinBox->setValue(value.toInt());

    value = setupFile->getSectionValue("ScanLimitations","Structured CC Mode");
    checked = value.left(value.indexOf("|"));
    value = value.mid(value.indexOf("|") + 1);
    checked=="checked"?ui->pwStructuredCCModeCheckBox->setChecked(true):ui->pwStructuredCCModeCheckBox->setChecked(false);
    ui->pwStructuredCCModeComboBox->setCurrentIndex(value.toInt());

    value = setupFile->getSectionValue("ScanLimitations","Max Scan-Time");
    checked = value.left(value.indexOf("|"));
    value = value.mid(value.indexOf("|") + 1);
    checked=="checked"?ui->pwMaxScanTimeCheckBox->setChecked(true):ui->pwMaxScanTimeCheckBox->setChecked(false);
    ui->pwMaxScanTimeSpinBox->setValue(value.toInt());

    value = setupFile->getSectionValue("ScanLimitations","Max recursion to HWP3 parsing function");
    checked = value.left(value.indexOf("|"));
    value = value.mid(value.indexOf("|") + 1);
    checked=="checked"?ui->pwMaxRecursionHWP3CheckBox->setChecked(true):ui->pwMaxRecursionHWP3CheckBox->setChecked(false);
    ui->pwMaxRecursionHWP3SpinBox->setValue(value.toInt());

    value = setupFile->getSectionValue("ScanLimitations","Max calls to PCRE match function");
    checked = value.left(value.indexOf("|"));
    value = value.mid(value.indexOf("|") + 1);
    checked=="checked"?ui->pwMaxCallsPCREMatchFunctionCheckBox->setChecked(true):ui->pwMaxCallsPCREMatchFunctionCheckBox->setChecked(false);
    ui->pwMaxCallsPCREMatchFunctionSpinBox->setValue(value.toInt());

    value = setupFile->getSectionValue("ScanLimitations","Max recursion calls to the PCRE match function");
    checked = value.left(value.indexOf("|"));
    value = value.mid(value.indexOf("|") + 1);
    checked=="checked"?ui->pwMaxRecursionCallsPCREMatchFunctionCheckBox->setChecked(true):ui->pwMaxRecursionCallsPCREMatchFunctionCheckBox->setChecked(false);
    ui->pwMaxRecursionCallsPCREMatchFunctionCpinBox->setValue(value.toInt());

    value = setupFile->getSectionValue("ScanLimitations","Max PCRE file size");
    checked = value.left(value.indexOf("|"));
    value = value.mid(value.indexOf("|") + 1);
    if ((value.right(1) == "K") || (value.right(1) == "M")){
        ui->pwMaxPCREFileSizeComboBox->setCurrentIndex(ui->pwMaxPCREFileSizeComboBox->findText(value.right(1)));
        value = value.left(value.length() - 1);
    }
    checked=="checked"?ui->pwMaxPCREFileSizeCheckBox->setChecked(true):ui->pwMaxPCREFileSizeCheckBox->setChecked(false);
    ui->pwMaxPCREFileSizeSpinBox->setValue(value.toInt());

    value = setupFile->getSectionValue("ScanLimitations","Database outdated if older than x days");
    checked = value.left(value.indexOf("|"));
    value = value.mid(value.indexOf("|") + 1);
    checked=="checked"?ui->pwdatabaseOutdatedCheckbox->setChecked(true):ui->pwdatabaseOutdatedCheckbox->setChecked(false);
    ui->pwdatabaseOutdatedSpinBox->setValue(value.toInt());

// OPTIONS

    QString lastOption = "xyz";
    scanoption * option;
    scanoptionyn * optionyn;
    QString newFileFilename = QDir::homePath() + "/.clamav-gui/profiles/" + profileName + ".ini";
    setupFileHandler * tempconfig = new setupFileHandler(newFileFilename);

    if (ui->optionLayout->count() == 0) {
        for (int i = 0; i < availableOptions.count(); i++){
            optionText = availableOptions.at(i);
            tooltipText = setupFile->getSectionValue("AvailableOptions",optionText);
            tempconfig->setSectionValue("AvailableOptions",optionText,tooltipText);
            if (optionText.indexOf(lastOption) == -1) {
                if (optionText.indexOf("<equal>") != -1) lastOption = optionText.left(optionText.indexOf("<equal>")); else lastOption = optionText;
                if (optionText.indexOf("<equal>") != -1) {
                    // --Switches with yes/no option
                    QString keyword = optionText.left(optionText.indexOf("<equal>"));
                    QString yes_no = optionText.mid(optionText.indexOf("<equal>") + 7);
                    bool optionfound = false;
                    if (setupFile->keywordExists("SelectedOptions",keyword+"<equal>yes") == true) {
                        optionyn = new scanoptionyn(nullptr,newFileFilename,"SelectedOptions",true,keyword + "<equal>yes",setupFile->getSectionValue("AvailableOptions",optionText));
                        optionfound = true;
                    }
                    if ((setupFile->keywordExists("SelectedOptions",keyword+"<equal>no") == true) && (optionfound == false)) {
                        optionyn = new scanoptionyn(nullptr,newFileFilename,"SelectedOptions",true,keyword + "<equal>no",setupFile->getSectionValue("AvailableOptions",optionText));
                        optionfound = true;
                    }
                    if (optionfound == false) {
                        optionyn = new scanoptionyn(nullptr,newFileFilename,"SelectedOptions",false,keyword+"<equal>" + yes_no,setupFile->getSectionValue("AvailableOptions",optionText));
                    }
                    ui->optionLayout->addWidget(optionyn);
                } else {
                    // --Switches without yes/no
                    if (setupFile->keywordExists("SelectedOptions",optionText) == true) {
                        option = new scanoption(nullptr,newFileFilename,"SelectedOptions",true,optionText,setupFile->getSectionValue("AvailableOptions",optionText));
                    } else {
                        option = new scanoption(nullptr,newFileFilename,"SelectedOptions",false,optionText,setupFile->getSectionValue("AvailableOptions",optionText));
                    }
                    ui->optionLayout->addWidget(option);
                }
            }
        }
    }
}

void ProfileWizardDialog::slot_nextButtonClicked(){
int index = ui->stackedWidget->currentIndex();
int count = ui->stackedWidget->count() - 1;

    if (index < count) {
        index++;
        ui->stackedWidget->setCurrentIndex(index);
    }

    if ((ui->profileNameLineEdit->text() != "") && (ui->optionLayout->count() == 0)){
        profileName = ui->profileNameLineEdit->text();
        readSettings();
    }

    slot_checkNavigationButtons();
}

void ProfileWizardDialog::slot_previousButtonClicked(){
int index = ui->stackedWidget->currentIndex();

    if (index > 0){
        index--;
        ui->stackedWidget->setCurrentIndex(index);
    }
    slot_checkNavigationButtons();
}

void ProfileWizardDialog::slot_checkNavigationButtons(){
int index = ui->stackedWidget->currentIndex();

QFile checkFile(QDir::homePath() + "/.clamav-gui/profiles/" + ui->profileNameLineEdit->text() + ".ini");

    switch (index){
    case 0  :   ui->previousButton->setEnabled(false);
                    break;
    case 1  :   ui->previousButton->setEnabled(true);
                if ((ui->profileNameLineEdit->text() == "") | (checkFile.exists())) ui->nextButton->setEnabled(false); else ui->nextButton->setEnabled(true);
                if (profileName != "") ui->nextButton->setEnabled(true);
                    break;
    case 2  :   slot_listChanged();
                if (profileName != "") ui->previousButton->setEnabled(false);
                    break;
    default :   ui->previousButton->setEnabled(true);
                if (index == ui->stackedWidget->count() - 1) ui->nextButton->setEnabled(false); else ui->nextButton->setEnabled(true);
                if (index == ui->stackedWidget->count() - 1) ui->createButton->setEnabled(true); else ui->createButton->setEnabled(false);
                    break;

    }
}

void ProfileWizardDialog::slot_enablePUACheckBoxActivated(){
    ui->pwenablePUACheckBox->isChecked()?ui->pwPUAFrame->setEnabled(true):ui->pwPUAFrame->setEnabled(false);
}

void ProfileWizardDialog::slot_homeButtonClicked(){
QModelIndex index = model->index(QDir::homePath());

    ui->treeView->scrollTo(index);
    ui->treeView->setCurrentIndex(index);
}

void ProfileWizardDialog::slot_rootButtonClicked(){
QModelIndex index = model->index("/");

    ui->treeView->scrollTo(index);
    ui->treeView->setCurrentIndex(index);
}

void ProfileWizardDialog::slot_listChanged(){
QList <QPersistentModelIndex> list = model->checkedIndexes.values();

    list.count()==0?ui->nextButton->setEnabled(false):ui->nextButton->setEnabled(true);
}

void ProfileWizardDialog::slot_loadAllSupportedDBButtonClicked(){
QString folder = QFileDialog::getExistingDirectory(this,tr("Signature Folder"),ui->loadVirusDatabaseLineEdit->text());
    if (folder != "") ui->loadVirusDatabaseLineEdit->setText(folder);
}

void ProfileWizardDialog::slot_saveScanReportToFileButtonClicked(){
QString filename = QFileDialog::getSaveFileName(this,tr("Log-File"),ui->scanReportToFileLineEdit->text());
    if (filename != "") ui->scanReportToFileLineEdit->setText(filename);
}

void ProfileWizardDialog::slot_scanFromFileButtonClicked(){
QString filename = QFileDialog::getOpenFileName(this,tr("Scan from File"),ui->scanFilesFromFileLineEdit->text());
    if (filename != "") ui->scanFilesFromFileLineEdit->setText(filename);
}

void ProfileWizardDialog::slot_createTempFilesButtonClicked(){
QString folder = QFileDialog::getExistingDirectory(this,tr("Folder for temporary files"),ui->tempFilesLineEdit->text());
    if (folder != "") ui->tempFilesLineEdit->setText(folder);
}

void ProfileWizardDialog::slot_moveInfectedFilesButtonClicked(){
QString folder = QFileDialog::getExistingDirectory(this,tr("Move Folder for infected Files"),ui->moveDirectoryLineEdit->text());
    if (folder != "") ui->moveDirectoryLineEdit->setText(folder);
}

void ProfileWizardDialog::slot_copyInfectedFilesButtonClicked(){
QString folder = QFileDialog::getExistingDirectory(this,tr("Copy Folder for infected Files"),ui->copyDirectoryLineEdit->text());
    if (folder != "") ui->copyDirectoryLineEdit->setText(folder);
}

void ProfileWizardDialog::slot_createButtonClicked(){
QList <QPersistentModelIndex> list = model->checkedIndexes.values();
QString directories;
QString profileName = ui->profileNameLineEdit->text();
setupFileHandler * profiles = new setupFileHandler(QDir::homePath() + "/.clamav-gui/profiles/" + profileName + ".ini");
QString section = "REGEXP_and_IncludeExclude";
QString checked;
QString keyword;
QString value;
QString optionString;
QString tooltipString;

    newProfile = false;
    for (int i = 0; i < list.count(); i++){
        if (i < list.count() - 1){
            directories = directories + list[i].data(QFileSystemModel::FilePathRole).toString() + "\n";
        } else {
            directories = directories + list[i].data(QFileSystemModel::FilePathRole).toString();
        }
    }

    profiles->setSectionValue(profileName,"Directories",directories);
    profiles->setSectionValue(profileName,"Recursion",ui->recursivCheckBox->isChecked());
    setupFile->setSectionValue("Profiles",profileName,profileName);

    keyword = "LoadSupportedDBFiles";
    value = ui->loadVirusDatabaseLineEdit->text();
    ui->loadVirusDatabaseCheckBox->isChecked() == true?checked="checked":checked="not checked";
    profiles->setSectionValue("Directories",keyword,checked + "|" + value);

    keyword = "ScanReportToFile";
    value = ui->scanReportToFileLineEdit->text();
    ui->scanReportToFileCheckBox->isChecked() == true?checked="checked":checked="not checked";
    profiles->setSectionValue("Directories",keyword,checked + "|" + value);

    keyword = "ScanFilesFromFile";
    value = ui->scanFilesFromFileLineEdit->text();
    ui->scanFilesFromFileCheckBox->isChecked() == true?checked="checked":checked="not checked";
    profiles->setSectionValue("Directories",keyword,checked + "|" + value);

    keyword = "TmpFile";
    value = ui->tempFilesLineEdit->text();
    ui->tempFileCheckBox->isChecked() == true?checked="checked":checked="not checked";
    profiles->setSectionValue("Directories",keyword,checked + "|" + value);

    keyword = "MoveInfectedFiles";
    value = ui->moveDirectoryLineEdit->text();
    ui->moveDirectoryCheckBox->isChecked() == true?checked="checked":checked="not checked";
    profiles->setSectionValue("Directories",keyword,checked + "|" + value);

    keyword = "CopyInfectedFiles";
    value = ui->copyDirectoryLineEdit->text();
    ui->copyDirectoryCheckBox->isChecked() == true?checked="checked":checked="not checked";
    profiles->setSectionValue("Directories",keyword,checked + "|" + value);

    keyword = "FollowDirectorySymLinks";
    value = QString::number(ui->followDirectorySymlinksComboBox->currentIndex());
    ui->followDirectorySymlinksCheckBox->isChecked() == true?checked="checked":checked="not checked";
    profiles->setSectionValue("Directories",keyword,checked + "|" + value);

    keyword = "FollowFileSymLinks";
    value = QString::number(ui->followFileSymlinksComboBox->currentIndex());
    ui->followFileSymlinksCheckBox->isChecked() == true?checked="checked":checked="not checked";
    profiles->setSectionValue("Directories",keyword,checked + "|" + value);

//    profiles->removeSection("AvailableOptions");
//    profiles->removeSection("SelectedOptions");

    keyword = "DontScanFileNamesMatchingRegExp";
    value = ui->pwdontScanFileNameLineEdit->text();
    ui->pwdontScanFileNameCheckBox->isChecked()?checked="checked":checked="not checked";
    profiles->setSectionValue(section,keyword,checked + "|" + value);

    keyword = "DontScanDiretoriesMatchingRegExp";
    value = ui->pwdontScanDirLineEdit->text();
    ui->pwdontScanDirCheckBox->isChecked()?checked="checked":checked="not checked";
    profiles->setSectionValue(section,keyword,checked + "|" + value);

    keyword = "OnlyScanFileNamesMatchingRegExp";
    value = ui->pwonlyScanFileNameLineEdit->text();
    ui->pwonlyScanFileNameCheckBox->isChecked()?checked="checked":checked="not checked";
    profiles->setSectionValue(section,keyword,checked + "|" + value);

    keyword = "OnlyScanDiretoriesMatchingRegExp";
    value = ui->pwonlyScanDirLineEdit->text();
    ui->pwonlyScanDirCheckBox->isChecked()?checked="checked":checked="not checked";
    profiles->setSectionValue(section,keyword,checked + "|" + value);

    keyword = "EnablePUAOptions";
    profiles->setSectionValue(section,keyword,ui->pwenablePUACheckBox->isChecked());

    keyword = "LoadPUAPacked";
    profiles->setSectionValue(section,keyword,ui->pwloadPUAPackedRadioButon->isChecked());

    keyword = "LoadPUAPWTool";
    profiles->setSectionValue(section,keyword,ui->pwloadPUAPWToolRadioButton->isChecked());

    keyword = "LoadPUANetTool";
    profiles->setSectionValue(section,keyword,ui->pwloadPUANetToolRadioButton->isChecked());

    keyword = "LoadPUAP2P";
    profiles->setSectionValue(section,keyword,ui->pwloadPUAP2PRadioButton->isChecked());

    keyword = "LoadPUAIRC";
    profiles->setSectionValue(section,keyword,ui->pwloadPUAIRCRadioButton->isChecked());

    keyword = "LoadPUARAT";
    profiles->setSectionValue(section,keyword,ui->pwloadPUARATRadioButton->isChecked());

    keyword = "LoadPUANetToolSpy";
    profiles->setSectionValue(section,keyword,ui->pwloadPUANetToolSpyRadioButton->isChecked());

    keyword = "LoadPUAServer";
    profiles->setSectionValue(section,keyword,ui->pwloadPUAServerRadioButton->isChecked());

    keyword = "LoadPUAScript";
    profiles->setSectionValue(section,keyword,ui->pwloadPUAScriptRadioButton->isChecked());

    keyword = "LoadPUAAndr";
    profiles->setSectionValue(section,keyword,ui->pwloadPUAAndrRadioButton->isChecked());

    keyword = "LoadPUAJava";
    profiles->setSectionValue(section,keyword,ui->pwloadPUAJavaRadioButton->isChecked());

    keyword = "LoadPUAOsx";
    profiles->setSectionValue(section,keyword,ui->pwloadPUAOsxRadioButton->isChecked());

    keyword = "LoadPUATool";
    profiles->setSectionValue(section,keyword,ui->pwloadPUAToolRadioButton->isChecked());

    keyword = "LoadPUAUnix";
    profiles->setSectionValue(section,keyword,ui->pwloadPUAUnixRadioButton->isChecked());

    keyword = "LoadPUAWin";
    profiles->setSectionValue(section,keyword,ui->pwloadPUAWinRadioButton->isChecked());

    keyword = "Files larger than this will be skipped and assumed clean";
    value = QString::number(ui->pwfilesLargerThanThisSpinBox->value()) + ui->pwfilesLargerThanThisComboBox->currentText();
    ui->pwfilesLargerThanThisCheckBox->isChecked() == true?checked="checked":checked="not checked";
    profiles->setSectionValue("ScanLimitations",keyword,checked + "|" + value);

    keyword = "The maximum amount of data to scan for each container file";
    value = QString::number(ui->pwmaxAmountForContainerSpinBox->value()) + ui->pwmaxAmountForContainerComboBox->currentText();
    ui->pwmaxAmountForContainerCheckBox->isChecked() == true?checked="checked":checked="not checked";
    profiles->setSectionValue("ScanLimitations",keyword,checked + "|" + value);

    keyword = "The maximum number of files to scan for each container file";
    value = QString::number(ui->pwmaxNumberForContainerSpinBox->value()) + ui->pwmaxNumberForContainerComboBox->currentText();
    ui->pwmaxNumberForContainerCheckBox->isChecked() == true?checked="checked":checked="not checked";
    profiles->setSectionValue("ScanLimitations",keyword,checked + "|" + value);

    keyword = "Maximum archive recursion level for container file";
    value = QString::number(ui->pwmaxArchiveRecursionForContainerSpinBox->value()) + ui->pwmaxArchiveRecursionForContainerComboBox->currentText();
    ui->pwmaxArchiveRecursionForContainerCheckBox->isChecked() == true?checked="checked":checked="not checked";
    profiles->setSectionValue("ScanLimitations",keyword,checked + "|" + value);

    keyword = "Maximum directory recursion level";
    value = QString::number(ui->pwmaxDirRecursionLevelSpinBox->value()) + ui->pwmaxDirRecursionLevelComboBox->currentText();
    ui->pwmaxDirRecursionLevelCheckBox->isChecked() == true?checked="checked":checked="not checked";
    profiles->setSectionValue("ScanLimitations",keyword,checked + "|" + value);

    keyword = "Maximum size file to check for embedded PE";
    value = QString::number(ui->pwmaxSizeFileForPESpinBox->value()) + ui->pwmaxSizeFileForPEComboBox->currentText();
    ui->pwmaxSizeFileForPECheckBox->isChecked() == true?checked="checked":checked="not checked";
    profiles->setSectionValue("ScanLimitations",keyword,checked + "|" + value);

    keyword = "Maximum size of HTML file to normalize";
    value = QString::number(ui->pwmaxSizeHTMLFileToNormalizeSpinBox->value()) + ui->pwmaxSizeHTMLFileToNormalizeComboBox->currentText();
    ui->pwmaxSizeHTMLFileToNormalizeCheckBox->isChecked() == true?checked="checked":checked="not checked";
    profiles->setSectionValue("ScanLimitations",keyword,checked + "|" + value);

    keyword = "Maximum size of normalized HTML file to scan";
    value = QString::number(ui->pwmaxSizeOfNormalizedHTMLFileSpinBox->value()) + ui->pwmaxSizeOfNormalizedHTMLFileComboBox->currentText();
    ui->pwmaxSizeOfNormalizedHTMLFileCheckBox->isChecked() == true?checked="checked":checked="not checked";
    profiles->setSectionValue("ScanLimitations",keyword,checked + "|" + value);

    keyword = "Maximum size of script file to normalize";
    value = QString::number(ui->pwmaxSizeOfScriptFileToNormalizeSpinBox->value()) + ui->pwmaxSizeOfScriptFileToNormalizeComboBox->currentText();
    ui->pwmaxSizeOfScriptFileToNormalizeCheckBox->isChecked() == true?checked="checked":checked="not checked";
    profiles->setSectionValue("ScanLimitations",keyword,checked + "|" + value);

    keyword = "Maximum size zip to type reanalyze";
    value = QString::number(ui->pwmaxSizeZipToTypeReanalzeSpinBox->value()) + ui->pwmaxSizeZipToTypeReanalzeComboBox->currentText();
    ui->pwmaxSizeZipToTypeReanalzeCheckBox->isChecked() == true?checked="checked":checked="not checked";
    profiles->setSectionValue("ScanLimitations",keyword,checked + "|" + value);

    keyword = "Maximum number of partitions in disk image to be scanned";
    value = QString::number(ui->pwmaxNumberOfPartitionsInDiskImageSpinBox->value()) + ui->pwmaxNumberOfPartitionsInDiskImageComboBox->currentText();
    ui->pwmaxNumberOfPartitionsInDiskImageCheckBox->isChecked() == true?checked="checked":checked="not checked";
    profiles->setSectionValue("ScanLimitations",keyword,checked + "|" + value);

    keyword = "Maximum number of icons in PE file to be scanned";
    value = QString::number(ui->pwmaxNumberOfIconsInPEFileSpinBox->value()) + ui->pwmaxNumberOfIconsInPEFileComboBox->currentText();
    ui->pwmaxNumberOfIconsInPEFileCheckBox->isChecked() == true?checked="checked":checked="not checked";
    profiles->setSectionValue("ScanLimitations",keyword,checked + "|" + value);

    keyword = "Number of seconds to wait for waiting a response back from the stats server";
    value = QString::number(ui->pwnumberOfSecondsToWaitForResponseSpinBox->value());
    ui->pwnumberOfSecondsToWaitForResponseCheckBox->isChecked() == true?checked="checked":checked="not checked";
    profiles->setSectionValue("ScanLimitations",keyword,checked + "|" + value);

    keyword = "Bytecode timeout in milliseconds";
    value = QString::number(ui->pwBytecodeTimeoutSpinBox->value());
    ui->pwBytecodeTimeoutCheckBox->isChecked() == true?checked="checked":checked="not checked";
    profiles->setSectionValue("ScanLimitations",keyword,checked + "|" + value);

    keyword = "Collect and print execution statistics";
    value = QString::number(ui->pwExecutionStatisticsComboBox->currentIndex());
    ui->pwExecutionStatisticsCheckBox->isChecked() == true?checked="checked":checked="not checked";
    profiles->setSectionValue("ScanLimitations",keyword,checked + "|" + value);

    keyword = "Structured SSN Format";
    value = QString::number(ui->pwStructuredSSNFormatComboBox->currentIndex());
    ui->pwstructuredSSNFormatCheckBox->isChecked() == true?checked="checked":checked="not checked";
    profiles->setSectionValue("ScanLimitations",keyword,checked + "|" + value);

    keyword = "Structured SSN Count";
    value = QString::number(ui->pwStructuredSSNCountSpinBox->value());
    ui->pwStructuredSSNCountCheckBox->isChecked() == true?checked="checked":checked="not checked";
    profiles->setSectionValue("ScanLimitations",keyword,checked + "|" + value);

    keyword = "Structured CC Count";
    value = QString::number(ui->pwStructuredCCCountSpinBox->value());
    ui->pwStructuredCCCountCheckBox->isChecked() == true?checked="checked":checked="not checked";
    profiles->setSectionValue("ScanLimitations",keyword,checked + "|" + value);

    keyword = "Structured CC Mode";
    value = QString::number(ui->pwStructuredCCModeComboBox->currentIndex());
    ui->pwStructuredCCModeCheckBox->isChecked() == true?checked="checked":checked="not checked";
    profiles->setSectionValue("ScanLimitations",keyword,checked + "|" + value);

    keyword = "Max Scan-Time";
    value = QString::number(ui->pwMaxScanTimeSpinBox->value());
    ui->pwMaxScanTimeCheckBox->isChecked() == true?checked="checked":checked="not checked";
    profiles->setSectionValue("ScanLimitations",keyword,checked + "|" + value);

    keyword = "Max recursion to HWP3 parsing function";
    value = QString::number(ui->pwMaxRecursionHWP3SpinBox->value());
    ui->pwMaxRecursionHWP3CheckBox->isChecked() == true?checked="checked":checked="not checked";
    profiles->setSectionValue("ScanLimitations",keyword,checked + "|" + value);

    keyword = "Max calls to PCRE match function";
    value = QString::number(ui->pwMaxCallsPCREMatchFunctionSpinBox->value());
    ui->pwMaxCallsPCREMatchFunctionCheckBox->isChecked() == true?checked="checked":checked="not checked";
    profiles->setSectionValue("ScanLimitations",keyword,checked + "|" + value);

    keyword = "Max recursion calls to the PCRE match function";
    value = QString::number(ui->pwMaxRecursionCallsPCREMatchFunctionCpinBox->value());
    ui->pwMaxRecursionCallsPCREMatchFunctionCheckBox->isChecked() == true?checked="checked":checked="not checked";
    profiles->setSectionValue("ScanLimitations",keyword,checked + "|" + value);

    keyword = "Max PCRE file size";
    value = QString::number(ui->pwMaxPCREFileSizeSpinBox->value()) + ui->pwMaxPCREFileSizeComboBox->currentText();
    ui->pwMaxPCREFileSizeCheckBox->isChecked() == true?checked="checked":checked="not checked";
    profiles->setSectionValue("ScanLimitations",keyword,checked + "|" + value);

    keyword = "Database outdated if older than x days";
    value = QString::number(ui->pwdatabaseOutdatedSpinBox->value());
    ui->pwdatabaseOutdatedCheckbox->isChecked() == true?checked="checked":checked="not checked";
    profiles->setSectionValue("ScanLimitations",keyword,checked + "|" + value);

    emit signal_profileSaved();
    this->accept();
}

void ProfileWizardDialog::slot_getClamscanProcessHasOutput()
{
    getClamscanProcessOutput = getClamscanProcessOutput + getClamscanParametersProcess->readAll();
}

void ProfileWizardDialog::slot_getClamscanProcessFinished()
{
    getClamscanProcessOutput = getClamscanProcessOutput + getClamscanParametersProcess->readAll();

    ui->pwBytecodeTimeoutCheckBox->setVisible(false);
    ui->pwBytecodeTimeoutSpinBox->setVisible(false);

    ui->pwExecutionStatisticsCheckBox->setVisible(false);
    ui->pwExecutionStatisticsComboBox->setVisible(false);

    ui->pwMaxCallsPCREMatchFunctionCheckBox->setVisible(false);
    ui->pwMaxCallsPCREMatchFunctionSpinBox->setVisible(false);

    ui->pwMaxPCREFileSizeCheckBox->setVisible(false);
    ui->pwMaxPCREFileSizeComboBox->setVisible(false);
    ui->pwMaxPCREFileSizeSpinBox->setVisible(false);

    ui->pwMaxRecursionCallsPCREMatchFunctionCheckBox->setVisible(false);
    ui->pwMaxRecursionCallsPCREMatchFunctionCpinBox->setVisible(false);

    ui->pwMaxRecursionHWP3CheckBox->setVisible(false);
    ui->pwMaxRecursionHWP3SpinBox->setVisible(false);

    ui->pwMaxScanTimeCheckBox->setVisible(false);
    ui->pwMaxScanTimeSpinBox->setVisible(false);

    ui->pwStructuredCCCountCheckBox->setVisible(false);
    ui->pwStructuredCCCountSpinBox->setVisible(false);

    ui->pwStructuredCCModeCheckBox->setVisible(false);
    ui->pwStructuredCCModeComboBox->setVisible(false);

    ui->pwStructuredSSNCountCheckBox->setVisible(false);
    ui->pwStructuredSSNCountSpinBox->setVisible(false);

    ui->pwstructuredSSNFormatCheckBox->setVisible(false);
    ui->pwStructuredSSNFormatComboBox->setVisible(false);

    ui->pwdatabaseOutdatedCheckbox->setVisible(false);
    ui->pwdatabaseOutdatedSpinBox->setVisible(false);

    ui->pwfilesLargerThanThisCheckBox->setVisible(false);
    ui->pwfilesLargerThanThisComboBox->setVisible(false);
    ui->pwfilesLargerThanThisSpinBox->setVisible(false);

    ui->pwmaxAmountForContainerCheckBox->setVisible(false);
    ui->pwmaxAmountForContainerComboBox->setVisible(false);
    ui->pwmaxAmountForContainerSpinBox->setVisible(false);

    ui->pwmaxArchiveRecursionForContainerCheckBox->setVisible(false);
    ui->pwmaxArchiveRecursionForContainerComboBox->setVisible(false);
    ui->pwmaxArchiveRecursionForContainerSpinBox->setVisible(false);

    ui->pwmaxDirRecursionLevelCheckBox->setVisible(false);
    ui->pwmaxDirRecursionLevelComboBox->setVisible(false);
    ui->pwmaxDirRecursionLevelSpinBox->setVisible(false);

    ui->pwmaxNumberForContainerCheckBox->setVisible(false);
    ui->pwmaxNumberForContainerComboBox->setVisible(false);
    ui->pwmaxNumberForContainerSpinBox->setVisible(false);

    ui->pwmaxNumberOfIconsInPEFileCheckBox->setVisible(false);
    ui->pwmaxNumberOfIconsInPEFileComboBox->setVisible(false);
    ui->pwmaxNumberOfIconsInPEFileSpinBox->setVisible(false);

    ui->pwmaxNumberOfPartitionsInDiskImageCheckBox->setVisible(false);
    ui->pwmaxNumberOfPartitionsInDiskImageComboBox->setVisible(false);
    ui->pwmaxNumberOfPartitionsInDiskImageSpinBox->setVisible(false);

    ui->pwmaxSizeFileForPECheckBox->setVisible(false);
    ui->pwmaxSizeFileForPEComboBox->setVisible(false);
    ui->pwmaxSizeFileForPESpinBox->setVisible(false);

    ui->pwmaxSizeHTMLFileToNormalizeCheckBox->setVisible(false);
    ui->pwmaxSizeHTMLFileToNormalizeComboBox->setVisible(false);
    ui->pwmaxSizeHTMLFileToNormalizeSpinBox->setVisible(false);

    ui->pwmaxSizeOfNormalizedHTMLFileCheckBox->setVisible(false);
    ui->pwmaxSizeOfNormalizedHTMLFileComboBox->setVisible(false);
    ui->pwmaxSizeOfNormalizedHTMLFileSpinBox->setVisible(false);

    ui->pwmaxSizeOfScriptFileToNormalizeCheckBox->setVisible(false);
    ui->pwmaxSizeOfScriptFileToNormalizeComboBox->setVisible(false);
    ui->pwmaxSizeOfScriptFileToNormalizeSpinBox->setVisible(false);

    ui->pwmaxSizeZipToTypeReanalzeCheckBox->setVisible(false);
    ui->pwmaxSizeZipToTypeReanalzeComboBox->setVisible(false);
    ui->pwmaxSizeZipToTypeReanalzeSpinBox->setVisible(false);

    ui->pwnumberOfSecondsToWaitForResponseCheckBox->setVisible(false);
    ui->pwnumberOfSecondsToWaitForResponseSpinBox->setVisible(false);

    QStringList lines = getClamscanProcessOutput.split("\n");
    QString line;

    for (int x = 0; x < lines.size(); x++) {
        line = lines[x];
        if (line.indexOf("--bytecode-timeout") != -1) {
            ui->pwBytecodeTimeoutCheckBox->setVisible(true);
            ui->pwBytecodeTimeoutSpinBox->setVisible(true);
        }

        if (line.indexOf("--statistics") != -1) {
            ui->pwExecutionStatisticsCheckBox->setVisible(true);
            ui->pwExecutionStatisticsComboBox->setVisible(true);
        }

        if (line.indexOf("--pcre-match-limit") != -1) {
            ui->pwMaxCallsPCREMatchFunctionCheckBox->setVisible(true);
            ui->pwMaxCallsPCREMatchFunctionSpinBox->setVisible(true);
        }

        if (line.indexOf("--pcre-max-filesize") != -1) {
            ui->pwMaxPCREFileSizeCheckBox->setVisible(true);
            ui->pwMaxPCREFileSizeComboBox->setVisible(true);
            ui->pwMaxPCREFileSizeSpinBox->setVisible(true);
        }

        if (line.indexOf("--pcre-recmatch-limit") != -1) {
            ui->pwMaxRecursionCallsPCREMatchFunctionCheckBox->setVisible(true);
            ui->pwMaxRecursionCallsPCREMatchFunctionCpinBox->setVisible(true);
        }

        if (line.indexOf("--max-rechwp3") != -1) {
            ui->pwMaxRecursionHWP3CheckBox->setVisible(true);
            ui->pwMaxRecursionHWP3SpinBox->setVisible(true);
        }

        if (line.indexOf("--max-scantime") != -1) {
            ui->pwMaxScanTimeCheckBox->setVisible(true);
            ui->pwMaxScanTimeSpinBox->setVisible(true);
        }

        if (line.indexOf("--structured-cc-count") != -1) {
            ui->pwStructuredCCCountCheckBox->setVisible(true);
            ui->pwStructuredCCCountSpinBox->setVisible(true);
        }

        if (line.indexOf("--structured-cc-mode") != -1) {
            ui->pwStructuredCCModeCheckBox->setVisible(true);
            ui->pwStructuredCCModeComboBox->setVisible(true);
        }

        if (line.indexOf("--structured-ssn-count") != -1) {
            ui->pwStructuredSSNCountCheckBox->setVisible(true);
            ui->pwStructuredSSNCountSpinBox->setVisible(true);
        }

        if (line.indexOf("--structured-ssn-format") != -1) {
            ui->pwstructuredSSNFormatCheckBox->setVisible(true);
            ui->pwStructuredSSNFormatComboBox->setVisible(true);
        }

        if (line.indexOf("--fail-if-cvd-older-than") != -1) {
            ui->pwdatabaseOutdatedCheckbox->setVisible(true);
            ui->pwdatabaseOutdatedSpinBox->setVisible(true);
        }

        if (line.indexOf("--max-filesize") != -1) {
            ui->pwfilesLargerThanThisCheckBox->setVisible(true);
            ui->pwfilesLargerThanThisComboBox->setVisible(true);
            ui->pwfilesLargerThanThisSpinBox->setVisible(true);
        }

        if (line.indexOf("--max-scansize") != -1) {
            ui->pwmaxAmountForContainerCheckBox->setVisible(true);
            ui->pwmaxAmountForContainerComboBox->setVisible(true);
            ui->pwmaxAmountForContainerSpinBox->setVisible(true);
        }

        if (line.indexOf("--max-recursion") != -1) {
            ui->pwmaxArchiveRecursionForContainerCheckBox->setVisible(true);
            ui->pwmaxArchiveRecursionForContainerComboBox->setVisible(true);
            ui->pwmaxArchiveRecursionForContainerSpinBox->setVisible(true);
        }

        if (line.indexOf("--max-dir-recursion") != -1) {
            ui->pwmaxDirRecursionLevelCheckBox->setVisible(true);
            ui->pwmaxDirRecursionLevelComboBox->setVisible(true);
            ui->pwmaxDirRecursionLevelSpinBox->setVisible(true);
        }

        if (line.indexOf("--max-files") != -1) {
            ui->pwmaxNumberForContainerCheckBox->setVisible(true);
            ui->pwmaxNumberForContainerComboBox->setVisible(true);
            ui->pwmaxNumberForContainerSpinBox->setVisible(true);
        }

        if (line.indexOf("--max-iconspe") != -1) {
            ui->pwmaxNumberOfIconsInPEFileCheckBox->setVisible(true);
            ui->pwmaxNumberOfIconsInPEFileComboBox->setVisible(true);
            ui->pwmaxNumberOfIconsInPEFileSpinBox->setVisible(true);
        }

        if (line.indexOf("--max-partitions") != -1) {
            ui->pwmaxNumberOfPartitionsInDiskImageCheckBox->setVisible(true);
            ui->pwmaxNumberOfPartitionsInDiskImageComboBox->setVisible(true);
            ui->pwmaxNumberOfPartitionsInDiskImageSpinBox->setVisible(true);
        }

        if (line.indexOf("--max-embeddedpe") != -1) {
            ui->pwmaxSizeFileForPECheckBox->setVisible(true);
            ui->pwmaxSizeFileForPEComboBox->setVisible(true);
            ui->pwmaxSizeFileForPESpinBox->setVisible(true);
        }

        if (line.indexOf("--max-htmlnormalize") != -1) {
            ui->pwmaxSizeHTMLFileToNormalizeCheckBox->setVisible(true);
            ui->pwmaxSizeHTMLFileToNormalizeComboBox->setVisible(true);
            ui->pwmaxSizeHTMLFileToNormalizeSpinBox->setVisible(true);
        }

        if (line.indexOf("--max-htmlnotags") != -1) {
            ui->pwmaxSizeOfNormalizedHTMLFileCheckBox->setVisible(true);
            ui->pwmaxSizeOfNormalizedHTMLFileComboBox->setVisible(true);
            ui->pwmaxSizeOfNormalizedHTMLFileSpinBox->setVisible(true);
        }

        if (line.indexOf("--max-scriptnormalize") != -1) {
            ui->pwmaxSizeOfScriptFileToNormalizeCheckBox->setVisible(true);
            ui->pwmaxSizeOfScriptFileToNormalizeComboBox->setVisible(true);
            ui->pwmaxSizeOfScriptFileToNormalizeSpinBox->setVisible(true);
        }

        if (line.indexOf("--max-ziptypercg") != -1) {
            ui->pwmaxSizeZipToTypeReanalzeCheckBox->setVisible(true);
            ui->pwmaxSizeZipToTypeReanalzeComboBox->setVisible(true);
            ui->pwmaxSizeZipToTypeReanalzeSpinBox->setVisible(true);
        }

        if (line.indexOf("--stats-timeout") != -1) {
            ui->pwnumberOfSecondsToWaitForResponseCheckBox->setVisible(true);
            ui->pwnumberOfSecondsToWaitForResponseSpinBox->setVisible(true);
        }
    }
}

void ProfileWizardDialog::slot_cancleButtonClicked()
{
    QString profileFile = QDir::homePath()+"/.clamav-gui/profiles/" + ui->profileNameLineEdit->text()+".ini";
    if ((newProfile == true) && (profileFile != QDir::homePath()+"/.clamav-gui/settings.ini")){
        QFile killfile(profileFile);
        if (killfile.exists() == true) killfile.remove();
    }
    this->close();
}
