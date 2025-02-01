#include "logviewerobject.h"
#include "ui_logviewerobject.h"

logViewerObject::logViewerObject(QWidget *parent) : QWidget(parent), ui(new Ui::logViewerObject)
{
    setupfile = new setupFileHandler(QDir::homePath() + "/.clamav-gui/settings.ini");
    ui->setupUi(this);
    slot_profilesChanged();
}

logViewerObject::~logViewerObject()
{
    delete ui;
}

void logViewerObject::slot_profilesChanged(){
QStringList profiles = setupfile->getKeywords("Profiles");
QStringList profilesWithLog;
QStringList values;
QString actualProfileName = ui->profileComboBox->currentText();

    foreach(QString profile,profiles){
        setupFileHandler * sf = new setupFileHandler(QDir::homePath() + "/.clamav-gui/profiles/" + profile + ".ini");
        values = sf->getSectionValue("Directories","ScanReportToFile").split("|");
        if (values.count() == 2){
            QFile tempFile(values[1]);
            if (tempFile.exists()) profilesWithLog << profile;
        }
    }

    ui->profileComboBox->clear();
    ui->profileComboBox->addItems(profilesWithLog);
    if (ui->profileComboBox->findText(actualProfileName) != -1){
        ui->profileComboBox->setCurrentIndex(ui->profileComboBox->findText(actualProfileName));
        loadLogFile(actualProfileName);
    } else {
        loadLogFile(ui->profileComboBox->currentText());
    }
}

void logViewerObject::loadLogFile(QString profile){
setupFileHandler * sf = new setupFileHandler(QDir::homePath() + "/.clamav-gui/profiles/" + profile + ".ini");
QString buffer;
QStringList logs;
QString tabHeader;
QStringList values;

    while(ui->logTab->count() > 0){
        ui->logTab->removeTab(0);
    }
    values = sf->getSectionValue("Directories","ScanReportToFile").split("|");
    if (values.count() == 2){
        QFile file(values[1]);
        logFileName = values[1];
        if (file.open(QIODevice::ReadOnly)){
            QTextStream stream(&file);
            buffer = stream.readAll();
            logs = buffer.split("<Scanning startet>");
            for (int i = 1; i < logs.count(); i++){
                partialLogObject * log = new partialLogObject(this,logs[i]);
                tabHeader = logs[i].mid(1,logs[i].indexOf("\n") - 1);
                ui->logTab->insertTab(0,log,QIcon(":/icons/icons/information.png"),tabHeader);
                //ui->logTab->addTab(log,QIcon(":/icons/icons/information.png"),tabHeader);
            }
            ui->logTab->setCurrentIndex(0);
            file.close();
        }
    }
}

void logViewerObject::slot_profileSeclectionChanged(){
    loadLogFile(ui->profileComboBox->currentText());
}

void logViewerObject::saveLog(){
QString logText;
partialLogObject * log;

    QFile logFile(logFileName);
    if (logFile.open(QIODevice::ReadWrite|QIODevice::Truncate|QIODevice::Text)){
        QTextStream stream(&logFile);
        for (int i = 0; i < ui->logTab->count(); i++){
            log = (partialLogObject*)ui->logTab->widget(i);
            logText = logText + log->getLogText();
        }
        stream << logText;
        logFile.close();
    }
}

void logViewerObject::slot_clearLogButtonClicked(){
int currentTab= ui->logTab->currentIndex();

    if (currentTab > -1){
        if (QMessageBox::question(this,tr("Clear Log"),tr("Do you realy want to remove this partial log?"),QMessageBox::Yes,QMessageBox::No) == QMessageBox::Yes){
            ui->logTab->removeTab(currentTab);
            saveLog();
        }
    }
}

void logViewerObject::slot_clearAllButtonClicked(){
int count = ui->logTab->count();

    if ((count > 0) && (QMessageBox::question(this,tr("Clear Log"),tr("Do you realy want to remove the complete log?"),QMessageBox::Yes,QMessageBox::No) == QMessageBox::Yes)){
        for (int i = 0; i < count; i++){
            ui->logTab->removeTab(ui->logTab->currentIndex());
        }
    }
    saveLog();
}
