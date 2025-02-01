#include "clamav_gui.h"
#include "ui_clamav_gui.h"

clamav_gui::clamav_gui(QWidget *parent) : QWidget(parent), ui(new Ui::clamav_gui)
{
    ui->setupUi(this);
    //setWindowFlags(Qt::WindowTitleHint);
    this->setWindowFlags(Qt::WindowTitleHint | Qt::WindowMinimizeButtonHint | Qt::WindowMaximizeButtonHint);

    QString path = QDir::homePath() + "/.clamav-gui/settings.ini";
    QDir tempDir;
    QStringList scanObjects;
    bool createDefaultSettings = false;
    error = false;
    guisudoapp = "pkexec";
//*****************************************************************************
//creating service Menu
//*****************************************************************************
if (tempDir.exists(QDir::homePath() + "/.local/share/kservices5/ServiceMenus/scanWithClamAV-GUI.desktop") == false) {
        if (tempDir.exists(QDir::homePath() + "/.local/share/kservices5/ServiceMenus")) tempDir.mkdir(QDir::homePath() + "/.local/share/kservices5/ServiceMenus");
        setupFileHandler * serviceFile = new setupFileHandler(QDir::homePath() + "/.local/share/kservices5/ServiceMenus/scanWithClamAV-GUI.desktop");
        serviceFile->setSectionValue("Desktop Entry","Type","Service");
        serviceFile->setSectionValue("Desktop Entry","ServiceTypes","KonqPopupMenu/Plugin");
        serviceFile->setSectionValue("Desktop Entry","MimeType","all/all;");
        serviceFile->setSectionValue("Desktop Entry","Actions","scan;");
        serviceFile->setSectionValue("Desktop Entry","Icon","clamav-gui");
        serviceFile->setSectionValue("Desktop Entry","X-KDE-Priority","TopLevel");
        serviceFile->setSectionValue("Desktop Entry","X-KDE-StartupNotify","false");
        serviceFile->setSectionValue("Desktop Entry","X-KDE-Submenu","Scan with ClamAV-GUI");
        serviceFile->setSectionValue("Desktop Entry","X-KDE-Submenu[de]","Mit ClamAV-GUI scannen");
        serviceFile->setSectionValue("Desktop Action scan","Name","scan");
        serviceFile->setSectionValue("Desktop Action scan","Name[de]","Scannen");
        serviceFile->setSectionValue("Desktop Action scan","Icon","clamav-gui");
        serviceFile->setSectionValue("Desktop Action scan","Exec","clamav-gui --scan %F");
    }
//*****************************************************************************

//*****************************************************************************
// Creating default direcotry structur in HOME
//*****************************************************************************
    if (tempDir.exists(QDir::homePath() + "/.clamav-gui") == false) {
        tempDir.mkdir(QDir::homePath() + "/.clamav-gui");
        createDefaultSettings = true;
    }
    if (tempDir.exists(QDir::homePath() + "/.clamav-gui/quarantine") == false) tempDir.mkdir(QDir::homePath() + "/.clamav-gui/quarantine");
    if (tempDir.exists(QDir::homePath() + "/.clamav-gui/signatures") == false) tempDir.mkdir(QDir::homePath() + "/.clamav-gui/signatures");
    if (tempDir.exists(QDir::homePath() + "/.clamav-gui/profiles") == false) tempDir.mkdir(QDir::homePath() + "/.clamav-gui/profiles");
    if (tempDir.exists(QDir::homePath() + "/.clamav-gui/logs") == false) tempDir.mkdir(QDir::homePath() + "/.clamav-gui/logs");
    QFile::setPermissions(QDir::homePath() + "/.clamav-gui/signatures",QFileDevice::ReadOwner|QFileDevice::ReadUser|QFileDevice::ReadGroup|QFileDevice::ReadOther|QFileDevice::WriteOwner|QFileDevice::WriteUser|QFileDevice::WriteGroup|QFileDevice::WriteOther|QFileDevice::ExeOwner|QFileDevice::ExeUser|QFileDevice::ExeGroup|QFileDevice::ExeOther);
//*****************************************************************************

//_____________________________________________________________________________________________________________________________________
// For UALinux
// If the settings.ini in the home folder of the user is not present a predefined version is been copied into the folder.
//_____________________________________________________________________________________________________________________________________
    QFile destChecker;
    QFile sourceChecker;
    QDir destDir;
    if (destChecker.exists(QDir::homePath() + "/.clamav-gui/settings.ini") == false) {
        destDir.mkdir(QDir::homePath() + "/.clamav-gui");
        if (sourceChecker.exists("/etc/clamav-gui/settings.ini") == true) QFile::copy("/etc/clamav-gui/settings.ini",QDir::homePath() + "/.clamav-gui/settings.ini");
    }
//______________________________________________________________________________________________________________________________________

    if (createDefaultSettings == true){
        setupFile = new setupFileHandler(path);
        if ((tempDir.exists("/var/lib/clamav") == true) && (destChecker.exists("/var/lib/clamav/freshclam.dat") == true)){
            setupFile->setSectionValue("Directories","LoadSupportedDBFiles","checked|/var/lib/clamav");
        } else {
            setupFile->setSectionValue("Directories","LoadSupportedDBFiles","checked|" + QDir::homePath() + "/.clamav-gui/signatures");
        }
        if (setupFile->keywordExists("Directories","TmpFile") == false) setupFile->setSectionValue("Directories","TmpFile","checked|/tmp");
        if (setupFile->keywordExists("Directories","ScanReportToFile") == false) setupFile->setSectionValue("Directories","ScanReportToFile","checked|" + QDir::homePath() + "/.clamav-gui/logs/report-scan.log");
        if (setupFile->keywordExists("Directories","MoveInfectedFiles") == false) setupFile->setSectionValue("Directories","MoveInfectedFiles","not checked|" + QDir::homePath()+"/.clamav-gui/quarantine");
        if (setupFile->keywordExists("Directories","CopyInfectedFiles") == false) setupFile->setSectionValue("Directories","CopyInfectedFiles","not checked|" + QDir::homePath()+"/.clamav-gui/quarantine");
        if (setupFile->keywordExists("Setup","language") == false) setupFile->setSectionValue("Setup","language",2);
    } else {
        setupFile = new setupFileHandler(path);
    }

    scanProcess = new QProcess(this);
    connect(scanProcess,SIGNAL(readyReadStandardError()),this,SLOT(slot_scanProcessHasErrOutput()));
    connect(scanProcess,SIGNAL(readyReadStandardOutput()),this,SLOT(slot_scanProcessHasStdOutput()));
    connect(scanProcess,SIGNAL(finished(int,QProcess::ExitStatus)),this,SLOT(slot_scanProcessFinished(int,QProcess::ExitStatus)));
    if (setupFile->getSectionBoolValue("Settings","ShowHideDropZone") == true) createDropZone();

    mainWindowTimer = new QTimer(this);
    connect(mainWindowTimer,SIGNAL(timeout()),this,SLOT(slot_mainWinTimerTimeout()));
    mainWindowTimer->setSingleShot(true);
    mainWindowTimer->start(250);

    createTrayIcon();
    trayIcon->setIcon(QIcon(":/icons/extra/icon32/clamav-gui.png"));
    trayIcon->show();
    connect(trayIcon,SIGNAL(activated(QSystemTrayIcon::ActivationReason)),this,SLOT(slot_systemTrayIconActivated(QSystemTrayIcon::ActivationReason)));

    ui->tabWidget->removeTab(0);
    scannerTab = new scanTab(this);
    connect(scannerTab,SIGNAL(requestDropZoneVisible()),this,SLOT(slot_showDropZone()));
    connect(scannerTab,SIGNAL(triggerScanRequest(QStringList)),this,SLOT(slot_scanRequest(QStringList)));
    connect(scannerTab,SIGNAL(abortScan()),this,SLOT(slot_abortScan()));
    connect(this,SIGNAL(setScannerForm(bool)),scannerTab,SLOT(slot_enableForm(bool)));
    ui->tabWidget->addTab(scannerTab,QIcon(":/icons/icons/Clam.png"),tr("Scan"));
    optionTab = new optionsDialog(this);
    ui->tabWidget->addTab(optionTab,QIcon(":/icons/icons/options.png"),tr("Options"));
    profileManagerTab = new ProfileManager(this);
    ui->tabWidget->addTab(profileManagerTab,QIcon(":/icons/icons/profilemanager.png"),tr("Profile Manager"));
    schedulerTab = new scheduler(this);
    ui->tabWidget->addTab(schedulerTab,QIcon(":/icons/icons/scheduler.png"),tr("Scheduler"));
    logTab = new logViewerObject(this);
    ui->tabWidget->addTab(logTab,QIcon(":/icons/icons/includeexclude.png"),tr("Logs"));
    freshclamTab = new freshclamsetter(this);
    connect(freshclamTab,SIGNAL(quitApplication()),this,SLOT(slot_quitApplication()));
    ui->tabWidget->addTab(freshclamTab,QIcon(":/icons/icons/freshclam.png"),tr("FreshClam"));
    clamdTab = new clamdManager(this);
    ui->tabWidget->addTab(clamdTab,QIcon(":/icons/icons/onaccess.png"),tr("Clamd"));
    setUpTab = new setupTab(this);
    ui->tabWidget->addTab(setUpTab,QIcon(":/icons/icons/setup.png"),tr("Setup"));
    infoTab = new infoDialog(this);
    ui->tabWidget->addTab(infoTab,QIcon(":/icons/icons/information.png"),tr("About"));
    ui->tabWidget->setTabShape(QTabWidget::Rounded);
    connect(freshclamTab,SIGNAL(setBallonMessage(int,QString,QString)),this,SLOT(slot_setTrayIconBalloonMessage(int,QString,QString)));
    connect(clamdTab,SIGNAL(setBallonMessage(int,QString,QString)),this,SLOT(slot_setTrayIconBalloonMessage(int,QString,QString)));
    connect(profileManagerTab,SIGNAL(triggerProfilesChanged()),schedulerTab,SLOT(slot_updateProfiles()));
    connect(profileManagerTab,SIGNAL(triggerProfilesChanged()),logTab,SLOT(slot_profilesChanged()));
    connect(schedulerTab,SIGNAL(triggerScanJob(QString,QStringList)),this,SLOT(slot_receiveScanJob(QString,QStringList)));
    connect(this,SIGNAL(scanJobFinished()),logTab,SLOT(slot_profilesChanged()));
    connect(schedulerTab,SIGNAL(logChanged()),logTab,SLOT(slot_profilesChanged()));
    logoTimer = new QTimer(this);
    logoTimer->setSingleShot(true);
    connect(logoTimer,SIGNAL(timeout()),this,SLOT(slot_logoTimerTimeout()));
    showLogoTimer = new QTimer(this);
    showLogoTimer->setSingleShot(true);
    connect(showLogoTimer,SIGNAL(timeout()),this,SLOT(slot_showLogoTimerTimeout()));
    showLogoTimer->start(250);
    connect(optionTab,SIGNAL(databasePathChanged(QString)),freshclamTab,SLOT(slot_dbPathChanged(QString)));
    connect(freshclamTab,SIGNAL(disableUpdateButtons()),freshclamTab,SLOT(slot_disableUpdateButtons()));
    connect(freshclamTab,SIGNAL(disableUpdateButtons()),scannerTab,SLOT(slot_disableScanButton()));
    connect(freshclamTab,SIGNAL(disableUpdateButtons()),schedulerTab,SLOT(slot_disableScheduler()));
    connect(freshclamTab,SIGNAL(reportError()),this,SLOT(slot_errorReporter()));
    connect(freshclamTab,SIGNAL(updateDatabase()),this,SLOT(slot_updateDatabase()));
    connect(optionTab,SIGNAL(updateDatabase()),this,SLOT(slot_updateDatabase()));
    connect(this,SIGNAL(startDatabaseUpdate()),freshclamTab,SLOT(slot_updateNowButtonClicked()));
    connect(optionTab,SIGNAL(updateClamdConf()),clamdTab,SLOT(slot_updateClamdConf()));
    connect(clamdTab,SIGNAL(setActiveTab()),this,SLOT(slot_startclamd()));
    connect(freshclamTab,SIGNAL(freshclamStarted()),clamdTab,SLOT(slot_waitForFreshclamStarted()));
    connect(freshclamTab,SIGNAL(systemStatusChanged()),setUpTab,SLOT(slot_updateSystemInfo()));
    connect(clamdTab,SIGNAL(systemStatusChanged()),setUpTab,SLOT(slot_updateSystemInfo()));
    connect(optionTab,SIGNAL(systemStatusChanged()),setUpTab,SLOT(slot_updateSystemInfo()));
    connect(setUpTab,SIGNAL(switchActiveTab(int)),this,SLOT(slot_switchActiveTab(int)));
    connect(setUpTab,SIGNAL(sendSystemInfo(QString)),scannerTab,SLOT(slot_receiveVersionInformation(QString)));
    ui->tabWidget->setCurrentIndex(0);

    sudoGUIProcess = new QProcess(this);
    connect(sudoGUIProcess,SIGNAL(finished(int)),this,SLOT(slot_sudoGUIProcessFinished()));
    QStringList parameters;
    parameters << "pkexec";
    sudoGUIProcess->start("whereis",parameters);
}
clamav_gui::~clamav_gui()
{
    delete ui;
}

void clamav_gui::closeEvent(QCloseEvent *event){
    if (error == true) {
        qApp->quit();
    } else {
        if (this->isVisible() == true) slot_hideWindow();
        event->ignore();
    }
}

void clamav_gui::changeEvent(QEvent * event)
{
    if (event->type() == QEvent::WindowStateChange) {
        if (isMinimized() == true) {
            slot_setMainWindowState(false);
        }
    }
}

void clamav_gui::createTrayIcon(){

    actionShowHideMainWindow = new QAction(QIcon(":/icons/icons/showhide.png"),tr("Show/Hide MainWindow"),this);
    connect(actionShowHideMainWindow,SIGNAL(triggered()),this,SLOT(slot_actionShowHideMainWindowTriggered()));
    actionShowHideDropZone = new QAction(QIcon(":/icons/icons/showhide.png"),tr("Show/Hide DropZone"),this);
    connect(actionShowHideDropZone,SIGNAL(triggered()),this,SLOT(slot_actionShowHideDropZoneTriggered()));
    actionQuit = new QAction(QIcon(":/icons/icons/application-exit.png"),tr("Quit"),this);
    connect(actionQuit,SIGNAL(triggered()),qApp,SLOT(quit()));

    trayIconMenu = new QMenu(this);
    trayIconMenu->setStyleSheet("background:lightgray");
    trayIconMenu->addAction(actionShowHideMainWindow);
    trayIconMenu->addAction(actionShowHideDropZone);
    trayIconMenu->addSeparator();
    trayIconMenu->addAction(actionQuit);
    trayIcon = new QSystemTrayIcon(this);
    trayIcon->setContextMenu(trayIconMenu);
}

void clamav_gui::slot_actionShowHideMainWindowTriggered(){
    if (setupFile->getSectionBoolValue("Settings","ShowHideMainWindow") == true) {
        slot_setMainWindowState(false);
        setupFile->setSectionValue("Settings","ShowHideMainWindow",false);
    } else {
        slot_setMainWindowState(true);
        setupFile->setSectionValue("Settings","ShowHideMainWindow",true);
    }
}

void clamav_gui::slot_systemTrayIconActivated(QSystemTrayIcon::ActivationReason reason){
    if (reason == QSystemTrayIcon::MiddleClick){
        slot_actionShowHideDropZoneTriggered();
    } else {
        if (reason == QSystemTrayIcon::Trigger){
            slot_actionShowHideMainWindowTriggered();
        }
    }
}

void clamav_gui::slot_setMainWindowState(bool state){
    if (state == true){
        this->showMaximized();
        this->activateWindow();
        setupFile->setSectionValue("Settings","ShowHideMainWindow",true);
    } else {
        if (this->isVisible() == true) this->hide();
        setupFile->setSectionValue("Settings","ShowHideMainWindow",false);
    }
}

void clamav_gui::slot_actionShowHideDropZoneTriggered(){
    if (setupFile->getSectionBoolValue("Settings","ShowHideDropZone") == false){
        setupFile->setSectionValue("Settings","ShowHideDropZone",true);
        createDropZone();
    } else {
        setupFile->setSectionValue("Settings","ShowHideDropZone",false);
        dropZone->close();
    }
}

void clamav_gui::slot_hideWindow(){
    slot_setMainWindowState(false);
}

void clamav_gui::createDropZone(){

    if (setupFile->getSectionBoolValue("Settings","ShowHideMainWindow") == true) this->hide();
    dropZone = new clamav_ctrl();
    connect(dropZone,SIGNAL(scanRequest(QStringList)),this,SLOT(slot_scanRequest(QStringList)));
    dropZone->show();
    if (setupFile->getSectionBoolValue("Settings","ShowHideMainWindow") == true) this->show();
}

void clamav_gui::slot_scanRequest(QStringList scanObjects){
QStringList parameters;
QStringList selectedOptions = setupFile->getKeywords("SelectedOptions");
QStringList directoryOptions = setupFile->getKeywords("Directories");
QStringList scanLimitations = setupFile->getKeywords("ScanLimitations");
QString option;
QString checked;
QString value;
QString temp;
QString moveDirectory = optionTab->getMoveDirectory();;
QString copyDirectory = optionTab->getCopyDirectory();

    emit setScannerForm(false);
    scannerTab->setStatusBarMessage(tr("Scanning started ......."),"#ffff00");
    if (scannerTab->recursivChecked() == true){
        parameters << "-r";
    }
    switch (scannerTab->getVirusFoundComboBoxValue()) {
    case 0:
        break;
    case 1: parameters << "--remove=yes";
        break;
    case 2: if (moveDirectory != "") parameters << "--move=" + moveDirectory;
        break;
    case 3: if (moveDirectory != "") parameters << "--copy=" + copyDirectory;
        break;
    }
    for (int i = 0; i < selectedOptions.count(); i++){
        parameters << selectedOptions.at(i).left(selectedOptions.indexOf("|")).replace("<equal>","=");
    }

    // Directory Options
    for (int i = 0; i < directoryOptions.count(); i++){
        option = directoryOptions.at(i);
        value = setupFile->getSectionValue("Directories",option);
        checked = value.left(value.indexOf("|"));
        value = value.mid(value.indexOf("|") + 1);
        if ((checked == "checked") && (value != "")) {
            if (option == "LoadSupportedDBFiles") parameters << "--database=" + value;
            if (option == "ScanReportToFile") {
                parameters << "--log=" + value;
                if (value != "") {
                    QFile file(value);
                    if (file.open(QIODevice::ReadWrite|QIODevice::Append|QIODevice::Text)){
                        QTextStream stream(&file);
                        stream << "\n<Scanning startet> " << QDateTime::currentDateTime().toString("yyyy/M/d - hh:mm");
                        file.close();
                    }
                }
            }
            if (option == "TmpFile") parameters << "--tempdir=" + value;
            if (option == "MoveInfectedFiles") parameters << "--move=" + value;
            if (option == "CopyInfectedFiles") parameters << "--copy=" + value;
            if (option == "SCanFileFromFiles") parameters << "--file-list=" + value;
            if (option == "FollowDirectorySymLinks") parameters << "--follow-dir-symlinks=" + value;
            if (option == "FollowFileSymLinks") parameters << "--follow-file-symlinks=" + value;
        }
    }

    // Scan Limitations
    for (int i = 0; i < scanLimitations.count(); i++){
        option = scanLimitations.at(i);
        value = setupFile->getSectionValue("ScanLimitations",option);
        checked = value.left(value.indexOf("|"));
        value = value.mid(value.indexOf("|") + 1);
        if (checked == "checked"){
            if (option == "Files larger than this will be skipped and assumed clean") parameters << "--max-filesize=" + value;
            if (option == "The maximum amount of data to scan for each container file") parameters << "--max-scansize=" + value;
            if (option == "The maximum number of files to scan for each container file") parameters << "--max-files=" + value;
            if (option == "Maximum archive recursion level for container file") parameters << "--max-recursion=" + value;
            if (option == "Maximum directory recursion level") parameters << "--max-dir-recursion=" + value;
            if (option == "Maximum size file to check for embedded PE") parameters << "--max-embeddedpe=" + value;
            if (option == "Maximum size of HTML file to normalize") parameters << "--max-htmlnormalize=" + value;
            if (option == "Maximum size of normalized HTML file to scan") parameters << "--max-htmlnotags=" + value;
            if (option == "Maximum size of script file to normalize") parameters << "--max-scriptnormalize=" + value;
            if (option == "Maximum size zip to type reanalyze") parameters << "--max-ziptypercg=" + value;
            if (option == "Maximum number of partitions in disk image to be scanned") parameters << "--max-partitions=" + value;
            if (option == "Maximum number of icons in PE file to be scanned") parameters << "--max-iconspe=" + value;
            if (option == "Number of seconds to wait for waiting a response back from the stats server") parameters << "--stats-timeout=" + value;
            if (option == "Bytecode timeout in milliseconds") parameters << "--bytecode-timeout=" + value;
            if (option == "Collect and print execution statistics") parameters << "--statistics=" + value;
            if (option == "Structured SSN Format") parameters << "--structured-ssn-format=" + value;
            if (option == "Structured SSN Count") parameters << "--structured-ssn-count=" + value;
            if (option == "Structured CC Count") parameters << "--structured-cc-count=" + value;
            if (option == "Structured CC Mode") parameters << "--structured-cc-mode=" + value;
            if (option == "Max Scan-Time") parameters << "--max-scantime=" + value;
            if (option == "Max recursion to HWP3 parsing function") parameters << "--max-rechwp3=" + value;
            if (option == "Max calls to PCRE match function") parameters << "--pcre-match-limit=" + value;
            if (option == "Max recursion calls to the PCRE match function") parameters << "--pcre-recmatch-limit=" + value;
            if (option == "Max PCRE file size") parameters << "--pcre-max-filesize=" + value;
            if (option == "Database outdated if older than x days") parameters << "--fail-if-cvd-older-than=" + value;

        }
    }

    // REGEXP and Include Exclude Options
    value = setupFile->getSectionValue("REGEXP_and_IncludeExclude","DontScanFileNamesMatchingRegExp");
    checked = value.left(value.indexOf("|"));
    value = value.mid(value.indexOf("|") + 1);
    if (checked == "checked") parameters << "--exclude=" + value;

    value = setupFile->getSectionValue("REGEXP_and_IncludeExclude","DontScanDirectoriesMatchingRegExp");
    checked = value.left(value.indexOf("|"));
    value = value.mid(value.indexOf("|") + 1);
    if (checked == "checked") parameters << "--exclude-dir=" + value;

    value = setupFile->getSectionValue("REGEXP_and_IncludeExclude","OnlyScanFileNamesMatchingRegExp");
    checked = value.left(value.indexOf("|"));
    value = value.mid(value.indexOf("|") + 1);
    if (checked == "checked") parameters << "--include=" + value;

    value = setupFile->getSectionValue("REGEXP_and_IncludeExclude","OnlyScanDirectoriesMatchingRegExp");
    checked = value.left(value.indexOf("|"));
    value = value.mid(value.indexOf("|") + 1);
    if (checked == "checked") parameters << "--include-dir=" + value;

    if (setupFile->getSectionBoolValue("REGEXP_and_IncludeExclude","EnablePUAOptions") == true){
        if (setupFile->getSectionBoolValue("REGEXP_and_IncludeExclude","LoadPUAPacked") == true) parameters << "--include-pua=Packed";
        if (setupFile->getSectionBoolValue("REGEXP_and_IncludeExclude","LoadPUAPWTool") == true) parameters << "--include-pua=PWTool";
        if (setupFile->getSectionBoolValue("REGEXP_and_IncludeExclude","LoadPUANetTool") == true) parameters << "--include-pua=NetTool";
        if (setupFile->getSectionBoolValue("REGEXP_and_IncludeExclude","LoadPUAP2P") == true) parameters << "--include-pua=P2P";
        if (setupFile->getSectionBoolValue("REGEXP_and_IncludeExclude","LoadPUAIRC") == true) parameters << "--include-pua=IRC";
        if (setupFile->getSectionBoolValue("REGEXP_and_IncludeExclude","LoadPUARAT") == true) parameters << "--include-pua=RAT";
        if (setupFile->getSectionBoolValue("REGEXP_and_IncludeExclude","LoadPUANetToolSpy") == true) parameters << "--include-pua=NetToolSpy";
        if (setupFile->getSectionBoolValue("REGEXP_and_IncludeExclude","LoadPUAServer") == true) parameters << "--include-pua=Server";
        if (setupFile->getSectionBoolValue("REGEXP_and_IncludeExclude","LoadPUAScript") == true) parameters << "--include-pua=Script";
        if (setupFile->getSectionBoolValue("REGEXP_and_IncludeExclude","LoadPUAAndr") == true) parameters << "--include-pua=Andr";
        if (setupFile->getSectionBoolValue("REGEXP_and_IncludeExclude","LoadPUAJava") == true) parameters << "--include-pua=Java";
        if (setupFile->getSectionBoolValue("REGEXP_and_IncludeExclude","LoadPUAOsx") == true) parameters << "--include-pua=Osx";
        if (setupFile->getSectionBoolValue("REGEXP_and_IncludeExclude","LoadPUATool") == true) parameters << "--include-pua=Tool";
        if (setupFile->getSectionBoolValue("REGEXP_and_IncludeExclude","LoadPUAUnix") == true) parameters << "--include-pua=Unix";
        if (setupFile->getSectionBoolValue("REGEXP_and_IncludeExclude","LoadPUAWin") == true) parameters << "--include-pua=Win";
    }

    temp = "clamscan ";
    QString path = temp;
    path = setupFile->getSectionValue("Directories","LoadSupportedDBFiles");
    path = path.mid(path.indexOf("|")+1);
    if (path.indexOf("not checked") != - 1) temp = temp + "-d " + path;
    for (int i = 0; i < parameters.count(); i++){
        temp = temp + " " + parameters.at(i);
    }

    // parameters << "-d" << path;
    for (int i = 0; i < scanObjects.count(); i++){
        parameters << scanObjects.at(i);
        temp = temp + " " + scanObjects.at(i);
    }

    slot_setMainWindowState(true);
    if (setupFile->getSectionBoolValue("Settings","ShowHideDropZone") == true){
        dropZone->close();
    }
    ui->tabWidget->setCurrentIndex(0);
    scannerTab->clearLogMessage();
    scannerTab->setStatusMessage(temp+char(13));
    scanProcess->start("clamscan",parameters);
}

void clamav_gui::slot_mainWinTimerTimeout(){
    if (setupFile->getSectionBoolValue("Settings","ShowHideMainWindow") == true) {
        slot_setMainWindowState(true);
    } else {
        slot_setMainWindowState(false);
    }
}

void clamav_gui::slot_scanProcessHasStdOutput(){
QString output = scanProcess->readAllStandardOutput();
    scannerTab->setStatusMessage(output);
}

void clamav_gui::slot_scanProcessHasErrOutput(){
QString output = scanProcess->readAllStandardError();
    scannerTab->setStatusMessage(output);
}

void clamav_gui::slot_scanProcessFinished(int exitCode,QProcess::ExitStatus status){
Q_UNUSED(exitCode);
Q_UNUSED(status);
QStringList parameters;
//QString temp;

    if (status == QProcess::CrashExit) {
        scannerTab->setStatusBarMessage(tr("Scan-Process aborted ......"),"#ff0000");
        trayIcon->showMessage(tr("Scan-Status"),tr("Scan Process aborted ....."),QSystemTrayIcon::Warning,5000);
    } else {
        if (exitCode == 0) {
            scannerTab->setStatusBarMessage(tr("Scan-Process finished ...... no Virus found!"),"#00ff00");
            trayIcon->showMessage(tr("Scan-Status"),tr("Scan Process finished ..... no virus found!"),QSystemTrayIcon::Information,5000);
        } else
            if (exitCode == 1) {
                scannerTab->setStatusBarMessage(tr("Scan-Process finished ...... Virus found!"),"#ff0000");
                trayIcon->showMessage(tr("Scan-Status"),tr("Scan Process finished ..... a virus was found!"),QSystemTrayIcon::Critical,5000);
            } else {
                scannerTab->setStatusBarMessage(tr("Scan-Process finished ...... an error occured!"),"#ff0000");
                trayIcon->showMessage(tr("Scan-Status"),tr("Scan Process finished ..... an error occurred!"),QSystemTrayIcon::Warning,5000);
            }
    }

    emit setScannerForm(true);

    if (setupFile->getSectionBoolValue("Settings","ShowHideDropZone") == true){
        createDropZone();
    }
    emit scanJobFinished();
}

void clamav_gui::slot_abortScan(){
    scannerTab->setStatusMessage(tr("Scan-Process aborted!"));
    scannerTab->setStatusBarMessage(tr("Scan-Process aborted!"),"#ff0000");
    if (scanProcess->state() == QProcess::Running) scanProcess->kill();
}

void clamav_gui::slot_showDropZone(){
    if (setupFile->getSectionBoolValue("Settings","ShowHideDropZone") == false){
        setupFile->setSectionValue("Settings","ShowHideDropZone",true);
        createDropZone();
    }
}

void clamav_gui::slot_receiveScanJob(QString name, QStringList parameters){
scheduleScanObject * scanObject = new scheduleScanObject(0,name,parameters);

    scanObject->setWindowTitle(name);
    scanObject->setWindowIcon(QIcon(":/icons/icons/media.png"));
    connect(scanObject,SIGNAL(sendStatusReport(int,QString,QString)),this,SLOT(slot_setTrayIconBalloonMessage(int,QString,QString)));
    connect(scanObject,SIGNAL(scanProcessFinished()),logTab,SLOT(slot_profilesChanged()));
    connect(scanObject,SIGNAL(closeRequest(QWidget*)),this,SLOT(slot_closeRequest(QWidget*)));
    scanObject->setModal(false);
    scanObject->show();
}

void clamav_gui::slot_setTrayIconBalloonMessage(int status,QString title,QString message){
    switch (status){
    case 0  :   trayIcon->showMessage(title,message,QSystemTrayIcon::Information,5000);
                    break;
    case 1  :   trayIcon->showMessage(title,message,QSystemTrayIcon::Warning);
                    break;
    case 2  :   trayIcon->showMessage(title,message,QSystemTrayIcon::Critical);
                    break;

    }

}

void clamav_gui::slot_closeRequest(QWidget * dialog){
    delete dialog;
}

void clamav_gui::slot_logoTimerTimeout(){
    startLogoLabel->hide();
    if (setupFile->keywordExists("Setup","WindowState")) {
        if (setupFile->getSectionValue("Setup","WindowState") == "minimized") this->close();
        if (setupFile->getSectionValue("Setup","WindowState") == "maximized") {
            this->showMaximized();
        }
    } else {
        this->showMaximized();
    }

}

void clamav_gui::slot_showLogoTimerTimeout(){
    QScreen * screen = QGuiApplication::primaryScreen();
    QRect screenGeometry = screen->geometry();
    startLogoLabel = new QLabel(0);
    startLogoLabel->setStyleSheet("background:transparent");
    startLogoLabel->setAttribute(Qt::WA_TranslucentBackground);
    startLogoLabel->setWindowFlags(Qt::FramelessWindowHint);
    startLogoLabel->setPixmap(QPixmap(":/icons/icons/startlogo.png"));
    startLogoLabel->setGeometry((screenGeometry.width()-400) / 2, (screenGeometry.height() - 300) / 2, 400,300);
    startLogoLabel->show();
    startLogoLabel->raise();
    logoTimer->start(2500);
}

void clamav_gui::slot_errorReporter()
{
    error = true;
}

void clamav_gui::slot_updateDatabase()
{
    ui->tabWidget->setCurrentIndex(5);
    emit startDatabaseUpdate();
}

void clamav_gui::slot_startclamd(){
    ui->tabWidget->setCurrentIndex(6);
}

void clamav_gui::slot_sudoGUIProcessFinished()
{
    QStringList parameters;

    QString sudoGUIOutput = sudoGUIProcess->readAll();
    if (guisudoapp == "pkexec") {
        if (sudoGUIOutput != "pkexec:\n") {
            QStringList values = sudoGUIOutput.split(" ");
            if (values.size() > 1) {
                if (values.length() > 0) setupFile->setSectionValue("Settings","SudoGUI",values[1]);
            } else {
                guisudoapp = "kdesu";
                parameters << "kdesu";
                sudoGUIProcess->start("whereis",parameters);
            }
        }
    }

    if (guisudoapp == "kdesudo") {
        if (sudoGUIOutput != "kdesudo:\n") {
            QStringList values = sudoGUIOutput.split(" ");
            if (values.size() > 1) {
                if (values.length() > 0) setupFile->setSectionValue("Settings","SudoGUI",values[1]);
            } else {
                QMessageBox::warning(this,tr("WARNING"),tr("Neither \"pkexec\" nor \"kdesu\" is installed. Please install at least one of this to apps!"));
            }
        }
    }
}

void clamav_gui::slot_switchActiveTab(int index)
{
    ui->tabWidget->setCurrentIndex(index);
}

void clamav_gui::slot_quitApplication()
{
    qApp->quit();
}
