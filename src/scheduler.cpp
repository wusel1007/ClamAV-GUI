#include "scheduler.h"
#include "ui_scheduler.h"

scheduler::scheduler(QWidget *parent) : QWidget(parent), ui(new Ui::scheduler)
{
QStringList header;

    setupFile = new setupFileHandler(QDir::homePath() + "/.clamav-gui/settings.ini");
    ui->setupUi(this);
    header << tr("ID") << tr("Interval") << tr("Profile") << tr("Last Scan") << tr("Next Scan") << tr("Remove") << tr("Scan Now") << tr("Log-File");
    ui->scanJobTableWidget->setHorizontalHeaderLabels(header);
    ui->scanJobTableWidget->setColumnWidth(6,120);
    ui->scanJobTableWidget->setColumnWidth(5,120);
    removeButtonGroup = new QButtonGroup(this);
    scanNowButtonGroup = new QButtonGroup(this);
    logButtonGroup = new QButtonGroup(this);
    connect(removeButtonGroup,SIGNAL(buttonClicked(int)),this,SLOT(slot_removeButtonClicked(int)));
    connect(scanNowButtonGroup,SIGNAL(buttonClicked(int)),this,SLOT(slot_scanButtonClicked(int)));
    connect(logButtonGroup,SIGNAL(buttonClicked(int)),this,SLOT(slot_logButtonClicked(int)));
    slot_updateProfiles();
    updateScheduleList();
    checkTimer = new QTimer(this);
    checkTimer->setSingleShot(false);
    connect(checkTimer,SIGNAL(timeout()),this,SLOT(slot_checkTimerTimeout()));
    checkTimer->start(15000);
}

scheduler::~scheduler()
{
    delete ui;
}

void scheduler::slot_addDailyScanJobButtonClicked(){
QDateTime currentDateTime = QDateTime::currentDateTime();
QString scanTime = ui->dailyTimeEdit->text();
    if (scanTime.length() < 8) scanTime = scanTime + ":00";
QDateTime nextScanObject = QDateTime(QDate::currentDate(),QTime::fromString(scanTime));
QString id = QString::number(QDateTime::currentDateTimeUtc().toMSecsSinceEpoch());
QString entry;

    if (nextScanObject < currentDateTime) nextScanObject = nextScanObject.addDays(1);
    entry = "daily|" + ui->profileComboBox->currentText() + "|" + "never" + "|" + QString::number(nextScanObject.toMSecsSinceEpoch());
    setupFile->setSectionValue("ScanJobs",id,entry);
    updateScheduleList();
}

void scheduler::slot_addWeeklyScanJobButtonClicked(){
QDateTime currentDateTime = QDateTime::currentDateTime();
QDateTime nextScanObject = currentDateTime;
QString id = QString::number(QDateTime::currentDateTimeUtc().toMSecsSinceEpoch());
int currentDayofweek = currentDateTime.date().dayOfWeek();
int dayofweek = ui->weeklyDayOfWeekComboBox->currentIndex() + 1;
QString entry;
QString scanTime = ui->weeklyTimeEdit->text();

    if (scanTime.length() < 8) scanTime = scanTime + ":00";

    if (currentDayofweek < dayofweek){
        int diff = dayofweek - currentDayofweek;
        nextScanObject = nextScanObject.addDays(diff);
    } else {
        if (currentDayofweek > dayofweek){
            int diff = 7 - (currentDayofweek - dayofweek);
            nextScanObject = nextScanObject.addDays(diff);
        } else {
            if (QTime::currentTime() > QTime::fromString(scanTime)){
                nextScanObject = nextScanObject = nextScanObject.addDays(7);
            }
        }
    }
    nextScanObject = QDateTime(nextScanObject.date(),QTime::fromString(scanTime));
    entry = "weekly|" + ui->profileComboBox->currentText() + "|" + "never" + "|" + QString::number(nextScanObject.toMSecsSinceEpoch());
    setupFile->setSectionValue("ScanJobs",id,entry);
    updateScheduleList();
}

void scheduler::slot_addMonthlyScanJobButtonClicked(){
QString entry;
QString id = QString::number(QDateTime::currentDateTimeUtc().toMSecsSinceEpoch());
QDateTime currentDateTime = QDateTime::currentDateTime();
QString scanTime = ui->monthlyTimeEdit->text();
    if (scanTime.length() < 8) scanTime = scanTime + ":00";
QDateTime nextScanObject = QDateTime(QDate(QDate::currentDate().year(),QDate::currentDate().month(),ui->monthlyDaySpinBox->value()),QTime::fromString(scanTime));

    if (nextScanObject < QDateTime::currentDateTime()){
        nextScanObject = nextScanObject.addMonths(1);
    }
    entry = "monthly|" + ui->profileComboBox->currentText() + "|" + "never" + "|" + QString::number(nextScanObject.toMSecsSinceEpoch());
    setupFile->setSectionValue("ScanJobs",id,entry);
    updateScheduleList();}

void scheduler::slot_updateProfiles(){
QStringList profiles = setupFile->getKeywords("Profiles");
QStringList selectableProfiles;
QStringList directories;

    foreach(QString profile,profiles){
        setupFileHandler * sf = new setupFileHandler(QDir::homePath() + "/.clamav-gui/profiles/" + profile + ".ini");
        if (sf->getSectionValue(profile,"Directories") != "") selectableProfiles << profile;
    }

    ui->profileComboBox->clear();
    ui->profileComboBox->addItems(selectableProfiles);
    if (selectableProfiles.count() == 0){
        ui->dailyAddScanJobButton->setEnabled(false);
        ui->weeklyAddScanJobButton->setEnabled(false);
        ui->monthlyAddScanJobButton->setEnabled(false);
        ui->dailyProfileLabel->setText("--------------");
        ui->weeklyProfileLabel->setText("--------------");
        ui->monthlyProfileLable->setText("--------------");
    } else {
        ui->dailyAddScanJobButton->setEnabled(true);
        ui->weeklyAddScanJobButton->setEnabled(true);
        ui->monthlyAddScanJobButton->setEnabled(true);
        ui->dailyProfileLabel->setText(ui->profileComboBox->currentText());
        ui->weeklyProfileLabel->setText(ui->profileComboBox->currentText());
        ui->monthlyProfileLable->setText(ui->profileComboBox->currentText());
        slot_profileSelectionChanged();
    }
}


void scheduler::updateScheduleList(){
QStringList jobs = setupFile->getKeywords("ScanJobs");
QStringList jobData;
QDateTime tempDateTime;
int width[8] = {130,80,160,180,180,130,130,130};
int id = 0;
int rowCount;

    while (ui->scanJobTableWidget->rowCount() > 0){
        QPushButton * removeButton = (QPushButton*)ui->scanJobTableWidget->cellWidget(0,5);
        QPushButton * scanNowButton = (QPushButton*)ui->scanJobTableWidget->cellWidget(0,6);
        QPushButton * logButton = (QPushButton*)ui->scanJobTableWidget->cellWidget(0,7);
        removeButtonGroup->removeButton(removeButton);
        scanNowButtonGroup->removeButton(scanNowButton);
        logButtonGroup->removeButton(logButton);
        ui->scanJobTableWidget->removeRow(0);
    }

    foreach(QString job,jobs){
        jobData = setupFile->getSectionValue("ScanJobs",job).split("|");
        rowCount = ui->scanJobTableWidget->rowCount();
        ui->scanJobTableWidget->insertRow(rowCount);
        for (int i = 0; i < 8; i++){
            ui->scanJobTableWidget->setColumnWidth(i,width[i]);
        }
        ui->scanJobTableWidget->setItem(rowCount,0,new QTableWidgetItem(job));
        ui->scanJobTableWidget->setItem(rowCount,1,new QTableWidgetItem(jobData[0]));
        ui->scanJobTableWidget->setItem(rowCount,2,new QTableWidgetItem(jobData[1]));
        if (jobData[2] == "never") {
            ui->scanJobTableWidget->setItem(rowCount,3,new QTableWidgetItem("Never"));
        } else {
            tempDateTime.setMSecsSinceEpoch(jobData[2].toLongLong());
            ui->scanJobTableWidget->setItem(rowCount,3,new QTableWidgetItem(tempDateTime.toString("dd.MM.yyyy 'at' hh:mm")));
        }
        tempDateTime.setMSecsSinceEpoch(jobData[3].toLongLong());
        ui->scanJobTableWidget->setItem(rowCount,4,new QTableWidgetItem(tempDateTime.toString("dd.MM.yyyy 'at' hh:mm")));
        QPushButton * removeButton = new QPushButton(QIcon(":/icons/icons/trash-can.png"),tr("remove task"),this);
        removeButtonGroup->addButton(removeButton,id);
        QPushButton * scanNowButton = new QPushButton(QIcon(":/icons/icons/start.png"),tr("scan now"),this);
        scanNowButtonGroup->addButton(scanNowButton,id);
        QPushButton * logButton = new QPushButton(QIcon(":/icons/icons/information.png"),tr("Log-File"),this);
        logButtonGroup->addButton(logButton,id);
        ui->scanJobTableWidget->setCellWidget(rowCount,5,removeButton);
        ui->scanJobTableWidget->setCellWidget(rowCount,6,scanNowButton);
        ui->scanJobTableWidget->setCellWidget(rowCount,7,logButton);
        for (int i = 0; i < 5; i++){
            ui->scanJobTableWidget->item(rowCount,i)->setTextAlignment(Qt::AlignCenter);
        }
        id++;
    }
}

void scheduler::slot_scanButtonClicked(int id){
int rc = QMessageBox::information(this,tr("Start Scan-Job"),tr("Do you realy want to start this Scan-Job?"),QMessageBox::Yes,QMessageBox::No);
QString profileName;
qint64 today = QDateTime::currentDateTime().toMSecsSinceEpoch();
QStringList values;
QString temp;
QString scanID;

    if (rc == QMessageBox::Yes){
        QTableWidgetItem *item = (QTableWidgetItem*)ui->scanJobTableWidget->item(id,2);
        QTableWidgetItem *item2 = (QTableWidgetItem*)ui->scanJobTableWidget->item(id,0);
        profileName = item->text();
        scanID = item2->text();
        values = setupFile->getSectionValue("ScanJobs",scanID).split("|");
        values[2] = QString::number(today);
        temp = values[0] + "|" + values[1] + "|" + values[2] + "|" + values[3];
        setupFile->setSectionValue("ScanJobs",scanID,temp);
        updateScheduleList();
        startScanJob(profileName);
    }
}

void scheduler::slot_removeButtonClicked(int id){
int rc = QMessageBox::information(this,tr("Remove Entry"),tr("Do you realy want to remove this entry?"),QMessageBox::Yes,QMessageBox::No);
QString jobID;

    if (rc == QMessageBox::Yes){
        QTableWidgetItem *item = (QTableWidgetItem*)ui->scanJobTableWidget->item(id,0);
        jobID = item->text();
        setupFile->removeKeyword("ScanJobs",jobID);
        updateScheduleList();
    }
}

void scheduler::slot_logButtonClicked(int id){
QString profileName;
QString logFile;

    QTableWidgetItem *item = (QTableWidgetItem*)ui->scanJobTableWidget->item(id,2);
    profileName = item->text();
    setupFileHandler * tempSF = new setupFileHandler(QDir::homePath() + "/.clamav-gui/profiles/" + profileName + ".ini");
    logFile = tempSF->getSectionValue("Directories","ScanReportToFile");
    if (logFile.left(logFile.indexOf("|")) == "checked"){
        logFile = logFile.mid(logFile.indexOf("|") + 1);
        logViewObject * logViewer = new logViewObject(this,logFile);
        connect(logViewer,SIGNAL(logChanged()),this,SLOT(slot_logChanged()));
        logViewer->setModal(true);
        logViewer->showMaximized();
    } else {
        QMessageBox::information(this,tr("INFO"),tr("No active log-file for this profile specified!"));
    }
}


void scheduler::startScanJob(QString profileName){
setupFileHandler * setupFile = new setupFileHandler(QDir::homePath() + "/.clamav-gui/profiles/" + profileName + ".ini");
QStringList parameters;
QStringList selectedOptions = setupFile->getKeywords("SelectedOptions");
QStringList directoryOptions = setupFile->getKeywords("Directories");
QStringList scanLimitations = setupFile->getKeywords("ScanLimitations");
QString option;
QString checked;
QString value;

    if (setupFile->getSectionBoolValue(profileName,"Recursion") == true){
        parameters << "-r";
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
        if (checked == "checked"){
            if (option == "LoadSupportedDBFiles") parameters << "--database="+ value;
            if (option == "ScanReportToFile") {
                parameters << "--log=" + value;
                QFile file(value);
                if (file.open(QIODevice::ReadWrite|QIODevice::Append|QIODevice::Text)){
                    QTextStream stream(&file);
                    stream << "\n<Scanning startet> " << QDateTime::currentDateTime().toString("yyyy/M/d - hh:mm");
                    file.close();
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
        setupFile->getSectionBoolValue("REGEXP_and_IncludeExclude","SkipPUAPacked") == true?parameters << "--exclude-pua=Packed":parameters << "--include-pua=Packed";
        setupFile->getSectionBoolValue("REGEXP_and_IncludeExclude","SkipPUAPWTool") == true?parameters << "--exclude-pua=PWTool":parameters << "--include-pua=PWTool";
        setupFile->getSectionBoolValue("REGEXP_and_IncludeExclude","SkipPUANetTool") == true?parameters << "--exclude-pua=NetTool":parameters << "--include-pua=NetTool";
        setupFile->getSectionBoolValue("REGEXP_and_IncludeExclude","SkipPUAP2P") == true?parameters << "--exclude-pua=P2P":parameters << "--include-pua=P2P";
        setupFile->getSectionBoolValue("REGEXP_and_IncludeExclude","SkipPUAIRC") == true?parameters << "--exclude-pua=IRC":parameters << "--include-pua=IRC";
        setupFile->getSectionBoolValue("REGEXP_and_IncludeExclude","SkipPUARAT") == true?parameters << "--exclude-pua=RAT":parameters << "--include-pua=RAT";
        setupFile->getSectionBoolValue("REGEXP_and_IncludeExclude","SkipPUANetToolSpy") == true?parameters << "--exclude-pua=NetToolSpy":parameters << "--include-pua=NetToolSpy";
        setupFile->getSectionBoolValue("REGEXP_and_IncludeExclude","SkipPUAServer") == true?parameters << "--exclude-pua=Server":parameters << "--include-pua=Server";
        setupFile->getSectionBoolValue("REGEXP_and_IncludeExclude","SkipPUAScript") == true?parameters << "--exclude-pua=Script":parameters << "--include-pua=Script";
    }

    QStringList directories = setupFile->getSectionValue(profileName,"Directories").split("\n");

    for (int i = 0; i < directories.count(); i++){
        if (directories.at(i) != "") parameters << directories.at(i);
    }

    emit triggerScanJob(profileName,parameters);
}

void scheduler::slot_checkTimerTimeout(){
QStringList scanJobs = setupFile->getKeywords("ScanJobs");
QStringList values;
QString line;
qint64 today = QDateTime::currentDateTime().toMSecsSinceEpoch();
qint64 scanDate;
QDateTime scanDateTime;

    foreach(QString scanJob,scanJobs){
        values = setupFile->getSectionValue("ScanJobs",scanJob).split("|");
        scanDate = values[3].toLongLong();
        if (scanDate < today){
            if (values[0] == "daily"){
                scanDateTime = QDateTime::fromMSecsSinceEpoch(scanDate);
                while (scanDateTime.toMSecsSinceEpoch() < today){
                    scanDateTime = scanDateTime.addDays(1);
                }
            }
            if (values[0] == "weekly"){
                scanDateTime = QDateTime::fromMSecsSinceEpoch(scanDate);
                while (scanDateTime.toMSecsSinceEpoch() < today){
                    scanDateTime = scanDateTime.addDays(7);
                }
            }
            if (values[0] == "monthly"){
                scanDateTime = QDateTime::fromMSecsSinceEpoch(scanDate);
                while (scanDateTime.toMSecsSinceEpoch() < today){
                    scanDateTime = scanDateTime.addMonths(1);
                }
            }
            line = values[0] + "|" + values[1] + "|" + QString::number(today) + "|" + QString::number(scanDateTime.toMSecsSinceEpoch());
            setupFile->setSectionValue("ScanJobs",scanJob,line);
            updateScheduleList();
            startScanJob(values[1]);
        }
    }
}

void scheduler::slot_profileSelectionChanged(){
QString profileName = ui->profileComboBox->currentText();
setupFileHandler * tempSetupFile = new setupFileHandler(QDir::homePath() + "/.clamav-gui/profiles/" + profileName + ".ini");
QStringList targets;
QString targetLabel;
QStringList options;
QString optionLabel;
QString logFile = tempSetupFile->getSectionValue("Directories","ScanReportToFile");

    ui->dailyProfileLabel->setText(profileName);
    ui->weeklyProfileLabel->setText(profileName);
    ui->monthlyProfileLable->setText(profileName);

    targets = tempSetupFile->getSectionValue(profileName,"Directories").split('\n');
    options = tempSetupFile->getKeywords("SelectedOptions");

    if ((targets[0] != "") &(targets.count() > 0)) targetLabel = targetLabel + targets[0];
    for (int i = 1; i < targets.count(); i++){
        if ((targets[i] != "") & (targetLabel != "")){
            targetLabel = targetLabel + "\n" + targets[i];
        } else {
            if (targets[i] != "") targetLabel = targets[i];
        }
    }

    ui->targetInfoLabel->setText(targetLabel);

    if (options.count() > 0) optionLabel = optionLabel + options[0];
    for (int i = 1; i < options.count(); i++){
        optionLabel = optionLabel + "\n" + options[i];
    }

    if (tempSetupFile->getSectionValue("REGEXP_and_IncludeExclude","DontScanFileNamesMatchingRegExp").indexOf("not checked") == -1) optionLabel = optionLabel + "\n" + "--exclude=" + tempSetupFile->getSectionValue("REGEXP_and_IncludeExclude","DontScanFileNamesMatchingRegExp").mid(tempSetupFile->getSectionValue("REGEXP_and_IncludeExclude","DontScanFileNamesMatchingRegExp").indexOf("|") + 1);
    if (tempSetupFile->getSectionValue("REGEXP_and_IncludeExclude","DontScanDiretoriesMatchingRegExp").indexOf("not checked") == -1) optionLabel = optionLabel + "\n" + "--exclude-dir=" + tempSetupFile->getSectionValue("REGEXP_and_IncludeExclude","DontScanDiretoriesMatchingRegExp").mid(tempSetupFile->getSectionValue("REGEXP_and_IncludeExclude","DontScanDiretoriesMatchingRegExp").indexOf("|") + 1);
    if (tempSetupFile->getSectionValue("REGEXP_and_IncludeExclude","OnlyScanFileNamesMatchingRegExp").indexOf("not checked") == -1) optionLabel = optionLabel + "\n" + "--include=" + tempSetupFile->getSectionValue("REGEXP_and_IncludeExclude","OnlyScanFileNamesMatchingRegExp").mid(tempSetupFile->getSectionValue("REGEXP_and_IncludeExclude","OnlyScanFileNamesMatchingRegExp").indexOf("|") + 1);
    if (tempSetupFile->getSectionValue("REGEXP_and_IncludeExclude","OnlyScanDiretoriesMatchingRegExp").indexOf("not checked") == -1) optionLabel = optionLabel + "\n" + "--include-dir=" + tempSetupFile->getSectionValue("REGEXP_and_IncludeExclude","OnlyScanDiretoriesMatchingRegExp").mid(tempSetupFile->getSectionValue("REGEXP_and_IncludeExclude","OnlyScanDiretoriesMatchingRegExp").indexOf("|") + 1);

    if (tempSetupFile->getSectionBoolValue("REGEXP_and_IncludeExclude","EnablePUAOptions") == true) {
        if (tempSetupFile->getSectionBoolValue("REGEXP_and_IncludeExclude","LoadPUAPacked") == true) optionLabel = optionLabel + "\n" + "--include-pua=Packed";
        if (tempSetupFile->getSectionBoolValue("REGEXP_and_IncludeExclude","LoadPUAPWTool") == true) optionLabel = optionLabel + "\n" + "--include-pua=PWTool";
        if (tempSetupFile->getSectionBoolValue("REGEXP_and_IncludeExclude","LoadPUANetTool") == true) optionLabel = optionLabel + "\n" + "--include-pua=NetTool";
        if (tempSetupFile->getSectionBoolValue("REGEXP_and_IncludeExclude","LoadPUAP2P") == true) optionLabel = optionLabel + "\n" + "--include-pua=P2P";
        if (tempSetupFile->getSectionBoolValue("REGEXP_and_IncludeExclude","LoadPUAIRC") == true) optionLabel = optionLabel + "\n" + "--include-pua=IRC";
        if (tempSetupFile->getSectionBoolValue("REGEXP_and_IncludeExclude","LoadPUARAT") == true) optionLabel = optionLabel + "\n" + "--include-pua=RAT";
        if (tempSetupFile->getSectionBoolValue("REGEXP_and_IncludeExclude","LoadPUANetToolSpy") == true) optionLabel = optionLabel + "\n" + "--include-pua=NetToolSpy";
        if (tempSetupFile->getSectionBoolValue("REGEXP_and_IncludeExclude","LoadPUAServer") == true) optionLabel = optionLabel + "\n" + "--include-pua=Server";
        if (tempSetupFile->getSectionBoolValue("REGEXP_and_IncludeExclude","LoadPUAScript") == true) optionLabel = optionLabel + "\n" + "--include-pua=Script";
        if (tempSetupFile->getSectionBoolValue("REGEXP_and_IncludeExclude","LoadPUAAndr") == true) optionLabel = optionLabel + "\n" + "--include-pua=Andr";
        if (tempSetupFile->getSectionBoolValue("REGEXP_and_IncludeExclude","LoadPUAJava") == true) optionLabel = optionLabel + "\n" + "--include-pua=Java";
        if (tempSetupFile->getSectionBoolValue("REGEXP_and_IncludeExclude","LoadPUAOsx") == true) optionLabel = optionLabel + "\n" + "--include-pua=Osx";
        if (tempSetupFile->getSectionBoolValue("REGEXP_and_IncludeExclude","LoadPUATool") == true) optionLabel = optionLabel + "\n" + "--include-pua=Tool";
        if (tempSetupFile->getSectionBoolValue("REGEXP_and_IncludeExclude","LoadPUAUnix") == true) optionLabel = optionLabel + "\n" + "--include-pua=Unix";
        if (tempSetupFile->getSectionBoolValue("REGEXP_and_IncludeExclude","LoadPUAWin") == true) optionLabel = optionLabel + "\n" + "--include-pua=Win";
    }

    QString value = "";
    value = tempSetupFile->getSectionValue("ScanLimitations","Files larger than this will be skipped and assumed clean");
    if ((value != "") && (value.indexOf("not checked") == -1)) optionLabel = optionLabel + "\n" + "--max-filesize=" + value.mid(value.indexOf("|") + 1);
    value = tempSetupFile->getSectionValue("ScanLimitations","The maximum amount of data to scan for each container file");
    if ((value != "") && (value.indexOf("not checked") == -1)) optionLabel = optionLabel + "\n" + "--max-scansize=" + value.mid(value.indexOf("|") + 1);
    value = tempSetupFile->getSectionValue("ScanLimitations","The maximum number of files to scan for each container file");
    if ((value != "") && (value.indexOf("not checked") == -1)) optionLabel = optionLabel + "\n" + "--max-files=" + value.mid(value.indexOf("|") + 1);
    value = tempSetupFile->getSectionValue("ScanLimitations","Maximum archive recursion level for container file");
    if ((value != "") && (value.indexOf("not checked") == -1)) optionLabel = optionLabel + "\n" + "--max-recursion=" + value.mid(value.indexOf("|") + 1);
    value = tempSetupFile->getSectionValue("ScanLimitations","Maximum directory recursion level");
    if ((value != "") && (value.indexOf("not checked") == -1)) optionLabel = optionLabel + "\n" + "--max-dir-recursion=" + value.mid(value.indexOf("|") + 1);
    value = tempSetupFile->getSectionValue("ScanLimitations","Maximum size file to check for embedded PE");
    if ((value != "") && (value.indexOf("not checked") == -1)) optionLabel = optionLabel + "\n" + "--max-embeddedpe=" + value.mid(value.indexOf("|") + 1);
    value = tempSetupFile->getSectionValue("ScanLimitations","Maximum size of HTML file to normalize");
    if ((value != "") && (value.indexOf("not checked") == -1)) optionLabel = optionLabel + "\n" + "--max-htmlnormalized=" + value.mid(value.indexOf("|") + 1);
    value = tempSetupFile->getSectionValue("ScanLimitations","Maximum size of normalized HTML file to scan");
    if ((value != "") && (value.indexOf("not checked") == -1)) optionLabel = optionLabel + "\n" + "--max-htmlnotags=" + value.mid(value.indexOf("|") + 1);
    value = tempSetupFile->getSectionValue("ScanLimitations","Maximum size of script file to normalize");
    if ((value != "") && (value.indexOf("not checked") == -1)) optionLabel = optionLabel + "\n" + "--max-scriptnormalize=" + value.mid(value.indexOf("|") + 1);
    value = tempSetupFile->getSectionValue("ScanLimitations","Maximum size zip to type reanalyze");
    if ((value != "") && (value.indexOf("not checked") == -1)) optionLabel = optionLabel + "\n" + "--max-ziptypercg=" + value.mid(value.indexOf("|") + 1);
    value = tempSetupFile->getSectionValue("ScanLimitations","Maximum number of partitions in disk image to be scanned");
    if ((value != "") && (value.indexOf("not checked") == -1)) optionLabel = optionLabel + "\n" + "--max-partitions=" + value.mid(value.indexOf("|") + 1);
    value = tempSetupFile->getSectionValue("ScanLimitations","Maximum number of icons in PE file to be scanned");
    if ((value != "") && (value.indexOf("not checked") == -1)) optionLabel = optionLabel + "\n" + "--max-iconspe=" + value.mid(value.indexOf("|") + 1);
    value = tempSetupFile->getSectionValue("ScanLimitations","Bytecode timeout in milliseconds");
    if ((value != "") && (value.indexOf("not checked") == -1)) optionLabel = optionLabel + "\n" + "--bytecode-timeout=" + value.mid(value.indexOf("|") + 1);
    value = tempSetupFile->getSectionValue("ScanLimitations","Collect and print execution statistics");
    if ((value != "") && (value.indexOf("not checked") == -1)) optionLabel = optionLabel + "\n" + "--statistics " + value.mid(value.indexOf("|") + 1);
    value = tempSetupFile->getSectionValue("ScanLimitations","Structured SSN Format");
    if ((value != "") && (value.indexOf("not checked") == -1)) optionLabel = optionLabel + "\n" + "--structured-ssn-format=" + value.mid(value.indexOf("|") + 1);
    value = tempSetupFile->getSectionValue("ScanLimitations","Structured SSN Count");
    if ((value != "") && (value.indexOf("not checked") == -1)) optionLabel = optionLabel + "\n" + "--structured-ssn-count=" + value.mid(value.indexOf("|") + 1);
    value = tempSetupFile->getSectionValue("ScanLimitations","Structured CC Count");
    if ((value != "") && (value.indexOf("not checked") == -1)) optionLabel = optionLabel + "\n" + "--structured-cc-count=" + value.mid(value.indexOf("|") + 1);
    value = tempSetupFile->getSectionValue("ScanLimitations","Structured CC Mode");
    if ((value != "") && (value.indexOf("not checked") == -1)) optionLabel = optionLabel + "\n" + "--structured-cc-mode=" + value.mid(value.indexOf("|") + 1);
    value = tempSetupFile->getSectionValue("ScanLimitations","Max Scan-Time");
    if ((value != "") && (value.indexOf("not checked") == -1)) optionLabel = optionLabel + "\n" + "--max-scantime=" + value.mid(value.indexOf("|") + 1);
    value = tempSetupFile->getSectionValue("ScanLimitations","Max recursion to HWP3 parsing function");
    if ((value != "") && (value.indexOf("not checked") == -1)) optionLabel = optionLabel + "\n" + "--max-rechwp3=" + value.mid(value.indexOf("|") + 1);
    value = tempSetupFile->getSectionValue("ScanLimitations","Max calls to PCRE match function");
    if ((value != "") && (value.indexOf("not checked") == -1)) optionLabel = optionLabel + "\n" + "--pcre-match-limit=" + value.mid(value.indexOf("|") + 1);
    value = tempSetupFile->getSectionValue("ScanLimitations","Max recursion calls to the PCRE match function");
    if ((value != "") && (value.indexOf("not checked") == -1)) optionLabel = optionLabel + "\n" + "--pcre-recmatch-limit=" + value.mid(value.indexOf("|") + 1);
    value = tempSetupFile->getSectionValue("ScanLimitations","Max PCRE file size");
    if ((value != "") && (value.indexOf("not checked") == -1)) optionLabel = optionLabel + "\n" + "--pcre-max-filesize=" + value.mid(value.indexOf("|") + 1);
    value = tempSetupFile->getSectionValue("ScanLimitations","Database outdated if older than x days");
    if ((value != "") && (value.indexOf("not checked") == -1)) optionLabel = optionLabel + "\n" + " --fail-if-cvd-older-than=" + value.mid(value.indexOf("|") + 1);

    if (tempSetupFile->getSectionBoolValue(profileName,"Recursion") == true){
        if (optionLabel != ""){
            optionLabel = optionLabel + "\n" + "-r";
        } else {
            optionLabel = "-r";
        }
    }

    optionLabel = optionLabel.replace("<equal>","=");
    ui->optionsInfoLabel->setText(optionLabel);

    if (logFile.left(logFile.indexOf("|")) == "checked"){
        logFile = logFile.mid(logFile.indexOf("|") + 1);
    } else {
        logFile = "";
    }
    ui->logFileLabel->setText("Log-File : " + logFile);
}

void scheduler::slot_logChanged(){
    emit logChanged();
}

void scheduler::slot_disableScheduler()
{
    ui->scanJobTableWidget->setEnabled(false);
}
