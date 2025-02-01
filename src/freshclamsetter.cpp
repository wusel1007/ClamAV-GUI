#include "freshclamsetter.h"
#include "ui_freshclamsetter.h"

freshclamsetter::freshclamsetter(QWidget *parent) : QWidget(parent), ui(new Ui::freshclamsetter)
{
QDir tempDir;

    ui->setupUi(this);
    startup = true;
    lockFreshclamConf = true;
    updateLogHighLighter = new highlighter(ui->logPlainText->document());
    freshclamLogHighLighter = new highlighter(ui->deamonLogText->document());
    setupFile = new setupFileHandler(QDir::homePath() + "/.clamav-gui/settings.ini");
    sudoGUI = setupFile->getSectionValue("Settings","SudoGUI");
    QFile freshclamConfFile(QDir::homePath() + "/.clamav-gui/freshclam.conf");
    freshclamConfFile.setPermissions(QFileDevice::ReadOwner|QFileDevice::WriteOwner);
    freshclamConf = new setupFileHandler(QDir::homePath() + "/.clamav-gui/freshclam.conf");
    updater = new QProcess(this);
    connect(updater,SIGNAL(readyReadStandardError()),this,SLOT(slot_updaterHasOutput()));
    connect(updater,SIGNAL(readyReadStandardOutput()),this,SLOT(slot_updaterHasOutput()));
    connect(updater,SIGNAL(finished(int)),this,SLOT(slot_updaterFinished(int)));

    startDeamonProcess = new QProcess(this);
    connect(startDeamonProcess,SIGNAL(finished(int,QProcess::ExitStatus)),this, SLOT(slot_startDeamonProcessFinished(int,QProcess::ExitStatus)));

    pidFileWatcher = new QFileSystemWatcher(this);
    connect(pidFileWatcher,SIGNAL(fileChanged(QString)),this,SLOT(slot_fileSystemWatcherTriggered()));

    logFileWatcher = new QFileSystemWatcher(this);
    connect(logFileWatcher,SIGNAL(fileChanged(QString)),this,SLOT(slot_logFileWatcherTriggered()));

    updateLogFileWatcher = new QFileSystemWatcher(this);
    connect(updateLogFileWatcher,SIGNAL(fileChanged(QString)),this,SLOT(slot_updateFileWatcherTriggered()));

    ps_process = new QProcess;
    connect(ps_process,SIGNAL(finished(int)),this,SLOT(slot_ps_processFinished(int)));

    freshclamStartupCounter = 5;
    startDelayTimer = new QTimer(this);
    startDelayTimer->setSingleShot(true);
    connect(startDelayTimer,SIGNAL(timeout()),this,SLOT(slot_startDelayTimerExpired()));

    QFile file(QDir::homePath() + "/.clamav-gui/update.log");
    if (tempDir.exists(QDir::homePath() + "/.clamav-gui/update.log") == true) {
        slot_updateFileWatcherTriggered();
        file.setPermissions(QFileDevice::ReadOwner|QFileDevice::WriteOwner|QFileDevice::ReadUser|QFileDevice::WriteUser|QFileDevice::ReadGroup|QFileDevice::WriteGroup|QFileDevice::ReadOther|QFileDevice::WriteOther);
        updateLogFileWatcher->addPath(QDir::homePath() + "/.clamav-gui/update.log");
    } else {
        if (file.open(QIODevice::ReadWrite|QIODevice::Append|QIODevice::Text)){
            QTextStream stream(&file);
            stream << "";
            file.close();
            slot_updateFileWatcherTriggered();
            updateLogFileWatcher->addPath(QDir::homePath() + "/.clamav-gui/update.log");
            file.setPermissions(QFileDevice::ReadOwner|QFileDevice::WriteOwner|QFileDevice::ReadUser|QFileDevice::WriteUser|QFileDevice::ReadGroup|QFileDevice::WriteGroup|QFileDevice::ReadOther|QFileDevice::WriteOther);
        }
    }

    QFile fileFreshclamLog(QDir::homePath() + "/.clamav-gui/freshclam.log");
    if (tempDir.exists(QDir::homePath() + "/.clamav-gui/freshclam.log") == true) {
        fileFreshclamLog.setPermissions(QFileDevice::ReadOwner|QFileDevice::WriteOwner|QFileDevice::ReadUser|QFileDevice::WriteUser|QFileDevice::ReadGroup|QFileDevice::WriteGroup|QFileDevice::ReadOther|QFileDevice::WriteOther);
        logFileWatcher->addPath(QDir::homePath() + "/.clamav-gui/freshclam.log");
    } else {
        if (fileFreshclamLog.open(QIODevice::ReadWrite|QIODevice::Append|QIODevice::Text)){
            QTextStream stream(&file);
            stream << "";
            fileFreshclamLog.close();
            logFileWatcher->addPath(QDir::homePath() + "/.clamav-gui/freshclam.log");
            fileFreshclamLog.setPermissions(QFileDevice::ReadOwner|QFileDevice::WriteOwner|QFileDevice::ReadUser|QFileDevice::WriteUser|QFileDevice::ReadGroup|QFileDevice::WriteGroup|QFileDevice::ReadOther|QFileDevice::WriteOther);
        }
    }

    processWatcher = new QTimer(this);
    connect(processWatcher,SIGNAL(timeout()),this,SLOT(slot_processWatcherExpired()));
    processWatcher->start(5000);

    checkDaemonRunning();
    initFreshclamSettings();
    setUpdaterInfo();
}

freshclamsetter::~freshclamsetter()
{
    delete ui;
}

void freshclamsetter::slot_updateNowButtonClicked(){
QStringList parameters;

    busyLabel = new progressDialog(this);
    busyLabel->setStyleSheet("background-color: rgba(192,192,192, 200);");
    busyLabel->setGeometry((this->width() - 300) / 2,(this->height() - 160) / 2,300,160);
    busyLabel->show();
    busyLabel->setText(tr("Update process startet ....."));
    setForm(false);
    if (setupFile->getSectionBoolValue("FreshClam","runasroot") == true){
        QStringList databaseToUpdate;
        databaseToUpdate << "all" << "main" << "daily" << "bytecode";
        QString whatDB = "";
        QString para;
        if (setupFile->getSectionIntValue("FreshClam","DataBaseToUpdate") > 0) whatDB = " --update-db=" + databaseToUpdate[setupFile->getSectionIntValue("FreshClam","DataBaseToUpdate")];
        if ((setupFile->getSectionValue("Directories","LoadSupportedDBFiles") != "") && (setupFile->getSectionValue("Directories","LoadSupportedDBFiles").indexOf("not checked") == -1)){
            para = setupFile->getSectionValue("FreshclamSettings","FreshclamLocation") + " --show-progress --datadir=" + setupFile->getSectionValue("Directories","LoadSupportedDBFiles").mid(setupFile->getSectionValue("Directories","LoadSupportedDBFiles").indexOf("|")+1) + " 2>&1 > " + QDir::homePath() + "/.clamav-gui/update.log" + " --config-file=" + QDir::homePath() + "/.clamav-gui/freshclam.conf" + whatDB;
        } else {
            para = setupFile->getSectionValue("FreshclamSettings","FreshclamLocation") + " 2>&1 > " + QDir::homePath() + "/.clamav-gui/update.log" + " --show-progress --config-file=" + QDir::homePath() + "/.clamav-gui/freshclam.conf" + whatDB;
        }
        QFile startfreshclamFile(QDir::homePath() + "/.clamav-gui/startfreshclam.sh");
        startfreshclamFile.remove();
        if (startfreshclamFile.open(QIODevice::Text|QIODevice::ReadWrite)){
            QTextStream stream(&startfreshclamFile);
            stream << "#!/bin/bash\n" << para;
            startfreshclamFile.close();
            startfreshclamFile.setPermissions(QFileDevice::ReadOwner|QFileDevice::WriteOwner|QFileDevice::ExeOwner|QFileDevice::ReadGroup|QFileDevice::WriteGroup|QFileDevice::ExeGroup);
        }
        parameters << QDir::homePath() + "/.clamav-gui/startfreshclam.sh";
        if (sudoGUI == "") sudoGUI = setupFile->getSectionValue("Settings","SudoGUI");
        updater->start(sudoGUI,parameters);
    } else {
        QStringList databaseToUpdate;
        databaseToUpdate << "all" << "main" << "daily" << "bytecode";
        QString databaseDir = setupFile->getSectionValue("Directories","LoadSupportedDBFiles");
        QString checked = databaseDir.mid(0,databaseDir.indexOf("|"));
        databaseDir = databaseDir.mid(databaseDir.indexOf("|") + 1);
        if ((databaseDir != "") && (checked == "checked")) parameters << "--datadir=" + databaseDir;
        if (setupFile->getSectionIntValue("FreshClam","DataBaseToUpdate") > 0) parameters << "--update-db=" + databaseToUpdate[setupFile->getSectionIntValue("FreshClam","DataBaseToUpdate")];
        parameters << "-l" << QDir::homePath() + "/.clamav-gui/update.log";
        parameters << "--show-progress";
        parameters << "--config-file" << QDir::homePath() + "/.clamav-gui/freshclam.conf";
        updater->start(setupFile->getSectionValue("FreshclamSettings","FreshclamLocation") ,parameters);
    }
    updateLogFileWatcher->removePath(QDir::homePath() + "/.clamav-gui/update.log");
    updateLogFileWatcher->addPath(QDir::homePath() + "/.clamav-gui/update.log");
}


void freshclamsetter::slot_startStopDeamonButtonClicked(){
QStringList parameters;
    if (pidFile == ""){
        if (logFile != "") logFileWatcher->removePath(logFile);
        pidFile = freshclamConf->getSingleLineValue("PidFile");
        logFile = QDir::homePath() + "/.clamav-gui/freshclam.log";
        logFileWatcher->addPath(logFile);
        if (setupFile->getSectionBoolValue("FreshClam","runasroot") == true){
            if (startup == false) {
                QString para = setupFile->getSectionValue("FreshclamSettings","FreshclamLocation") + " -d -l " + logFile + " --config-file=" + QDir::homePath() + "/.clamav-gui/freshclam.conf";
                QFile startfreshclamFile(QDir::homePath() + "/.clamav-gui/startfreshclam.sh");
                startfreshclamFile.remove();
                if (startfreshclamFile.open(QIODevice::Text|QIODevice::ReadWrite)){
                    QTextStream stream(&startfreshclamFile);
                    stream << "#!/bin/bash\n" << para;
                    startfreshclamFile.close();
                    startfreshclamFile.setPermissions(QFileDevice::ReadOwner|QFileDevice::WriteOwner|QFileDevice::ExeOwner|QFileDevice::ReadGroup|QFileDevice::WriteGroup|QFileDevice::ExeGroup);
                }
                parameters << QDir::homePath() + "/.clamav-gui/startfreshclam.sh";
                if (sudoGUI == "") sudoGUI = setupFile->getSectionValue("Settings","SudoGUI");
                startDeamonProcess->start(sudoGUI,parameters);
            } else {
                startDelayTimer->start(2500);
            }
        } else {
            parameters << "-d";
            parameters << "-l" << logFile;
            parameters << "--config-file" << QDir::homePath() + "/.clamav-gui/freshclam.conf";
            startDeamonProcess->start(setupFile->getSectionValue("FreshclamSettings","FreshclamLocation"),parameters);
        }
    } else {
        QFile tempFile(pidFile);
        QString pidString;
        if (tempFile.exists() == true) {
            tempFile.open(QIODevice::ReadOnly);
            QTextStream stream(&tempFile);
            pidString = stream.readLine();
        }
        if (setupFile->getSectionBoolValue("FreshClam","runasroot") == true) {
          QString para = "/bin/kill -sigterm " + pidString + " && rm " + pidFile;
          QFile stopfreshclamFile(QDir::homePath() + "/.clamav-gui/stopfreshclam.sh");
          stopfreshclamFile.remove();
          if (stopfreshclamFile.open(QIODevice::Text|QIODevice::ReadWrite)){
              QTextStream stream(&stopfreshclamFile);
              stream << "#!/bin/bash\n" << para;
              stopfreshclamFile.close();
              stopfreshclamFile.setPermissions(QFileDevice::ReadOwner|QFileDevice::WriteOwner|QFileDevice::ExeOwner|QFileDevice::ReadGroup|QFileDevice::WriteGroup|QFileDevice::ExeGroup);
          }
          parameters << QDir::homePath() + "/.clamav-gui/stopfreshclam.sh";
          if (sudoGUI == "") sudoGUI = setupFile->getSectionValue("Settings","SudoGUI");
          QProcess::execute(sudoGUI,parameters);
          checkDaemonRunning();
        } else {
          parameters << "-sigterm" << pidString;
          QProcess::execute("kill",parameters);
          checkDaemonRunning();
        }
    }
}

void freshclamsetter::checkDaemonRunning(){
    QStringList ps_parameters;
    ps_parameters << "-s" << "freshclam";
    ps_process->start("pidof",ps_parameters);
}

void freshclamsetter::slot_ps_processFinished(int rc)
{
    QFile tempFile;

    if (rc == 0) {
        pidFile = freshclamConf->getSingleLineValue("PidFile");
        logFile = QDir::homePath() + "/.clamav-gui/freshclam.log";
    } else {
        pidFile = "";
        logFile = "";
    }

    if ((pidFile != "") && (tempFile.exists(pidFile) == true)){
        freshclamStartupCounter = 0;
        emit freshclamStarted();
        ui->startStopDeamonButton->setText(tr("Deamon running - stop deamon"));
        ui->startStopDeamonButton->setStyleSheet("background:green");
        ui->startStopDeamonButton->setIcon(QIcon(":/icons/icons/Clam.png"));
        setupFile->setSectionValue("Freshclam","Started",true);
        pidFileWatcher->addPath(pidFile);
        slot_setFreshclamsettingsFrameState(false);
        QFile tempFile(pidFile);
        QString pidString;
        if (tempFile.exists() == true) {
            tempFile.open(QIODevice::ReadOnly);
            QTextStream stream(&tempFile);
            pidString = stream.readLine();
        }
        setupFile->setSectionValue("Freshclam","Pid",pidString);
        emit systemStatusChanged();
    } else {
        pidFile = "";
        ui->startStopDeamonButton->setText(tr("Deamon not running - start deamon"));
        ui->startStopDeamonButton->setStyleSheet("background:red");
        ui->startStopDeamonButton->setIcon(QIcon(":/icons/icons/Clam.png"));
        setupFile->setSectionValue("Freshclam","Started",false);
        setupFile->setSectionValue("Freshclam","Pid","n/a");
        slot_setFreshclamsettingsFrameState(true);
        if ((startup == true) && (setupFile->getSectionBoolValue("Freshclam","StartDaemon") == true)) {
            freshclamStartupCounter--;
            if (freshclamStartupCounter > 0) startDelayTimer->start(2500);
        } else {
            if (startup == true) emit freshclamStarted();
        }
    }

    if ((logFile != "") && (tempFile.exists(logFile) == true)){
        logFileWatcher->addPath(logFile);
        slot_logFileWatcherTriggered();
    } else {
        logFile = QDir::homePath() + "/.clamav-gui/freshclam.log";
        if (tempFile.exists(logFile) == true){
          logFileWatcher->addPath(logFile);
          slot_logFileWatcherTriggered();
        } else {
          QFile file(QDir::homePath() + "/.clamav-gui/freshclam.log");
          if (file.open(QIODevice::ReadWrite|QIODevice::Append|QIODevice::Text)){
              QTextStream stream(&file);
              stream << "";
              file.close();
              logFileWatcher->addPath(logFile);
              slot_logFileWatcherTriggered();
          }
        }
    }
    startup = false;
}

void freshclamsetter::slot_disableUpdateButtons()
{
    setForm(false);
}

void freshclamsetter::slot_startDelayTimerExpired()
{
QStringList parameters;
    if (setupFile->getSectionBoolValue("FreshClam","runasroot") == true) {
        QString para = setupFile->getSectionValue("FreshclamSettings","FreshclamLocation") + " -d -l " + logFile + " --config-file=" + QDir::homePath() + "/.clamav-gui/freshclam.conf";
        QFile startfreshclamFile(QDir::homePath() + "/.clamav-gui/startfreshclam.sh");
        startfreshclamFile.remove();
        if (startfreshclamFile.open(QIODevice::Text|QIODevice::ReadWrite)){
            QTextStream stream(&startfreshclamFile);
            stream << "#!/bin/bash\n" << para;
            startfreshclamFile.close();
            startfreshclamFile.setPermissions(QFileDevice::ReadOwner|QFileDevice::WriteOwner|QFileDevice::ExeOwner|QFileDevice::ReadGroup|QFileDevice::WriteGroup|QFileDevice::ExeGroup);
        }
        parameters << QDir::homePath() + "/.clamav-gui/startfreshclam.sh";
        if (sudoGUI == "") sudoGUI = setupFile->getSectionValue("Settings","SudoGUI");
        startDeamonProcess->start(sudoGUI,parameters);
    } else {
        parameters << "-d";
        parameters << "-l" << logFile;
        parameters << "--config-file" << QDir::homePath() + "/.clamav-gui/freshclam.conf";
        startDeamonProcess->start(setupFile->getSectionValue("FreshclamSettings","FreshclamLocation"),parameters);
        startup = false;
        emit freshclamStarted();
    }
}

void freshclamsetter::slot_updaterFinished(int rc){
    delete busyLabel;
    QString rcstring = updater->readAll();

    if (rc == 0) {
        emit setBallonMessage(0,tr("INFO"),tr("Update-Process finished"));
        setUpdaterInfo();
    } else {
        emit setBallonMessage(1,tr("WARNING"),tr("Update-Process failed!\nRead log-messages for possible reason."));
    }
    setForm(true);
}

void freshclamsetter::slot_fileSystemWatcherTriggered(){
QDir tempDir;

    if ((pidFile != "") && (tempDir.exists(pidFile) == true)){
        ui->startStopDeamonButton->setText(tr("Deamon running - stop deamon"));
        ui->startStopDeamonButton->setStyleSheet("background:green");
        ui->startStopDeamonButton->setIcon(QIcon(":/icons/icons/Clam.png"));
        pidFileWatcher->addPath(pidFile);
    } else {
        pidFile = "";
        ui->startStopDeamonButton->setText(tr("Deamon not running - start deamon"));
        ui->startStopDeamonButton->setStyleSheet("background:red");
        ui->startStopDeamonButton->setIcon(QIcon(":/icons/icons/freshclam.png"));
        //if (setupFile->getSectionBoolValue("Freshclam","Started") == true) slot_startStopDeamonButtonClicked();
        setupFile->setSectionValue("Freshclam","Pid","n/a");
        emit systemStatusChanged();
    }
}

void freshclamsetter::slot_clearLogButtonClicked(){
QFile file(QDir::homePath() + "/.clamav-gui/update.log");

    file.remove();
    file.open(QIODevice::ReadWrite);
    QTextStream stream(&file);
    stream << "";
    file.close();
    ui->logPlainText->setPlainText("");
}

void freshclamsetter::slot_logFileWatcherTriggered(){
QFile file(logFile);
QString content;
QString value;
int pos;

    if (file.exists() == true) {
        file.open(QIODevice::ReadOnly);
        QTextStream stream(&file);
        content = stream.readAll();
        file.close();
    }
    pos = content.lastIndexOf("ClamAV update process started at");
    if (pos != -1){
        value = content.mid(pos + 33,content.indexOf("\n",pos + 33) - (pos + 33));
        setupFile->setSectionValue("Updater","LastUpdate",value);
    }
    pos = content.lastIndexOf("main.cvd updated (");
    if (pos != -1){
        value = content.mid(pos + 17,content.indexOf("\n",pos + 17) - (pos + 17));
        value.replace("(","");
        value.replace(")","");
        setupFile->setSectionValue("Updater","MainVersion",value);
    }
    pos = content.lastIndexOf("main.cvd database is up-to-date (");
    if (pos != -1){
        value = content.mid(pos + 32,content.indexOf("\n",pos + 32) - (pos + 32));
        value.replace("(","");
        value.replace(")","");
        setupFile->setSectionValue("Updater","MainVersion",value);
    }
    pos = content.lastIndexOf("daily.cvd updated (");
    if (pos != -1){
        value = content.mid(pos + 18,content.indexOf("\n",pos + 18) - (pos + 18));
        value.replace("(","");
        value.replace(")","");
        setupFile->setSectionValue("Updater","DailyVersion",value);
    }
    pos = content.lastIndexOf("daily.cld updated (");
    if (pos != -1){
        value = content.mid(pos + 18,content.indexOf("\n",pos + 18) - (pos + 18));
        value.replace("(","");
        value.replace(")","");
        setupFile->setSectionValue("Updater","DailyVersion",value);
    }
    pos = content.lastIndexOf("daily.cld database is up-to-date (");
    if (pos != -1){
        value = content.mid(pos + 33,content.indexOf("\n",pos + 33) - (pos + 33));
        value.replace("(","");
        value.replace(")","");
        setupFile->setSectionValue("Updater","DailyVersion",value);
    }
    pos = content.lastIndexOf("bytecode.cvd database is up-to-date (");
    if (pos != -1){
        value = content.mid(pos + 36,content.indexOf("\n",pos + 36) - (pos + 36));
        value.replace("(","");
        value.replace(")","");
        setupFile->setSectionValue("Updater","BytecodeVersion",value);
    }
    pos = content.lastIndexOf("bytecode.cvd updated (");
    if (pos != -1){
        value = content.mid(pos + 21,content.indexOf("\n",pos + 21) - (pos + 21));
        value.replace("(","");
        value.replace(")","");
        setupFile->setSectionValue("Updater","BytecodeVersion",value);
    }
    pos = content.lastIndexOf("Database updated");
    if (pos != -1){
        value = content.mid(pos,content.indexOf("\n",pos) - (pos));
        setupFile->setSectionValue("Updater","DatabaseFrom",value);
    }

    setUpdaterInfo();
    emit systemStatusChanged();

    ui->deamonLogText->clear();
    QStringList lines = content.split("\n");
    foreach(QString line,lines){
        ui->deamonLogText->insertPlainText(line + "\n");
        ui->deamonLogText->ensureCursorVisible();
    }
}

void freshclamsetter::slot_updateFileWatcherTriggered(){
QFile file(QDir::homePath() + "/.clamav-gui/update.log");
QString content;
QString value;
int pos;

    if (file.exists() == true) {
      file.open(QIODevice::ReadOnly);
      QTextStream stream(&file);
      content = stream.readAll();
      file.close();
    }

    pos = content.lastIndexOf("ClamAV update process started at");
    if (pos != -1){
        value = content.mid(pos + 33,content.indexOf("\n",pos + 33) - (pos + 33));
        setupFile->setSectionValue("Updater","LastUpdate",value);
    }
    pos = content.lastIndexOf("main.cvd updated");
    if (pos != -1){
        value = content.mid(pos + 17,content.indexOf("\n",pos + 17) - (pos + 17));
        value.replace("(","");
        value.replace(")","");
        setupFile->setSectionValue("Updater","MainVersion",value);
    }
    pos = content.lastIndexOf("daily.cvd updated");
    if (pos != -1){
        value = content.mid(pos + 18,content.indexOf("\n",pos + 18) - (pos + 18));
        value.replace("(","");
        value.replace(")","");
        setupFile->setSectionValue("Updater","DailyVersion",value);
    }
    pos = content.lastIndexOf("bytecode.cvd updated");
    if (pos != -1){
        value = content.mid(pos + 21,content.indexOf("\n",pos + 21) - (pos + 21));
        value.replace("(","");
        value.replace(")","");
        setupFile->setSectionValue("Updater","BytecodeVersion",value);
    }
    pos = content.lastIndexOf("Database updated");
    if (pos != -1){
        value = content.mid(pos,content.indexOf("\n",pos) - (pos));
        setupFile->setSectionValue("Updater","DatabaseFrom",value);
    }

    emit systemStatusChanged();

    ui->logPlainText->setPlainText("");
    QStringList lines = content.split("\n");
    foreach(QString line,lines){
        ui->logPlainText->insertPlainText(line + "\n");
        ui->logPlainText->ensureCursorVisible();
    }
}

void freshclamsetter::setForm(bool mode){
    ui->clearLogButton->setEnabled(mode);
    ui->deamonClearLogButton->setEnabled(mode);
    ui->deamonClearLogButton->setEnabled(mode);
    ui->startStopDeamonButton->setEnabled(mode);
    ui->updateNowButton->setEnabled(mode);
    ui->updateSettingsGroupBox->setEnabled(mode);
}

void freshclamsetter::slot_clearDeamonLogButtonClicked(){
QFile file(QDir::homePath() + "/.clamav-gui/freshclam.log");

    ui->deamonLogText->setPlainText("");
    updateLogFileWatcher->removePath(QDir::homePath() + "/.clamav-gui/freshclam.log");
    file.remove();
    file.open(QIODevice::ReadWrite);
    QTextStream stream(&file);
    stream << "";
    file.close();
    file.setPermissions(QFileDevice::ReadOwner|QFileDevice::WriteOwner|QFileDevice::ReadUser|QFileDevice::WriteUser|QFileDevice::ReadGroup|QFileDevice::WriteGroup|QFileDevice::ReadOther|QFileDevice::WriteOther);
    updateLogFileWatcher->addPath(QDir::homePath() + "/.clamav-gui/freshclam.log");

}

void freshclamsetter::setUpdaterInfo(){
    QString htmlCode = "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">\n<html><head><meta name=\"qrichtext\" content=\"1\" /><style type=\"text/css\">\np, li { white-space: pre-wrap; }\n</style></head><body style=\" font-family:'Sans Serif'; font-size:9pt; font-weight:400; font-style:normal;\">\n<p style=\"-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><br /></p>";
    htmlCode = htmlCode + "<table><tr><td width='200'>";
    if (setupFile->getSectionValue("Updater","DatabaseFrom") != "") {
        htmlCode = htmlCode + tr("Database origin : </td><td>") + setupFile->getSectionValue("Updater","DatabaseFrom") + "</td></tr><tr><td>";
    }
    if (setupFile->getSectionValue("Updater","LastUpdate") != "") {
        htmlCode = htmlCode + tr("Last Update : </td><td>") + setupFile->getSectionValue("Updater","LastUpdate") + "</td></tr><tr><td>";
    }
    if (setupFile->getSectionValue("Updater","MainVersion") != "") {
        htmlCode = htmlCode + tr("Main File : </td><td>") + setupFile->getSectionValue("Updater","MainVersion") + "</td></tr><tr><td>";
    }
    if (setupFile->getSectionValue("Updater","DailyVersion") != "") {
        htmlCode = htmlCode + tr("Daily File : </td><td>") + setupFile->getSectionValue("Updater","DailyVersion") + "</td></tr><tr><td>";
    }
    if (setupFile->getSectionValue("Updater","BytecodeVersion") != "") {
        htmlCode = htmlCode + tr("ByteCode File : </td><td>") + setupFile->getSectionValue("Updater","BytecodeVersion") + "</td></tr></table>";
    }
    htmlCode = htmlCode + "</body></html>";
    ui->updateInfoText->setHtml(htmlCode);
}

QString freshclamsetter::extractPureNumber(QString value) {
    QString rc = "";
    QString validator = "0.123456789";
    bool isValid = true;
    int index = 0;
    value = value.trimmed();
    QString character;
    while ((index < value.length()) && (isValid == true)) {
        character = value.mid(index,1);
        if (validator.indexOf(character) != -1) rc += character; else isValid = false;
        index++;
    }

    return rc;
}

void freshclamsetter::slot_updaterHasOutput(){
    static QString oldLine;
    QString output = updater->readAll();
    int start = output.lastIndexOf("]") + 1;
    int end = output.lastIndexOf("[");
    QString line = output.mid(start,end-start-1);
    if (line != "") {
        QStringList values = line.split("/");
        if (values.size() == 2) {
            QString maxValueString = values.at(1);
            QString valueString = values.at(0);
            maxValueString = extractPureNumber(maxValueString);
            valueString = extractPureNumber(valueString);
            double valueMax = maxValueString.toDouble();
            double value = valueString.toDouble();
            busyLabel->setProgressBarMaxValue(valueMax);
            busyLabel->setProgressBarValue(value);
        }
    }
    line="";
    if ((output.indexOf("Testing database:") > -1) && (oldLine != "Testing Database")) line = "Testing Database";
    if ((output.indexOf("bytecode database available for download") != -1) && (oldLine != "Downloading bytecode.cvd")) line = "Downloading bytecode.cvd";
    if ((output.indexOf("main database available for download") != -1) && (oldLine != "Downloading main.cvd")) line = "Downloading main.cvd";
    if ((output.indexOf("daily database available for download") != -1) && (oldLine != "Downloading daily.cvd")) line = "Downloading daily.cvd";
    if (line != "") {
        busyLabel->setText(line);
        oldLine = line;
    }
}

void freshclamsetter::slot_startDeamonProcessFinished(int exitCode,QProcess::ExitStatus exitStatus)
{
    if ((exitCode != 0) || (exitStatus == QProcess::CrashExit))freshclamStartupCounter = 0;
    if (exitCode == 0){
        ui->startStopDeamonButton->setText(tr("Deamon running - stop deamon"));
        ui->startStopDeamonButton->setStyleSheet("background:green");
        ui->startStopDeamonButton->setIcon(QIcon(":/icons/icons/Clam.png"));
        pidFileWatcher->addPath(pidFile);
        logFileWatcher->addPath(logFile);
        slot_logFileWatcherTriggered();
        pidFileWatcher->addPath(pidFile);
        checkDaemonRunning();
    } else {
        pidFile = "";
        ui->startStopDeamonButton->setText(tr("Deamon not running - start deamon"));
        ui->startStopDeamonButton->setStyleSheet("background:red");
        ui->startStopDeamonButton->setIcon(QIcon(":/icons/icons/Clam.png"));
        setupFile->setSectionValue("Freshclam","Pid","n/a");
        emit systemStatusChanged();
    }
}

void freshclamsetter::initFreshclamSettings() {
    QStringList parameters;
    QFile tempFile;

    lockFreshclamConf = true;
    setupFile = new setupFileHandler(QDir::homePath() + "/.clamav-gui/settings.ini");
    if (setupFile->keywordExists("FreshClam","runasroot") == true) ui->runasrootCheckBox->setChecked(setupFile->getSectionBoolValue("FreshClam","runasroot")); else {
        setupFile->setSectionValue("FreshClam","runasroot","true");
        ui->runasrootCheckBox->setChecked(true);
    }
    if (setupFile->keywordExists("Freshclam","StartDaemon") == true) ui->autoStartDaemonCheckBox->setChecked(setupFile->getSectionBoolValue("Freshclam","StartDaemon")); else {
        setupFile->setSectionValue("Freshclam","StartDaemon",false);
        ui->autoStartDaemonCheckBox->setChecked(false);
    }

    getDBUserProcess = new QProcess(this);
    connect(getDBUserProcess,SIGNAL(finished(int)),this,SLOT(slot_getDBUserProcessFinished()));

    freshclamLocationProcess = new QProcess(this);
    connect(freshclamLocationProcess,SIGNAL(finished(int)),this,SLOT(slot_freshclamLocationProcessFinished()));
    connect(freshclamLocationProcess,SIGNAL(readyRead()),this,SLOT(slot_freshclamLocationProcessHasOutput()));

    clamscanLocationProcess = new QProcess(this);
    connect(clamscanLocationProcess,SIGNAL(finished(int)),this,SLOT(slot_clamscanLocationProcessFinished()));
    connect(clamscanLocationProcess,SIGNAL(readyRead()),this,SLOT(slot_clamscanLocationProcessHasOutput()));
    freshclamConf = new setupFileHandler(QDir::homePath() + "/.clamav-gui/freshclam.conf");

    if (freshclamConf->singleLineExists("DatabaseDirectory") == true) {
        ui->databaseDirectoryPathLabel->setText(freshclamConf->getSingleLineValue("DatabaseDirectory"));
    } else {
        QDir tempdir;
        if ((tempdir.exists("/var/lib/clamav") == true) && ((tempFile.exists("/var/lib/clamav/freshclam.dat") == true))){
            freshclamConf->setSingleLineValue("DatabaseDirectory","/var/lib/clamav");
            ui->runasrootCheckBox->setChecked(true);
            setupFile->setSectionValue("FreshClam","runasroot",true);
        } else {
            freshclamConf->setSingleLineValue("DatabaseDirectory",QDir::homePath() + "/.clamav-gui/signatures");
            ui->runasrootCheckBox->setChecked(false);
            setupFile->setSectionValue("FreshClam","runasroot",false);
        }
        ui->databaseDirectoryPathLabel->setText(freshclamConf->getSingleLineValue("DatabaseDirectory"));
    }

    if (freshclamConf->singleLineExists("LogSyslog") == true) ui->logSysLogComboBox->setCurrentText(freshclamConf->getSingleLineValue("LogSyslog")); else {
        freshclamConf->setSingleLineValue("LogSyslog","no");
        ui->logSysLogComboBox->setCurrentText(freshclamConf->getSingleLineValue("LogSyslog"));
    }
    if (freshclamConf->singleLineExists("LogFacility") == true) ui->logFacilityComboBox->setCurrentText(freshclamConf->getSingleLineValue("LogFacility")); else {
        freshclamConf->setSingleLineValue("LogFacility","LOG_LOCAL6");
        ui->logFacilityComboBox->setCurrentText(freshclamConf->getSingleLineValue("LogFacility"));
    }
    if (freshclamConf->singleLineExists("LogTime") == true) ui->logTimeComboBox->setCurrentText(freshclamConf->getSingleLineValue("LogTime")); else {
        freshclamConf->setSingleLineValue("LogTime","no");
        ui->logTimeComboBox->setCurrentText(freshclamConf->getSingleLineValue("LogTime"));
    }
    if (freshclamConf->singleLineExists("LogRotate") == true) ui->logRotateComboBox->setCurrentText(freshclamConf->getSingleLineValue("LogRotate")); else {
        freshclamConf->setSingleLineValue("LogRotate","no");
        ui->logRotateComboBox->setCurrentText(freshclamConf->getSingleLineValue("LogRotate"));
    }
    if (freshclamConf->singleLineExists("PidFile") == true) ui->pidFilePathLabel->setText(freshclamConf->getSingleLineValue("PidFile")); else {
        freshclamConf->setSingleLineValue("PidFile","/tmp/freshclam.pid");
        ui->pidFilePathLabel->setText(freshclamConf->getSingleLineValue("PidFile"));
    }
    if (freshclamConf->singleLineExists("DatabaseOwner") == true) ui->databaseOwnerLineEdit->setText(freshclamConf->getSingleLineValue("DatabaseOwner")); else {
        freshclamConf->setSingleLineValue("DatabaseOwner","clamav");
        ui->databaseOwnerLineEdit->setText(freshclamConf->getSingleLineValue("DatabaseOwner"));
    }
    if (freshclamConf->singleLineExists("DatabaseMirror") == true) ui->databaseMirrorLineEdit->setText(freshclamConf->getSingleLineValue("DatabaseMirror")); else {
        freshclamConf->setSingleLineValue("DatabaseMirror","database.clamav.net");
        ui->databaseMirrorLineEdit->setText(freshclamConf->getSingleLineValue("DatabaseMirror"));
    }
    if (freshclamConf->singleLineExists("LogVerbose") == true) ui->logVerboseComboBox->setCurrentText(freshclamConf->getSingleLineValue("LogVerbose")); else {
        freshclamConf->setSingleLineValue("LogVerbose","no");
        ui->logVerboseComboBox->setCurrentText(freshclamConf->getSingleLineValue("LogVerbose"));
    }
    if (freshclamConf->singleLineExists("Checks") == true) ui->checkPerDaySpinBox->setValue(freshclamConf->getSingleLineValue("Checks").toInt()); else {
        freshclamConf->setSingleLineValue("Checks","12");
        ui->checkPerDaySpinBox->setValue(freshclamConf->getSingleLineValue("Checks").toInt());
    }
    ui->freshclamLocationLineEdit->setText(setupFile->getSectionValue("FreshclamSettings","FreshclamLocation"));
    ui->databaseTypeComboBox->setCurrentIndex(setupFile->getSectionIntValue("FreshClam","DataBaseToUpdate"));

    if (freshclamConf->singleLineExists("HTTPProxyServer") == true) ui->httpProxyServerLineEdit->setText(freshclamConf->getSingleLineValue("HTTPProxyServer"));
    if (freshclamConf->singleLineExists("HTTPProxyPort") == true) ui->httpProxyPortLineEdit->setText(freshclamConf->getSingleLineValue("HTTPProxyPort"));
    if (freshclamConf->singleLineExists("HTTPProxyUsername") == true) ui->httpProxyUsernameLineEdit->setText(freshclamConf->getSingleLineValue("HTTPProxyUsername"));
    if (freshclamConf->singleLineExists("HTTPProxyPassword") == true) ui->httpProxyPasswordLineEdit->setText(freshclamConf->getSingleLineValue("HTTPProxyPassword"));
    if (freshclamConf->singleLineExists("OnUpdateExecute") == true) ui->onUpdateExecuteLineEdit->setText(freshclamConf->getSingleLineValue("OnUpdateExecute"));
    if (freshclamConf->singleLineExists("OnErrorExecute") == true) ui->onErrorExecuteLineEdit->setText(freshclamConf->getSingleLineValue("OnErrorExecute"));
    if (freshclamConf->singleLineExists("OnOutdatedExecute") == true) ui->onOutdatedExecuteLineEdit->setText(freshclamConf->getSingleLineValue("OnOutdatedExecute"));

    parameters << "-ld" << ui->databaseDirectoryPathLabel->text();
    QDir dbDir;

    if (dbDir.exists(ui->databaseDirectoryPathLabel->text()) == true) {
        getDBUserProcess->start("ls",parameters);
    }

    lockFreshclamConf = false;
    clamscanlocationProcessOutput = "";
    freshclamlocationProcessOutput = "";

    parameters.clear();
    parameters << "clamscan";
    clamscanLocationProcess->start("whereis",parameters);

    parameters.clear();
    parameters << "freshclam";
    freshclamLocationProcess->start("whereis",parameters);
}

void freshclamsetter::slot_runasrootCheckBoxChanged()
{
    setupFile->setSectionValue("FreshClam","runasroot",ui->runasrootCheckBox->isChecked());
}

void freshclamsetter::slot_writeFreshclamSettings()
{
    if (lockFreshclamConf == false) {
        freshclamConf->setSingleLineValue("DatabaseDirectory",ui->databaseDirectoryPathLabel->text());
        freshclamConf->setSingleLineValue("LogSyslog",ui->logSysLogComboBox->currentText());
        freshclamConf->setSingleLineValue("LogFacility",ui->logFacilityComboBox->currentText());
        freshclamConf->setSingleLineValue("LogRotate",ui->logRotateComboBox->currentText());
        freshclamConf->setSingleLineValue("PidFile",ui->pidFilePathLabel->text());
        freshclamConf->setSingleLineValue("DatabaseOwner",ui->databaseOwnerLineEdit->text());
        freshclamConf->setSingleLineValue("DatabaseMirror",ui->databaseMirrorLineEdit->text());
        freshclamConf->setSingleLineValue("LogTime",ui->logTimeComboBox->currentText());
        freshclamConf->setSingleLineValue("LogVerbose",ui->logVerboseComboBox->currentText());
        if (ui->onErrorExecuteLineEdit->text() != "") freshclamConf->setSingleLineValue("OnErrorExecute",ui->onErrorExecuteLineEdit->text()); else {
            freshclamConf->setSingleLineValue("OnErrorExecute","obsolete");
            freshclamConf->removeSingleLine("OnErrorExecute","obsolete");
        }
        if (ui->onUpdateExecuteLineEdit->text() != "") freshclamConf->setSingleLineValue("OnUpdateExecute",ui->onUpdateExecuteLineEdit->text()); else {
            freshclamConf->setSingleLineValue("OnUpdateExecute","obsolete");
            freshclamConf->removeSingleLine("OnUpdateExecute","obsolete");
        }
        if (ui->onOutdatedExecuteLineEdit->text() != "") freshclamConf->setSingleLineValue("OnOutdatedExecute",ui->onOutdatedExecuteLineEdit->text()); else {
            freshclamConf->setSingleLineValue("OnOutdatedExecute","obsolete");
            freshclamConf->removeSingleLine("OnOutdatedExecute","obsolete");
        }
        if (ui->httpProxyServerLineEdit->text() != "") freshclamConf->setSingleLineValue("HTTPProxyServer",ui->httpProxyServerLineEdit->text()); else {
            freshclamConf->setSingleLineValue("HTTPProxyServer","obsolete");
            freshclamConf->removeSingleLine("HTTPProxyServer","obsolete");
        }
        if (ui->httpProxyPortLineEdit->text() != "") freshclamConf->setSingleLineValue("HTTPProxyPort",ui->httpProxyPortLineEdit->text()); else {
            freshclamConf->setSingleLineValue("HTTPProxyPort","obsolete");
            freshclamConf->removeSingleLine("HTTPProxyPort","obsolete");
        }
        if (ui->httpProxyUsernameLineEdit->text() != "") freshclamConf->setSingleLineValue("HTTPProxyUsername",ui->httpProxyUsernameLineEdit->text()); else {
            freshclamConf->setSingleLineValue("HTTPProxyUsername","obsolete");
            freshclamConf->removeSingleLine("HTTPProxyUsername","obsolete");
        }
        if (ui->httpProxyPasswordLineEdit->text() != "") freshclamConf->setSingleLineValue("HTTPProxyPassword",ui->httpProxyPasswordLineEdit->text()); else {
            freshclamConf->setSingleLineValue("HTTPProxyPassword","obsolete");
            freshclamConf->removeSingleLine("HTTPProxyPassword","obsolete");
        }
        setupFile->setSectionValue("FreshClam","UpdatesPerDay",ui->checkPerDaySpinBox->value());
        setupFile->setSectionValue("FreshClam","DataBaseToUpdate",ui->databaseTypeComboBox->currentIndex());
    }
}

void freshclamsetter::slot_dbPathChanged(QString dbPath)
{
    if (dbPath != "") {
        freshclamConf->setSingleLineValue("DatabaseDirectory",dbPath);
        ui->databaseDirectoryPathLabel->setText(dbPath);

        QStringList parameters;
        parameters << "-ld" << ui->databaseDirectoryPathLabel->text();
        QDir dbDir;
        if (dbDir.exists(ui->databaseDirectoryPathLabel->text()) == true) {
            getDBUserProcess->start("ls",parameters);
        } else {
            freshclamConf->setSingleLineValue("DatabaseOwner","clamav");
        }

    }
}

void freshclamsetter::slot_getDBUserProcessFinished()
{
    QString output = getDBUserProcess->readAll();
    QStringList values = output.split(" ");
    if (values.size() > 1) {
        freshclamConf->setSingleLineValue("DatabaseOwner",values[2]);
        ui->databaseOwnerLineEdit->setText(values[2]);
    }
}

void freshclamsetter::slot_pidFileSelectButtonClicked()
{
    QString rc = QFileDialog::getExistingDirectory(this,tr("Select Folder for the PID-File"),"/tmp");
    if (rc != "") {
        freshclamConf->setSingleLineValue("PidFile",rc + "/freshclam.pid");
        setupFile->setSectionValue("Freshclam","PidFile",rc + "/freshclam.pid");
        ui->pidFilePathLabel->setText(rc + "/freshclam.pid");
    }
}

void freshclamsetter::slot_freshclamLocationProcessFinished()
{
    freshclamlocationProcessOutput = freshclamlocationProcessOutput + freshclamLocationProcess->readAll();
    if (freshclamlocationProcessOutput.length() > 13) {
        QStringList values = freshclamlocationProcessOutput.split(" ");
        if (values.size() > 1) {
            if (values.length() > 0) setupFile->setSectionValue("FreshclamSettings","FreshclamLocation",values[1].replace("\n","")); else setupFile->setSectionValue("FreshclamSettings","FreshclamLocation","not found");
            ui->freshclamLocationLineEdit->setText(values[1]);
            emit systemStatusChanged();
            QFile file(ui->databaseDirectoryPathLabel->text() + "/freshclam.dat");
            if (file.exists() == false) {
                if (QMessageBox::warning(this,tr("Virus definitions missing!"),ui->databaseDirectoryPathLabel->text() + "\n" + tr("No virus definitions found in the database folder. Should the virus definitions be downloaded?"),QMessageBox::Yes,QMessageBox::No) == QMessageBox::Yes) {
                    emit updateDatabase();
                }
            }
            //if (setupFile->getSectionBoolValue("Freshclam","Started") == true) slot_startStopDeamonButtonClicked();
        }
    } else {
        setupFile->setSectionValue("FreshclamSettings","FreshclamLocation","not found");
        ui->freshclamLocationLineEdit->setText("not found");
        QMessageBox::warning(this,"WARNING","Freshclam is missing. Please install!",QMessageBox::Ok);
        setForm(false);        //emit disableUpdateButtons();
        emit quitApplication();
    }
}

void freshclamsetter::slot_clamscanLocationProcessFinished()
{
    clamscanlocationProcessOutput = clamscanlocationProcessOutput + clamscanLocationProcess->readAll();
    if (clamscanlocationProcessOutput.length() < 13) {
        QMessageBox::warning(this,"ERROR","Clamav is missing. Please install!",QMessageBox::Ok);
        emit quitApplication();
    }
}

void freshclamsetter::slot_freshclamLocationProcessHasOutput()
{
    freshclamlocationProcessOutput = freshclamlocationProcessOutput + freshclamLocationProcess->readAll();
}

void freshclamsetter::slot_clamscanLocationProcessHasOutput()
{
    clamscanlocationProcessOutput = clamscanlocationProcessOutput + clamscanLocationProcess->readAll();
}

void freshclamsetter::slot_setFreshclamsettingsFrameState(bool state)
{
    ui->freshclamLocationLineEdit->setEnabled(state);
    ui->pidFileSelectPushButton->setEnabled(state);
    ui->pidFilePathLabel->setEnabled(state);
    ui->databaseOwnerLineEdit->setEnabled(state);
    ui->databaseMirrorLineEdit->setEnabled(state);
    ui->logSysLogComboBox->setEnabled(state);
    ui->logFacilityComboBox->setEnabled(state);
    ui->logRotateComboBox->setEnabled(state);
    ui->logTimeComboBox->setEnabled(state);
    ui->logVerboseComboBox->setEnabled(state);
    ui->databaseDirectoryPathLabel->setEnabled(state);
    ui->databaseTypeComboBox->setEnabled(state);
    ui->checkPerDaySpinBox->setEnabled(state);
    ui->runasrootCheckBox->setEnabled(state);
    ui->autoStartDaemonCheckBox->setEnabled(state);
    ui->httpProxyServerLineEdit->setEnabled(state);
    ui->httpProxyPortLineEdit->setEnabled(state);
    ui->httpProxyUsernameLineEdit->setEnabled(state);
    ui->httpProxyPasswordLineEdit->setEnabled(state);
    ui->onUpdateExecuteLineEdit->setEnabled(state);
    ui->onUpdateExecutePushButton->setEnabled(state);
    ui->onErrorExecuteLineEdit->setEnabled(state);
    ui->onErrorExecutePushButton->setEnabled(state);
    ui->onOutdatedExecuteLineEdit->setEnabled(state);
    ui->onOutdatedExecutePushButton->setEnabled(state);
}

void freshclamsetter::slot_autoStartDaemon()
{
    setupFile->setSectionValue("Freshclam","StartDaemon",ui->autoStartDaemonCheckBox->isChecked());
}

void freshclamsetter::slot_onUpdateExecuteButtonClicked()
{
    QString rc = QFileDialog::getOpenFileName(this,tr("On Update Execute"),tr("Select a programm that will be executed when the database is updated."));
    if (rc != "") ui->onUpdateExecuteLineEdit->setText(rc);
    slot_writeFreshclamSettings();
}

void freshclamsetter::slot_onErrorExecuteButtonClicked()
{
    QString rc = QFileDialog::getOpenFileName(this,tr("On Error Execute"),tr("Select a programm that will be executed when an error occured."));
    if (rc != "") ui->onErrorExecuteLineEdit->setText(rc);
    slot_writeFreshclamSettings();
}

void freshclamsetter::slot_onOutdatedExecuteButtonClicked()
{
    QString rc = QFileDialog::getOpenFileName(this,tr("On Outdated Execute"),tr("Select a programm that will be executed when the database is outdated."));
    if (rc != "") ui->onOutdatedExecuteLineEdit->setText(rc);
    slot_writeFreshclamSettings();
}

void freshclamsetter::slot_processWatcherExpired()
{
    QString freshclamPid = setupFile->getSectionValue("Freshclam","Pid");

    QDir checkDir;
    if ( freshclamPid != "n/a") {
        if (checkDir.exists("/proc/" + freshclamPid) == false) {
            setupFile->setSectionValue("Freshclam","Pid","n/a");
            emit systemStatusChanged();
            checkDaemonRunning();
        }
    }
}
