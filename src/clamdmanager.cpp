#include "clamdmanager.h"
#include "ui_clamdmanager.h"

clamdManager::clamdManager(QWidget *parent) : QWidget(parent), ui(new Ui::clamdManager)
{
    ui->setupUi(this);
    startup = true;
    waitForFreshclam = true;
    clamdStartupCounter = 0;
    startDelayTimer = new QTimer(this);
    startDelayTimer->setSingleShot(true);
    connect(startDelayTimer,SIGNAL(timeout()),this,SLOT(slot_startDelayTimerExpired()));

    processWatcher = new QTimer(this);
    connect(processWatcher,SIGNAL(timeout()),this,SLOT(slot_processWatcherExpired()));
    processWatcher->start(30000);

    initClamdSettings();
}

clamdManager::~clamdManager()
{
    delete ui;
}

QString clamdManager::trimLocationOutput(QString value)
{
    QString rc = value;
    int start = rc.indexOf(" ") + 1;
    int ende = rc.indexOf(" ",start);
    ende = ende - start;
    rc = value.mid(start,ende);
    rc = rc.replace("\n","");
    return rc;
}

void clamdManager::initClamdSettings() {
    setupFile = new setupFileHandler(QDir::homePath() + "/.clamav-gui/settings.ini");
    sudoGUI = setupFile->getSectionValue("Settings","SudoGUI");
    clamdConf = new setupFileHandler(QDir::homePath() + "/.clamav-gui/clamd.conf");
    QStringList parameters;
    QStringList monitorings = setupFile->getKeywords("Clamonacc");
    ui->monitoringComboBox->addItems(monitorings);
    logHighlighter = new highlighter(ui->clamdLogPlainTextEdit->document());

    clamdPidWatcher = new QFileSystemWatcher(this);
    connect(clamdPidWatcher,SIGNAL(fileChanged(QString)),this,SLOT(slot_pidWatcherTriggered()));

    clamdLocationProcess = new QProcess(this);
    connect(clamdLocationProcess,SIGNAL(finished(int)),this,SLOT(slot_clamdLocationProcessFinished()));

    clamonaccLocationProcess = new QProcess(this);
    connect(clamonaccLocationProcess,SIGNAL(finished(int)),this,SLOT(slot_clamonaccLocationProcessFinished()));

    startClamdProcess = new QProcess(this);
    connect(startClamdProcess,SIGNAL(finished(int,QProcess::ExitStatus)),this,SLOT(slot_startClamdProcessFinished(int,QProcess::ExitStatus)));

    killProcess = new QProcess(this);
    connect(killProcess,SIGNAL(finished(int)),this,SLOT(slot_killClamdProcessFinished()));

    findclamonaccProcess = new QProcess(this);
    connect(findclamonaccProcess,SIGNAL(finished(int)),this,SLOT(slot_findclamonaccProcessFinished(int)));

    restartClamonaccProcess = new QProcess(this);
    connect(restartClamonaccProcess,SIGNAL(finished(int)),this,SLOT(slot_restartClamonaccProcessFinished()));

    if (checkClamdRunning() == true) {
        ui->startStopClamdPushButton->setStyleSheet("background-color:green");
        ui->startStopClamdPushButton->setText(tr("  Clamd running - Stop clamd"));
        ui->startStopClamdPushButton->setIcon(QIcon(":/icons/icons/stopclamd.png"));
        clamdPidWatcher->addPath("/tmp/clamd.pid");
    } else {
        ui->startStopClamdPushButton->setStyleSheet("background-color:red");
    }

    clamdRestartInProgress = false;

    if (setupFile->keywordExists("OnAccess","LogFileMaxSize") == true) ui->accLogFileMaxSizeSpinBox->setValue(setupFile->getSectionIntValue("OnAccess","LogFileMaxSize")); else {
        setupFile->setSectionValue("OnAccess","LogFileMaxSize","1");
        clamdConf->setSingleLineValue("LogFileMaxSize","1");
    }
    if (setupFile->keywordExists("OnAccess","LogTimes") == true) ui->accLogTimesComboBox->setCurrentText(setupFile->getSectionValue("OnAccess","LogTimes")); else {
        setupFile->setSectionValue("OnAccess","LogTimes","yes");
        clamdConf->setSingleLineValue("LogTime","yes");
    }
    if (setupFile->keywordExists("OnAccess","LotRotate") == true) ui->accLogRotateComboBox->setCurrentText(setupFile->getSectionValue("OnAccess","LogRotate")); else {
        setupFile->setSectionValue("OnAccess","LogRotate","yes");
        clamdConf->setSingleLineValue("LogRotate","yes");
    }
    if (setupFile->keywordExists("OnAccess","OnAccessMaxFileSize") == true) ui->onAccessMaxFileSizeSpinBox->setValue(setupFile->getSectionDoubleValue("OnAccess","OnAccessMaxFileSize")); else {
        setupFile->setSectionValue("OnAccess","OnAccessMaxFileSize","10");
        clamdConf->setSingleLineValue("OnAccessMaxFileSize","10M");
    }
    if (setupFile->keywordExists("OnAccess","OnAccessMaxThreads") == true) ui->onAccessMaxThreadsSpinBox->setValue(setupFile->getSectionDoubleValue("OnAccess","OnAccessMaxThreads")); else {
        setupFile->setSectionValue("OnAccess","OnAccessMaxThreads","10");
        clamdConf->setSingleLineValue("OnAccessMaxThreads","10");
    }
    if (setupFile->keywordExists("OnAccess","OnAccessPrevention") == true) ui->onAccessPreventionComboBox->setCurrentText(setupFile->getSectionValue("OnAccess","OnAccessPrevention")); else {
        setupFile->setSectionValue("OnAccess","OnAccessPrevention","yes");
        clamdConf->setSingleLineValue("OnAccessPrevention","yes");
    }
    if (setupFile->keywordExists("OnAccess","OnAccessDenyOnError") == true) ui->onAccessDenyOnErrorComboBox->setCurrentText(setupFile->getSectionValue("OnAccess","OnAccessDenyOnError")); else {
        setupFile->setSectionValue("OnAccess","OnAccessDenyOnError","no");
        clamdConf->setSingleLineValue("OnAccessDenyOnError","no");
    }
    if (setupFile->keywordExists("OnAccess","OnAccessExtraScanning") == true) ui->accExtraScanningComboBox->setCurrentText(setupFile->getSectionValue("OnAccess","OnAccessExtraScanning")); else {
        setupFile->setSectionValue("OnAccess","OnAccessExtraScanning","yes");
        clamdConf->setSingleLineValue("OnAccessExtraScanning","yes");
    }
    if (setupFile->keywordExists("OnAccess","OnAccessRetryAttempts") == true) ui->accRetryAttemptsSpinBox->setValue(setupFile->getSectionDoubleValue("OnAccess","OnAccessRetryAttempts")); else {
        setupFile->setSectionValue("OnAccess","OnAccessRetryAttempts","0");
        clamdConf->setSingleLineValue("OnAccessRetryAttempts","0");
    }
    if (setupFile->keywordExists("OnAccess","ExtendedDetection") == true) ui->extendedDetectionInfoComboBox->setCurrentText(setupFile->getSectionValue("OnAccess","ExtendedDetection")); else {
        setupFile->setSectionValue("OnAccess","ExtendedDetection","yes");
        clamdConf->setSingleLineValue("ExtendedDetection","yes");
    }
    if (setupFile->keywordExists("OnAccess","InfectedFiles") == true) ui->infectedFilesComboBox->setCurrentIndex(setupFile->getSectionIntValue("OnAccess","InfectedFiles"));

    ui->restartClamdPushButton->setVisible(false);
    slot_updateClamdConf();

    parameters << "clamd";
    clamdLocationProcess->start("whereis",parameters);

    parameters.clear();
    parameters << "clamonacc";
    clamonaccLocationProcess->start("whereis",parameters);
}

void clamdManager::slot_updateClamdConf()
{
    QStringList path;
    QString logPath = QDir::homePath() + "/.clamav-gui/clamd.log";
    QFile checkFile(logPath);
    if (checkFile.exists() == false) {
        if (checkFile.open(QIODevice::WriteOnly|QIODevice::Text)){
            checkFile.close();
        }
    } else {
        slot_logFileContentChanged();
    }

    path << logPath;
    clamdLogWatcher = new QFileSystemWatcher(path,this);
    connect(clamdLogWatcher,SIGNAL(fileChanged(QString)),this,SLOT(slot_logFileContentChanged()));
    setupFileHandler * helperHandler = new setupFileHandler(QDir::homePath() + "/.clamav-gui/settings.ini");
    freshclamConf = new setupFileHandler(QDir::homePath() + "/.clamav-gui/freshclam.conf");

    clamdConf = new setupFileHandler(QDir::homePath() + "/.clamav-gui/clamd.conf");
    clamdConf->clearSetupFile();
    clamdConf->setSingleLineValue("DatabaseDirectory", freshclamConf->getSingleLineValue("DatabaseDirectory"));
    clamdConf->setSingleLineValue("LogSyslog", freshclamConf->getSingleLineValue("LogSyslog"));
    clamdConf->setSingleLineValue("LogFacility", freshclamConf->getSingleLineValue("LogFacility"));
    clamdConf->setSingleLineValue("PidFile", "/tmp/clamd.pid");
    clamdConf->setSingleLineValue("ExtendedDetectionInfo",setupFile->getSectionValue("OnAccess","ExtendedDetection"));
    clamdConf->setSingleLineValue("LocalSocket", QDir::homePath() + "/.clamav-gui/clamd-socket");
    clamdConf->addSingleLineValue("LogFile",QDir::homePath() + "/.clamav-gui/clamd.log");
    clamdConf->setSingleLineValue("LocalSocketGroup", "users");
    clamdConf->setSingleLineValue("TCPAddr", "127.0.0.1");
    clamdConf->addSingleLineValue("TCPAddr", "::1");
    clamdConf->addSingleLineValue("LogFileMaxSize",setupFile->getSectionValue("OnAccess", "LogFileMaxSize")+"M");
    clamdConf->addSingleLineValue("LogTime",setupFile->getSectionValue("OnAccess", "LogTimes"));
    clamdConf->addSingleLineValue("LogRotate",setupFile->getSectionValue("OnAccess", "LogRotate"));
    clamdConf->addSingleLineValue("OnAccessMaxFileSize",setupFile->getSectionValue("OnAccess", "OnAccessMaxFileSize")+"M");
    clamdConf->addSingleLineValue("OnAccessMaxThreads",setupFile->getSectionValue("OnAccess", "OnAccessMaxThreads"));
    clamdConf->addSingleLineValue("OnAccessPrevention",setupFile->getSectionValue("OnAccess", "OnAccessPrevention"));
    clamdConf->addSingleLineValue("OnAccessDenyOnError",setupFile->getSectionValue("OnAccess", "OnAccessDenyOnError"));
    clamdConf->addSingleLineValue("OnAccessExtraScanning",setupFile->getSectionValue("OnAccess", "OnAccessExtraScanning"));
    clamdConf->addSingleLineValue("OnAccessRetryAttempts",setupFile->getSectionValue("OnAccess", "OnAccessRetryAttempts"));
    clamdConf->addSingleLineValue("OnAccessExcludeUname","root");
    clamdConf->addSingleLineValue("OnAccessExcludeUID","0");
    QStringList watchList = setupFile->getKeywords("Clamonacc");
    foreach (QString entry, watchList) {
        clamdConf->addSingleLineValue("OnAccessIncludePath",entry);
    }

    if (helperHandler->keywordExists("SelectedOptions","--alert-broken-media<equal>yes") == true) clamdConf->setSingleLineValue("AlertBrokenMedia", "yes");
    if (helperHandler->keywordExists("SelectedOptions","--alert-broken-media<equal>no") == true) clamdConf->setSingleLineValue("AlertBrokenMedia", "no");
    if (helperHandler->keywordExists("SelectedOptions","--alert-broken<equal>yes") == true) clamdConf->setSingleLineValue("AlertBrokenExecutables", "yes");
    if (helperHandler->keywordExists("SelectedOptions","--alert-broken<equal>no") == true) clamdConf->setSingleLineValue("AlertBrokenExecutables", "no");
    if (helperHandler->keywordExists("SelectedOptions","--alert-encrypted-archive<equal>yes") == true) clamdConf->setSingleLineValue("AlertEncryptedArchive", "yes");
    if (helperHandler->keywordExists("SelectedOptions","--alert-encrypted-archive<equal>no") == true) clamdConf->setSingleLineValue("AlertEncryptedArchive", "no");
    if (helperHandler->keywordExists("SelectedOptions","--alert-encrypted-doc<equal>yes") == true) clamdConf->setSingleLineValue("AlertEncryptedDoc", "yes");
    if (helperHandler->keywordExists("SelectedOptions","--alert-encrypted-doc<equal>no") == true) clamdConf->setSingleLineValue("AlertEncryptedDoc", "no");
    if (helperHandler->keywordExists("SelectedOptions","--alert-encrypted<equal>yes") == true) clamdConf->setSingleLineValue("AlertEncrypted", "yes");
    if (helperHandler->keywordExists("SelectedOptions","--alert-encrypted<equal>no") == true) clamdConf->setSingleLineValue("AlertEncrypted", "no");
    if (helperHandler->keywordExists("SelectedOptions","--alert-exceeds-max<equal>yes") == true) clamdConf->setSingleLineValue("AlertExceedsMax", "yes");
    if (helperHandler->keywordExists("SelectedOptions","--alert-exceeds-max<equal>no") == true) clamdConf->setSingleLineValue("AlertExceedsMax", "no");
    if (helperHandler->keywordExists("SelectedOptions","--alert-macros<equal>yes") == true) clamdConf->setSingleLineValue("AlertOLE2Macros", "yes");
    if (helperHandler->keywordExists("SelectedOptions","--alert-macros<equal>no") == true) clamdConf->setSingleLineValue("AlertOLE2Macros", "no");
    if (helperHandler->keywordExists("SelectedOptions","--alert-partition-intersection<equal>yes") == true) clamdConf->setSingleLineValue("AlertPartitionIntersection", "yes");
    if (helperHandler->keywordExists("SelectedOptions","--alert-partition-intersection<equal>no") == true) clamdConf->setSingleLineValue("AlertPartitionIntersection", "no");
    if (helperHandler->keywordExists("SelectedOptions","--alert-phishing-cloak<equal>yes") == true) clamdConf->setSingleLineValue("AlertPhishingCloak", "yes");
    if (helperHandler->keywordExists("SelectedOptions","--alert-phishing-cloak<equal>no") == true) clamdConf->setSingleLineValue("AlertPhishingCloak", "no");
    if (helperHandler->keywordExists("SelectedOptions","--alert-phishing-ssl<equal>yes") == true) clamdConf->setSingleLineValue("AlertPhishingSSLMismatch", "yes");
    if (helperHandler->keywordExists("SelectedOptions","--alert-phishing-ssl<equal>no") == true) clamdConf->setSingleLineValue("AlertPhishingSSLMismatch", "no");
    if (helperHandler->keywordExists("SelectedOptions","--allmatch<equal>yes") == true) clamdConf->setSingleLineValue("AllowAllMatchScan", "yes");
    if (helperHandler->keywordExists("SelectedOptions","--allmatch<equal>no") == true) clamdConf->setSingleLineValue("AllowAllMatchScan", "no");
    if (helperHandler->keywordExists("SelectedOptions","--bytecode-unsigned<equal>yes") == true) clamdConf->setSingleLineValue("BytecodeUnsigned", "yes");
    if (helperHandler->keywordExists("SelectedOptions","--bytecode-unsigned<equal>no") == true) clamdConf->setSingleLineValue("BytecodeUnsigned", "no");
    if (helperHandler->keywordExists("SelectedOptions","--cross-fs<equal>yes") == true) clamdConf->setSingleLineValue("CrossFilesystems", "yes");
    if (helperHandler->keywordExists("SelectedOptions","--cross-fs<equal>no") == true) clamdConf->setSingleLineValue("CrossFilesystems", "no");
    if (helperHandler->keywordExists("SelectedOptions","--debug") == true) clamdConf->setSingleLineValue("Debug", "yes");
    if (helperHandler->keywordExists("SelectedOptions","--detect-pua<equal>yes") == true) clamdConf->setSingleLineValue("DetectPUA","yes");
    if (helperHandler->keywordExists("SelectedOptions","--detect-pua<equal>no") == true) clamdConf->setSingleLineValue("DetectPUA", "no");
    if (helperHandler->keywordExists("SelectedOptions","--detect-structured<equal>yes") == true) clamdConf->setSingleLineValue("StructuredDataDetection", "yes");
    if (helperHandler->keywordExists("SelectedOptions","--detect-structured<equal>no") == true) clamdConf->setSingleLineValue("StructuredDataDetection", "no");
    if (helperHandler->keywordExists("SelectedOptions","--heuristic-alerts<equal>yes") == true) clamdConf->setSingleLineValue("HeuristicAlerts", "yes");
    if (helperHandler->keywordExists("SelectedOptions","--heuristic-alerts<equal>no") == true) clamdConf->setSingleLineValue("HeuristicAlerts", "no");
    if (helperHandler->keywordExists("SelectedOptions","--heuristic-scan-precedence<equal>yes") == true) clamdConf->setSingleLineValue("HeuristicScanPrecedence", "yes");
    if (helperHandler->keywordExists("SelectedOptions","--heuristic-scan-precedence<equal>no") == true) clamdConf->setSingleLineValue("HeuristicScanPrecedence", "no");
    if (helperHandler->keywordExists("SelectedOptions","--leave-temps<equal>yes") == true) clamdConf->setSingleLineValue("LeaveTemporaryFiles", "yes");
    if (helperHandler->keywordExists("SelectedOptions","--leave-temps<equal>no") == true) clamdConf->setSingleLineValue("LeaveTemporaryFiles", "no");
    if (helperHandler->keywordExists("SelectedOptions","--nocerts") == true) clamdConf->setSingleLineValue("DisableCertCheck", "yes");
    if (helperHandler->keywordExists("SelectedOptions","--official-db-only<equal>yes") == true) clamdConf->setSingleLineValue("OfficialDatabaseOnly", "yes");
    if (helperHandler->keywordExists("SelectedOptions","--official-db-only<equal>no") == true) clamdConf->setSingleLineValue("OfficialDatabaseOnly", "no");
    if (helperHandler->keywordExists("SelectedOptions","--phishing-scan-urls<equal>yes") == true) clamdConf->setSingleLineValue("PhishingScanURLs", "yes");
    if (helperHandler->keywordExists("SelectedOptions","--phishing-scan-urls<equal>no") == true) clamdConf->setSingleLineValue("PhishingScanURLs", "no");
    if (helperHandler->keywordExists("SelectedOptions","--phishing-sigs<equal>yes") == true) clamdConf->setSingleLineValue("PhishingSignatures", "yes");
    if (helperHandler->keywordExists("SelectedOptions","--phishing-sigs<equal>no") == true) clamdConf->setSingleLineValue("PhishingSignatures", "no");
    if (helperHandler->keywordExists("SelectedOptions","--scan-archive<equal>yes") == true) clamdConf->setSingleLineValue("ScanArchive", "yes");
    if (helperHandler->keywordExists("SelectedOptions","--scan-archive<equal>no") == true) clamdConf->setSingleLineValue("ScanArchive", "no");
    if (helperHandler->keywordExists("SelectedOptions","--scan-elf<equal>yes") == true) clamdConf->setSingleLineValue("ScanELF", "yes");
    if (helperHandler->keywordExists("SelectedOptions","--scan-elf<equal>no") == true) clamdConf->setSingleLineValue("ScanELF", "no");
    if (helperHandler->keywordExists("SelectedOptions","--scan-html<equal>no") == true) clamdConf->setSingleLineValue("ScanHTML", "yes");
    if (helperHandler->keywordExists("SelectedOptions","--scan-html<equal>no") == true) clamdConf->setSingleLineValue("ScanHTML", "no");
    if (helperHandler->keywordExists("SelectedOptions","--scan-hwp3<equal>yes") == true) clamdConf->setSingleLineValue("ScanHWP3", "yes");
    if (helperHandler->keywordExists("SelectedOptions","--scan-hwp3<equal>no") == true) clamdConf->setSingleLineValue("ScanHWP3", "no");
    if (helperHandler->keywordExists("SelectedOptions","--scan-mail<equal>yes") == true) clamdConf->setSingleLineValue("ScanMail", "yes");
    if (helperHandler->keywordExists("SelectedOptions","--scan-mail<equal>no") == true) clamdConf->setSingleLineValue("ScanMail", "no");
    if (helperHandler->keywordExists("SelectedOptions","--scan-ole2<equal>yes") == true) clamdConf->setSingleLineValue("ScanOLE2", "yes");
    if (helperHandler->keywordExists("SelectedOptions","--scan-ole2<equal>no") == true) clamdConf->setSingleLineValue("ScanOLE2", "no");
    if (helperHandler->keywordExists("SelectedOptions","--scan-pe<equal>yes") == true) clamdConf->setSingleLineValue("ScanPDF", "yes");
    if (helperHandler->keywordExists("SelectedOptions","--scan-pe<equal>no") == true) clamdConf->setSingleLineValue("ScanPDF", "no");
    if (helperHandler->keywordExists("SelectedOptions","--scan-swf<equal>yes") == true) clamdConf->setSingleLineValue("ScanSWF", "yes");
    if (helperHandler->keywordExists("SelectedOptions","--scan-swf<equal>no") == true) clamdConf->setSingleLineValue("ScanSWF", "no");
    if (helperHandler->keywordExists("SelectedOptions","--scan-xmldocs<equal>yes") == true) clamdConf->setSingleLineValue("ScanXMLDOCS", "yes");
    if (helperHandler->keywordExists("SelectedOptions","--scan-xmldocs<equal>no") == true) clamdConf->setSingleLineValue("ScanXMLDOCS", "no");
    if (helperHandler->keywordExists("SelectedOptions","--verbose") == true) clamdConf->setSingleLineValue("LogVerbose", "yes");

    if ((helperHandler->sectionExists("REGEXP_and_IncludeExclude")) && (helperHandler->getSectionValue("REGEXP_and_IncludeExclude","DontScanDiretoriesMatchingRegExp").indexOf("checked|") == 0 )) clamdConf->addSingleLineValue("ExcludePath",helperHandler->getSectionValue("REGEXP_and_IncludeExclude","DontScanDiretoriesMatchingRegExp").mid(8));
    if ((helperHandler->sectionExists("REGEXP_and_IncludeExclude")) && (helperHandler->getSectionValue("REGEXP_and_IncludeExclude","DontScanFileNamesMatchingRegExp").indexOf("checked|") == 0 )) clamdConf->addSingleLineValue("ExcludePath",helperHandler->getSectionValue("REGEXP_and_IncludeExclude","DontScanDiretoriesMatchingRegExp").mid(8));
    if (helperHandler->getSectionBoolValue("REGEXP_and_IncludeExclude","EnablePUAOptions") == true) {
        if (helperHandler->getSectionBoolValue("REGEXP_and_IncludeExclude","LoadPUAPacked") == true) clamdConf->addSingleLineValue("IncludePUA","Packed");
        if (helperHandler->getSectionBoolValue("REGEXP_and_IncludeExclude","LoadPUAPWTool") == true) clamdConf->addSingleLineValue("IncludePUA","PWTool");
        if (helperHandler->getSectionBoolValue("REGEXP_and_IncludeExclude","LoadPUANetTool") == true) clamdConf->addSingleLineValue("IncludePUA","NetTool");
        if (helperHandler->getSectionBoolValue("REGEXP_and_IncludeExclude","LoadPUAP2P") == true) clamdConf->addSingleLineValue("IncludePUA","P2P");
        if (helperHandler->getSectionBoolValue("REGEXP_and_IncludeExclude","LoadPUAIRC") == true) clamdConf->addSingleLineValue("IncludePUA","IRC");
        if (helperHandler->getSectionBoolValue("REGEXP_and_IncludeExclude","LoadPUARAT") == true) clamdConf->addSingleLineValue("IncludePUA","RAT");
        if (helperHandler->getSectionBoolValue("REGEXP_and_IncludeExclude","LoadPUANetToolSpy") == true) clamdConf->addSingleLineValue("IncludePUA","NetToolSpy");
        if (helperHandler->getSectionBoolValue("REGEXP_and_IncludeExclude","LoadPUAServer") == true) clamdConf->addSingleLineValue("IncludePUA","Server");
        if (helperHandler->getSectionBoolValue("REGEXP_and_IncludeExclude","LoadPUAScript") == true) clamdConf->addSingleLineValue("IncludePUA","Script");
        if (helperHandler->getSectionBoolValue("REGEXP_and_IncludeExclude","LoadPUAAndr") == true) clamdConf->addSingleLineValue("IncludePUA","Andr");
        if (helperHandler->getSectionBoolValue("REGEXP_and_IncludeExclude","LoadPUAJava") == true) clamdConf->addSingleLineValue("IncludePUA","Java");
        if (helperHandler->getSectionBoolValue("REGEXP_and_IncludeExclude","LoadPUAOsx") == true) clamdConf->addSingleLineValue("IncludePUA","Osx");
        if (helperHandler->getSectionBoolValue("REGEXP_and_IncludeExclude","LoadPUATool") == true) clamdConf->addSingleLineValue("IncludePUA","Tool");
        if (helperHandler->getSectionBoolValue("REGEXP_and_IncludeExclude","LoadPUAUnix") == true) clamdConf->addSingleLineValue("IncludePUA","Unix");
        if (helperHandler->getSectionBoolValue("REGEXP_and_IncludeExclude","LoadPUAWin") == true) clamdConf->addSingleLineValue("IncludePUA","Win");
    }
    if (helperHandler->getSectionValue("ScanLimitations","Files larger than this will be skipped and assumed clean").indexOf("checked|") == 0) clamdConf->addSingleLineValue("MaxFileSize",helperHandler->getSectionValue("ScanLimitations","Files larger than this will be skipped and assumed clean").mid(8));
    if (helperHandler->getSectionValue("ScanLimitations","The maximum amount of data to scan for each container file").indexOf("checked|") == 0) clamdConf->addSingleLineValue("MaxScanSize",helperHandler->getSectionValue("ScanLimitations","The maximum amount of data to scan for each container file").mid(8));
    if (helperHandler->getSectionValue("ScanLimitations","The maximum number of files to scan for each container file").indexOf("checked|") == 0) clamdConf->addSingleLineValue("MaxFiles",helperHandler->getSectionValue("ScanLimitations","The maximum number of files to scan for each container file").mid(8));
    if (helperHandler->getSectionValue("ScanLimitations","Max Scan-Time").indexOf("checked|") == 0) clamdConf->addSingleLineValue("MaxScanTime",helperHandler->getSectionValue("ScanLimitations","Max Scan-Time").mid(8));
    if (helperHandler->getSectionValue("ScanLimitations","Maximum directory recursion level").indexOf("checked|") == 0) clamdConf->addSingleLineValue("MaxRecursion",helperHandler->getSectionValue("ScanLimitations","Maximum directory recursion level").mid(8));
    if (helperHandler->getSectionValue("ScanLimitations","Maximum size file to check for embedded PE").indexOf("checked|") == 0) clamdConf->addSingleLineValue("MaxEmbeddedPE",helperHandler->getSectionValue("ScanLimitations","Maximum size file to check for embedded PE").mid(8));
    if (helperHandler->getSectionValue("ScanLimitations","Maximum size of HTML file to normalize").indexOf("checked|") == 0) clamdConf->addSingleLineValue("MaxHTMLNormalize",helperHandler->getSectionValue("ScanLimitations","Maximum size of HTML file to normalize").mid(8));
    if (helperHandler->getSectionValue("ScanLimitations","Maximum size of normalized HTML file to scan").indexOf("checked|") == 0) clamdConf->addSingleLineValue("MaxHTMLNoTags",helperHandler->getSectionValue("ScanLimitations","Maximum size of normalized HTML file to scan").mid(8));
    if (helperHandler->getSectionValue("ScanLimitations","Maximum size of script file to normalize").indexOf("checked|") == 0) clamdConf->addSingleLineValue("MaxScriptNormalize",helperHandler->getSectionValue("ScanLimitations","Maximum size of script file to normalize").mid(8));
    if (helperHandler->getSectionValue("ScanLimitations","Maximum size zip to type reanalyze").indexOf("checked|") == 0) clamdConf->addSingleLineValue("MaxZipTypeRcg",helperHandler->getSectionValue("ScanLimitations","Maximum size zip to type reanalyze").mid(8));
    if (helperHandler->getSectionValue("ScanLimitations","Maximum number of partitions in disk image to be scanned").indexOf("checked|") == 0) clamdConf->addSingleLineValue("MaxPartitions",helperHandler->getSectionValue("ScanLimitations","Maximum number of partitions in disk image to be scanned").mid(8));
    if (helperHandler->getSectionValue("ScanLimitations","Maximum number of icons in PE file to be scanned").indexOf("checked|") == 0) clamdConf->addSingleLineValue("MaxIconsPE",helperHandler->getSectionValue("ScanLimitations","Maximum number of icons in PE file to be scanned").mid(8));
    if (helperHandler->getSectionValue("ScanLimitations","Max recursion to HWP3 parsing function").indexOf("checked|") == 0) clamdConf->addSingleLineValue("MaxRecHWP3",helperHandler->getSectionValue("ScanLimitations","Max recursion to HWP3 parsing function").mid(8));
    if (helperHandler->getSectionValue("ScanLimitations","Max calls to PCRE match function").indexOf("checked|") == 0) clamdConf->addSingleLineValue("PCREMatchLimit",helperHandler->getSectionValue("ScanLimitations","Max calls to PCRE match function").mid(8));
    if (helperHandler->getSectionValue("ScanLimitations","Max recursion calls to the PCRE match function").indexOf("checked|") == 0) clamdConf->addSingleLineValue("PCRERecMatchLimit",helperHandler->getSectionValue("ScanLimitations","Max recursion calls to the PCRE match function").mid(8));
    if (helperHandler->getSectionValue("ScanLimitations","Max PCRE file size").indexOf("checked|") == 0) clamdConf->addSingleLineValue("PCREMaxFileSize",helperHandler->getSectionValue("ScanLimitations","Max PCRE file size").mid(8));
    if (helperHandler->getSectionValue("ScanLimitations","Structured CC Count").indexOf("checked|") == 0) clamdConf->addSingleLineValue("StructuredMinCreditCardCount",helperHandler->getSectionValue("ScanLimitations","Structured CC Count").mid(8));
    if ((helperHandler->getSectionValue("ScanLimitations","Structured CC Mode").indexOf("checked|") == 0) && (helperHandler->getSectionValue("ScanLimitations","Structured CC Mode").right(1) == "1")) clamdConf->addSingleLineValue("StructuredCCOnly","yes");
    if (helperHandler->getSectionValue("ScanLimitations","Structured SSN Count").indexOf("checked|") == 0) clamdConf->addSingleLineValue("StructuredMinSSNCount",helperHandler->getSectionValue("ScanLimitations","Structured SSN Count").mid(8));
    if ((helperHandler->getSectionValue("ScanLimitations","Structured SSN Format").indexOf("checked|") == 0) && (helperHandler->getSectionValue("ScanLimitations","Structured SSN Format").right(1) == "0")) clamdConf->addSingleLineValue("StructuredSSNFormatNormal","yes");
    if ((helperHandler->getSectionValue("ScanLimitations","Structured SSN Format").indexOf("checked|") == 0) && (helperHandler->getSectionValue("ScanLimitations","Structured SSN Format").right(1) == "1")) clamdConf->addSingleLineValue("StructuredSSNFormatStripped","yes");
    if ((helperHandler->getSectionValue("ScanLimitations","Structured SSN Format").indexOf("checked|") == 0) && (helperHandler->getSectionValue("ScanLimitations","Structured SSN Format").right(1) == "2")) {
        clamdConf->addSingleLineValue("StructuredSSNFormatNormal","yes");
        clamdConf->addSingleLineValue("StructuredSSNFormatStripped","yes");
    }
    if (helperHandler->getSectionValue("ScanLimitations","Bytecode timeout in milliseconds").indexOf("checked|") == 0 ) clamdConf->addSingleLineValue("BytecodeTimeout",helperHandler->getSectionValue("ScanLimitations","Bytecode timeout in milliseconds").mid(8));
}

void clamdManager::slot_logFileContentChanged()
{
    QFile file(QDir::homePath() + "/.clamav-gui/clamd.log");
    QString content;
    QString checkString;

    file.open(QIODevice::ReadOnly);
    QTextStream stream(&file);
    content = stream.readAll();
    file.close();

    QStringList lines =  content.split("\n");
    foreach (QString line, lines) {
        if ((line.indexOf("/") == 0) && (line.indexOf("FOUND") == line.length() - 5)) checkString = line;
    }

    if ((checkString != lastFound) && (checkString != "")) {
        lastFound = checkString;
        emit setBallonMessage(2,tr("WARNING"),lastFound);
    }

    ui->clamdLogPlainTextEdit->clear();
    ui->clamdLogPlainTextEdit->insertPlainText(content);
    ui->clamdLogPlainTextEdit->ensureCursorVisible();
}

void clamdManager::slot_clamdStartStopButtonClicked()
{
    QStringList monitorings = setupFile->getKeywords("Clamonacc");
    if (monitorings.length() > 0) {
        QStringList parameters;
        QFile pidFile("/tmp/clamd.pid");
        QString clamonaccOptions;
        int value = setupFile->getSectionIntValue("OnAccess","InfectedFiles");
        if (value == 1) clamonaccOptions = " --copy=" + QDir::homePath() + "/.clamav-gui/quarantine";
        if (value == 2) clamonaccOptions = " --move=" + QDir::homePath() + "/.clamav-gui/quarantine";
        if (value == 3) clamonaccOptions = " --remove=";

        if (checkClamdRunning() == false) {
            slot_updateClamdConf();
            ui->clamdIconLabel->setMovie(new QMovie(":/icons/icons/gifs/spinning_segments_small.gif"));
            ui->clamdIconLabel->movie()->start();
            clamdLogWatcher->removePath(QDir::homePath() + "/.clamav-gui/clamd.log");
            QFile logFile(QDir::homePath() + "/.clamav-gui/clamd.log");
            if (logFile.exists() == true) logFile.remove();
            logFile.open(QIODevice::ReadWrite);
            QTextStream stream(&logFile);
            stream << "";
            logFile.close();
            clamdLogWatcher->addPath(QDir::homePath() + "/.clamav-gui/clamd.log");
            ui->clamdLogPlainTextEdit->clear();
            ui->startStopClamdPushButton->setText(tr("  Clamd starting. Please wait!"));
            QFile startclamdFile(QDir::homePath() + "/.clamav-gui/startclamd.sh");
            if (startclamdFile.exists() == true) startclamdFile.remove();
            if (startclamdFile.open(QIODevice::Text|QIODevice::ReadWrite)){
                QString logFile = clamdConf->getSingleLineValue("LogFile");
                QTextStream stream(&startclamdFile);
                stream << "#!/bin/bash\n" << clamdLocation + " 2> " + logFile + " -c " + QDir::homePath() + "/.clamav-gui/clamd.conf && " + clamonaccLocation + " -c " + QDir::homePath() + "/.clamav-gui/clamd.conf -l " + QDir::homePath() + "/.clamav-gui/clamd.log" + clamonaccOptions;
                startclamdFile.close();
                startclamdFile.setPermissions(QFileDevice::ReadOwner|QFileDevice::WriteOwner|QFileDevice::ExeOwner|QFileDevice::ReadGroup|QFileDevice::WriteGroup|QFileDevice::ExeGroup);
            }
            parameters << QDir::homePath() + "/.clamav-gui/startclamd.sh";
            if (sudoGUI == "") sudoGUI = setupFile->getSectionValue("Settings","SudoGUI");
            startClamdProcess->start(sudoGUI,parameters);
            setupFile->setSectionValue("Clamd","Status","starting up ...");
            emit systemStatusChanged();
        } else {
            setupFile->setSectionValue("Clamd","Status","shutting down ...");
            emit systemStatusChanged();
            pidFile.open(QIODevice::ReadOnly);
            QTextStream stream(&pidFile);
            QString pid = stream.readLine();
            pidFile.close();
            ui->startStopClamdPushButton->setText(tr("  Stopping Clamd. Please wait!"));
            QFile stopclamdFile(QDir::homePath() + "/.clamav-gui/stopclamd.sh");
            if (stopclamdFile.exists() == true) stopclamdFile.remove();
            if (stopclamdFile.open(QIODevice::Text|QIODevice::ReadWrite)){
                QTextStream stream(&stopclamdFile);
                stream << "#!/bin/bash\n/bin/kill -sigterm " + pid + " && kill -9 " + clamonaccPid;
                stopclamdFile.close();
                stopclamdFile.setPermissions(QFileDevice::ReadOwner|QFileDevice::WriteOwner|QFileDevice::ExeOwner|QFileDevice::ReadGroup|QFileDevice::WriteGroup|QFileDevice::ExeGroup);
            }
            parameters << QDir::homePath() + "/.clamav-gui/stopclamd.sh";
            if (sudoGUI == "") sudoGUI = setupFile->getSectionValue("Settings","SudoGUI");
            killProcess->start(sudoGUI,parameters);
        }
        ui->startStopClamdPushButton->setEnabled(false);
        ui->monitoringAddButton->setEnabled(false);
        ui->monitoringDelButton->setEnabled(false);
    } else {
        clamdStartupCounter = 0;
        QMessageBox::warning(this,tr("WARNING"),tr("Clamd and Clamonacc can not be launched. First you have to add at least one folder for monitoring!"));
    }
}

void clamdManager::slot_startClamdProcessFinished(int exitCode, QProcess::ExitStatus exitStatus) {
    if ((exitCode != 0) || (exitStatus == QProcess::CrashExit))clamdStartupCounter = 0;
    ui->clamdIconLabel->clear();
    ui->clamdIconLabel->setPixmap(QPixmap(":/icons/icons/onaccess.png"));
    ui->clamdIconLabel_2->clear();;
    ui->clamdIconLabel_2->setPixmap(QPixmap(":/icons/icons/options.png"));
    if (checkClamdRunning() == false) {
        clamdPidWatcher->removePath("/tmp/clamd.pid");
        ui->startStopClamdPushButton->setStyleSheet("background-color:red;color:yellow");
        ui->startStopClamdPushButton->setText(tr("  Clamd not running - Start Clamd"));
        ui->startStopClamdPushButton->setIcon(QIcon(":/icons/icons/startclamd.png"));
        setupFile->setSectionValue("Clamd","ClamdPid","n/a");
        setupFile->setSectionValue("Clamd","ClamonaccPid","n/a");
        setupFile->setSectionValue("Clamd","Status","not running");
        emit systemStatusChanged();
        if (clamdStartupCounter > 0) {
            if (waitForFreshclam == false) clamdStartupCounter--;
            startDelayTimer->start(2500);
        }
    } else {
        clamdStartupCounter = 0;
        clamdPidWatcher->addPath("/tmp/clamd.pid");
        QFile pidFile("/tmp/clamd.pid");
        pidFile.open(QIODevice::ReadOnly);
        QTextStream stream(&pidFile);
        QString pid = stream.readLine();
        pid = pid.replace("\n","");
        setupFile->setSectionValue("Clamd","ClamdPid",pid);
        setupFile->setSectionValue("Clamd","Status","is running");
        pidFile.close();
        emit systemStatusChanged();
        ui->startStopClamdPushButton->setStyleSheet("background-color:green;color:yellow");
        ui->startStopClamdPushButton->setText(tr("  Clamd running - Stop Clamd"));
        ui->startStopClamdPushButton->setIcon(QIcon(":/icons/icons/stopclamd.png"));
        slot_logFileContentChanged();
        QStringList parameters;
        parameters << "-s" << "clamonacc";
        findclamonaccProcess->start("pidof",parameters);
    }
    clamdRestartInProgress = false;
    ui->startStopClamdPushButton->setEnabled(true);
    ui->monitoringAddButton->setEnabled(true);
    ui->monitoringDelButton->setEnabled(true);
    ui->restartClamdPushButton->setVisible(false);
}

void clamdManager::slot_killClamdProcessFinished()
{
    ui->clamdIconLabel->clear();
    ui->clamdIconLabel->setPixmap(QPixmap(":/icons/icons/onaccess.png"));
    ui->clamdIconLabel_2->clear();;
    ui->clamdIconLabel_2->setPixmap(QPixmap(":/icons/icons/options.png"));
    if (checkClamdRunning() == false) {
        clamdPidWatcher->removePath("/tmp/clamd.pid");
        ui->startStopClamdPushButton->setStyleSheet("background-color:red;color:yellow");
        ui->startStopClamdPushButton->setText(tr("  Clamd not running - Start Clamd"));
        ui->startStopClamdPushButton->setIcon(QIcon(":/icons/icons/startclamd.png"));
        setupFile->setSectionValue("Clamd","ClamdPid","n/a");
        setupFile->setSectionValue("Clamd","ClamonaccPid","n/a");
        setupFile->setSectionValue("Clamd","Status","shut down");
        emit systemStatusChanged();
    } else {
        clamdPidWatcher->addPath("/tmp/clamd.pid");
        ui->startStopClamdPushButton->setStyleSheet("background-color:green;color:yellow");
        ui->startStopClamdPushButton->setText(tr("  Clamd running - Stop Clamd"));
        ui->startStopClamdPushButton->setIcon(QIcon(":/icons/icons/stopclamd.png"));
    }
    ui->startStopClamdPushButton->setEnabled(true);
    ui->monitoringAddButton->setEnabled(true);
    ui->monitoringDelButton->setEnabled(true);
}

void clamdManager::slot_findclamonaccProcessFinished(int rc)
{
    if (rc == 0) {
        clamonaccPid = findclamonaccProcess->readAllStandardOutput();
        clamonaccPid = clamonaccPid.replace("\n","");
        setupFile->setSectionValue("Clamd","ClamonaccPid",clamonaccPid);
        emit systemStatusChanged();
    } else {
        setupFile->setSectionValue("Clamd","ClamonaccPid","n/a");
        emit systemStatusChanged();
    }

    if ((setupFile->keywordExists("Clamd","StartClamdOnStartup") == true) && (setupFile->getSectionBoolValue("Clamd","StartClamdOnStartup") == true)) {
        ui->startClamdOnStartupCheckBox->setChecked(setupFile->getSectionBoolValue("Clamd","StartClamdOnStartup"));
    }
}

void clamdManager::slot_startDelayTimerExpired() {
    if (checkClamdRunning() == false) {
        emit setActiveTab();
        slot_clamdStartStopButtonClicked();
    }
}

void clamdManager::slot_waitForFreshclamStarted()
{
    waitForFreshclam = false;

    if ((startup == true) && (setupFile->keywordExists("Clamd","StartClamdOnStartup") == true) && (setupFile->getSectionBoolValue("Clamd","StartClamdOnStartup") == true)) {
        ui->startClamdOnStartupCheckBox->setChecked(setupFile->getSectionBoolValue("Clamd","StartClamdOnStartup"));
        clamdStartupCounter = 5;
        if (startup == true) startDelayTimer->start(2500);
        startup = false;
    }
}


void clamdManager::slot_processWatcherExpired()
{
    QString clamdPid = setupFile->getSectionValue("Clamd","ClamdPid");
    QString clamonaccPid = setupFile->getSectionValue("Clamd","ClamonaccPid");

    QDir checkDir;
    if ( clamdPid != "n/a") {
        if (checkDir.exists("/proc/" + clamdPid) == false) {
            setupFile->setSectionValue("Clamd","ClamdPid","n/a");
            emit systemStatusChanged();
            slot_killClamdProcessFinished();
        }
    }

    if ( clamonaccPid != "n/a") {
        if (checkDir.exists("/proc/" + clamonaccPid) == false) {
            setupFile->setSectionValue("Clamd","ClamonaccPid","n/a");
            emit systemStatusChanged();
        }
    }
}


void clamdManager::slot_pidWatcherTriggered()
{
    QFile pidFile("/tmp/clamd.pid");
    if ((pidFile.exists() == false) && (clamdRestartInProgress == false)){
        clamdPidWatcher->removePath("/tmp/clamd.pid");
        ui->startStopClamdPushButton->setStyleSheet("background-color:red;color:yellow");
        ui->startStopClamdPushButton->setText(tr("  Clamd not running - Start Clamd"));
        ui->startStopClamdPushButton->setIcon(QIcon(":/icons/icons/startclamd.png"));
        ui->startStopClamdPushButton->setEnabled(true);
        ui->monitoringAddButton->setEnabled(true);
        ui->monitoringDelButton->setEnabled(true);
        setupFile->setSectionValue("Clamd","ClamdPid","n/a");
        setupFile->setSectionValue("Clamd","ClamonaccPid","n/a");
        setupFile->setSectionValue("Clamd","Status","not running");
        emit systemStatusChanged();
    }
}

void clamdManager::slot_clamdLocationProcessFinished()
{
    QString searchstring = "clamd:";
    QString output = clamdLocationProcess->readAll();
    if (output == searchstring + "\n") {
        ui->startStopClamdPushButton->setEnabled(false);
        ui->monitoringAddButton->setEnabled(false);
        ui->monitoringDelButton->setEnabled(false);
        setupFile->setSectionValue("Clamd","ClamdLocation","n/a");
        emit systemStatusChanged();
    } else {
        clamdLocation = trimLocationOutput(output);
        setupFile->setSectionValue("Clamd","ClamdLocation",clamdLocation);
        emit systemStatusChanged();
    }
}

void clamdManager::slot_clamonaccLocationProcessFinished()
{
    QString searchstring = "clamonacc:";
    QString output = clamonaccLocationProcess->readAll();
    if (output == searchstring + "\n") {
        ui->startStopClamdPushButton->setEnabled(false);
        ui->monitoringAddButton->setEnabled(false);
        ui->monitoringDelButton->setEnabled(false);
        setupFile->setSectionValue("Clamd","ClamonaccLocation","n/a");
        emit systemStatusChanged();
    } else {
        clamonaccLocation = trimLocationOutput(output);
        setupFile->setSectionValue("Clamd","ClamonaccLocation",clamonaccLocation);
        emit systemStatusChanged();

        QStringList parameters;
        parameters << "-s" << "clamonacc";
        findclamonaccProcess->start("pidof",parameters);
    }
}

void clamdManager::slot_startClamdOnStartupCheckBoxClicked()
{
    setupFile->setSectionValue("Clamd","StartClamdOnStartup",ui->startClamdOnStartupCheckBox->isChecked());
}

void clamdManager::slot_monitoringAddButtonClicked()
{
    QString path = QFileDialog::getExistingDirectory(this,tr("Directory to monitor"),QDir::homePath());
    if (path != "") {
        if (ui->monitoringComboBox->findText(path) == -1) {
            ui->monitoringComboBox->addItem(path);
            setupFile->setSectionValue("Clamonacc",path,"under monitoring");
            clamdConf->addSingleLineValue("OnAccessIncludePath",path);
            if (checkClamdRunning() == true) restartClamonacc();
        } else {
            QMessageBox::warning(this,tr("WARNING"),tr("Path already under monitoring"));
        }
    }
}

void clamdManager::slot_monitoringDelButtonClicked()
{
    QString entry = ui->monitoringComboBox->currentText();
    if (entry != "") {
        if (QMessageBox::information(this,tr("Remove Folder from monitoring"),tr("Path: ") + entry + "\n" + tr("Do you want to remove the folder from the monitoring list?"),QMessageBox::Yes,QMessageBox::No) == QMessageBox::Yes) {
            setupFile->removeKeyword("Clamonacc",entry);
            ui->monitoringComboBox->removeItem(ui->monitoringComboBox->currentIndex());
            clamdConf->removeSingleLine("OnAccessIncludePath",entry);
            if (checkClamdRunning() == true) restartClamonacc();
        }
    }
}

void clamdManager::slot_restartClamonaccProcessFinished()
{
    QStringList parameters;
    parameters << "-s" << "clamonacc";
    findclamonaccProcess->start("pidof",parameters);
}

void clamdManager::slot_restartClamdButtonClicked()
{
    QFile pidFile("/tmp/clamd.pid");
    pidFile.open(QIODevice::ReadOnly);
    QTextStream stream(&pidFile);
    QString pid = stream.readLine();
    pidFile.close();
    setupFile->setSectionValue("Clamd","ClamdPid",pid);

    QString clamonaccOptions;
    int value = setupFile->getSectionIntValue("OnAccess","InfectedFiles");
    if (value == 1) clamonaccOptions = " --copy=" + QDir::homePath() + "/.clamav-gui/quarantine";
    if (value == 2) clamonaccOptions = " --move=" + QDir::homePath() + "/.clamav-gui/quarantine";
    if (value == 3) clamonaccOptions = " --remove=";

    clamdRestartInProgress = true;
    ui->startStopClamdPushButton->setEnabled(false);
    ui->monitoringAddButton->setEnabled(false);
    ui->monitoringDelButton->setEnabled(false);
    ui->startStopClamdPushButton->setText(tr("  Clamd restarting. Please wait!"));
    ui->clamdIconLabel_2->setMovie(new QMovie(":/icons/icons/gifs/spinning_segments_small.gif"));
    ui->clamdIconLabel_2->movie()->start();
    QStringList parameters;

    QFile startclamdFile(QDir::homePath() + "/.clamav-gui/startclamd.sh");
    startclamdFile.remove();
    if (startclamdFile.open(QIODevice::Text|QIODevice::ReadWrite)){
        QTextStream stream(&startclamdFile);
        stream << "#!/bin/bash\n" << "kill -sigterm " + pid + " && kill -9 " + clamonaccPid + " && sleep 20 && " + clamdLocation + " -c " + QDir::homePath() + "/.clamav-gui/clamd.conf && " + clamonaccLocation + " -c " + QDir::homePath() + "/.clamav-gui/clamd.conf -l " + QDir::homePath() + "/.clamav-gui/clamd.log" + clamonaccOptions;
        startclamdFile.close();
        startclamdFile.setPermissions(QFileDevice::ReadOwner|QFileDevice::WriteOwner|QFileDevice::ExeOwner|QFileDevice::ReadGroup|QFileDevice::WriteGroup|QFileDevice::ExeGroup);
    }
    parameters << QDir::homePath() + "/.clamav-gui/startclamd.sh";
    if (sudoGUI == "") sudoGUI = setupFile->getSectionValue("Settings","SudoGUI");
    startClamdProcess->start(sudoGUI,parameters);
    setupFile->setSectionValue("Clamd","Status","starting up ...");
    setupFile->setSectionValue("Clamd","ClamdPid","n/a");
    setupFile->setSectionValue("Clamd","ClamonaccPid","n/a");
    emit systemStatusChanged();
}

void clamdManager::slot_clamdSettingsChanged()
{
    setupFile->setSectionValue("OnAccess","ExtendedDetectionInfo",ui->extendedDetectionInfoComboBox->currentText());
    setupFile->setSectionValue("OnAccess","LogFileMaxSize",QString::number(ui->accLogFileMaxSizeSpinBox->value()));
    setupFile->setSectionValue("OnAccess","LogTimes",ui->accLogTimesComboBox->currentText());
    setupFile->setSectionValue("OnAccess","LogRotate",ui->accLogRotateComboBox->currentText());
    setupFile->setSectionValue("OnAccess","OnAccessMaxFileSize",QString::number(ui->onAccessMaxFileSizeSpinBox->value()));
    setupFile->setSectionValue("OnAccess","OnAccessMaxThreats",ui->onAccessMaxThreadsSpinBox->value());
    setupFile->setSectionValue("OnAccess","OnAccessPrevention",ui->onAccessPreventionComboBox->currentText());
    setupFile->setSectionValue("OnAccess","OnAccessDenyOnError",ui->onAccessDenyOnErrorComboBox->currentText());
    setupFile->setSectionValue("OnAccess","OnAccessExtraScanning",ui->accExtraScanningComboBox->currentText());
    setupFile->setSectionValue("OnAccess","OnAccessRetryAttempts",ui->accRetryAttemptsSpinBox->value());
    setupFile->setSectionValue("OnAccess","InfectedFiles",ui->infectedFilesComboBox->currentIndex());

    clamdConf->addSingleLineValue("LogFileMaxSize",setupFile->getSectionValue("OnAccess", "LogFileMaxSize")+"M");
    clamdConf->addSingleLineValue("LogTime",setupFile->getSectionValue("OnAccess", "LogTimes"));
    clamdConf->addSingleLineValue("LogRotate",setupFile->getSectionValue("OnAccess", "LogRotate"));
    clamdConf->addSingleLineValue("OnAccessMaxFileSize",setupFile->getSectionValue("OnAccess", "OnAccessMaxFileSize")+"M");
    clamdConf->addSingleLineValue("OnAccessMaxThreads",setupFile->getSectionValue("OnAccess", "OnAccessMaxThreads"));
    clamdConf->addSingleLineValue("OnAccessPrevention",setupFile->getSectionValue("OnAccess", "OnAccessPrevention"));
    clamdConf->addSingleLineValue("OnAccessDenyOnError",setupFile->getSectionValue("OnAccess", "OnAccessDenyOnError"));
    clamdConf->addSingleLineValue("OnAccessExtraScanning",setupFile->getSectionValue("OnAccess", "OnAccessExtraScanning"));
    clamdConf->addSingleLineValue("OnAccessRetryAttempts",setupFile->getSectionValue("OnAccess", "OnAccessRetryAttempts"));

    if (checkClamdRunning() == true) {
        ui->restartClamdPushButton->setVisible(true);
        ui->restartClamdPushButton->setStyleSheet("background-color:green");
    }
}

void clamdManager::restartClamonacc()
{
    QStringList parameters;

    QFile restartclamdFile(QDir::homePath() + "/.clamav-gui/restartclamd.sh");
    restartclamdFile.remove();
    if (restartclamdFile.open(QIODevice::Text|QIODevice::ReadWrite)){
        QTextStream stream(&restartclamdFile);
        stream << "#!/bin/bash\n/bin/kill -9 " + clamonaccPid + " && " + clamonaccLocation + " -c " + QDir::homePath() + "/.clamav-gui/clamd.conf -l " + QDir::homePath() + "/.clamav-gui/clamd.log";
        restartclamdFile.close();
        restartclamdFile.setPermissions(QFileDevice::ReadOwner|QFileDevice::WriteOwner|QFileDevice::ExeOwner|QFileDevice::ReadGroup|QFileDevice::WriteGroup|QFileDevice::ExeGroup);
    }
    parameters << QDir::homePath() + "/.clamav-gui/restartclamd.sh";
    if (sudoGUI == "") sudoGUI = setupFile->getSectionValue("Settings","SudoGUI");
    startClamdProcess->start(sudoGUI,parameters);
    setupFile->setSectionValue("Clamd","Status","shutting down ...");
    emit systemStatusChanged();
}

bool clamdManager::checkClamdRunning()
{
    bool rc = false;
    QProcess checkPIDProcess;
    QStringList parameters;

    parameters << "clamd";
    int pid = checkPIDProcess.execute("pidof",parameters);
    if (pid == 0) {
        rc = true;
        QFile pidFile("/tmp/clamd.pid");
        pidFile.open(QIODevice::ReadOnly);
        QTextStream stream(&pidFile);
        QString pidString = stream.readLine();
        pidString = pidString.replace("\n","");
        setupFile->setSectionValue("Clamd","ClamdPid",pidString);
        setupFile->setSectionValue("Clamd","Status","is running");
        pidFile.close();
        emit systemStatusChanged();
    } else {
        setupFile->setSectionValue("Clamd","ClamdPid","n/a");
        setupFile->setSectionValue("Clamd","Status","not running");
        emit systemStatusChanged();
    }

    return rc;
}
