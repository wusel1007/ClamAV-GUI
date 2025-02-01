#include "scantab.h"
#include "ui_scantab.h"

scanTab::scanTab(QWidget *parent) : QWidget(parent),ui(new Ui::scanTab)
{
    ui->setupUi(this);
    logHighLighter = new highlighter(ui->logPlainTextEdit->document());
    setupFile = new setupFileHandler(QDir::homePath() + "/.clamav-gui/settings.ini");

    model = new CFileSystemModel;
    model->setFilter(QDir::AllDirs|QDir::NoDotAndDotDot);

    ui->treeView->setModel(model);
    ui->treeView->hideColumn(1);
    ui->treeView->hideColumn(2);
    ui->treeView->hideColumn(3);

    QStringList directories = setupFile->getSectionValue("Settings","Directories").split("\n");
    ui->treeView->collapseAll();
    model->unCheckAll();
    model->setRootPath("/");
    foreach(QString dir,directories){
        if (dir != ""){
            model->setChecked(dir,true);
            QModelIndex index = model->index(dir);
            ui->treeView->scrollTo(index);
            ui->treeView->setCurrentIndex(index);
        }
    }

    setupFile->getSectionBoolValue("Settings","RecursivScan") == true?ui->recursivCheckBox->setChecked(true):ui->recursivCheckBox->setChecked(false);
    ui->virusFoundComboBox->setCurrentIndex(setupFile->getSectionIntValue("Settings","VirusFoundComboBox"));

    whoamiProcess = new QProcess(this);
    connect(whoamiProcess,SIGNAL(finished(int)),this,SLOT(slot_whoamiProcessFinished()));
    whoamiProcess->start("whoami",QStringList());
}

scanTab::~scanTab()
{
    delete ui;
}

void scanTab::slot_scanButtonClicked(){
QList <QPersistentModelIndex> list = model->checkedIndexes.values();
QStringList scanObjects;

    for (int i = 0; i < list.count(); i++){
        if (list[i].data(QFileSystemModel::FilePathRole).toString() != "") scanObjects << list[i].data(QFileSystemModel::FilePathRole).toString();
    }
    if (scanObjects.count() > 0) emit triggerScanRequest(scanObjects);
}

void scanTab::slot_basePathButtonClicked(){
QModelIndex index = model->index("/");

    ui->treeView->scrollTo(index);
    ui->treeView->setCurrentIndex(index);
}

void scanTab::slot_homePathButtonClicked(){
QModelIndex index = model->index(QDir::homePath());

    ui->treeView->scrollTo(index);
    ui->treeView->setCurrentIndex(index);
}

void scanTab::slot_updateDeviceList(){
QDir dir("/run/media/" + username.trimmed());
QDir dir2("/run/media");
QDir dirUbuntu("/media/" + username.trimmed());
QDir dirUbuntu2("/media");
fileSystemWatcher = new QFileSystemWatcher(this);
fileSystemWatcherUbuntu = new QFileSystemWatcher(this);
QStringList filters;
    filters << "*";
QStringList dirs = dir.entryList(filters,QDir::AllDirs|QDir::NoDotAndDotDot);
QStringList dirsUbuntu = dirUbuntu.entryList(filters,QDir::AllDirs|QDir::NoDotAndDotDot);
QLabel * deviceLabel = new QLabel(tr("Devices"));
QLayoutItem *item = NULL;
    while ((item = ui->devicesFrame->layout()->takeAt(0)) != 0) {
        delete item->widget();
    }

    if (dir.exists() == true){
        fileSystemWatcher->addPath("/run/media/" + username.trimmed());
        connect(fileSystemWatcher,SIGNAL(directoryChanged(QString)),this,SLOT(slot_updateDeviceList()));
    } else {
        if (dir2.exists() == true) {
            fileSystemWatcher->addPath("/run/media");
            connect(fileSystemWatcher,SIGNAL(directoryChanged(QString)),this,SLOT(slot_updateDeviceList()));
        }
    }

    if (dirUbuntu.exists() == true) {
        fileSystemWatcherUbuntu->addPath("/media/" + username.trimmed());
        connect(fileSystemWatcherUbuntu,SIGNAL(directoryChanged(QString)),this,SLOT(slot_updateDeviceList()));
    } else {
        if (dirUbuntu2.exists() == true) {
            fileSystemWatcherUbuntu->addPath("/media");
            connect(fileSystemWatcherUbuntu,SIGNAL(directoryChanged(QString)),this,SLOT(slot_updateDeviceList()));
        }
    }

    deviceLabel->setStyleSheet("background:#c0c0c0;padding:3px;");
    ui->devicesFrame->layout()->addWidget(deviceLabel);

    deviceGroup = new QButtonGroup(this);
    connect(deviceGroup,SIGNAL(buttonClicked(int)),this,SLOT(slot_deviceButtonClicked(int)));

    if (dirs.count() > 0){
        devices.clear();
        buttonID = 0;
        foreach (QString entry,dirs){
            devices << (QString)("/run/media/" + username.trimmed() + "/" + entry);
            dragablePushButton * button = new dragablePushButton(QIcon(":/icons/icons/media.png"),entry.mid(entry.lastIndexOf("/") + 1),this,(QString)("/var/run/media/" + username.trimmed() + "/" + entry));
            connect(button,SIGNAL(dragStarted()),this,SLOT(slot_requestDropZoneVisible()));
            button->setIconSize(QSize(28,28));
            button->setStyleSheet("text-align:left");
            button->setFlat(true);
            deviceGroup->addButton(button,buttonID);
            ui->devicesFrame->layout()->addWidget(button);
            buttonID++;
        }
    }

    if (dirsUbuntu.count() > 0){
        devices.clear();
        buttonID = 0;
        foreach (QString entry,dirsUbuntu){
            devices << (QString)("/media/" + username.trimmed() + "/" + entry);
            dragablePushButton * button = new dragablePushButton(QIcon(":/icons/icons/media.png"),entry.mid(entry.lastIndexOf("/") + 1),this,(QString)("/media/" + username.trimmed() + "/" + entry));
            connect(button,SIGNAL(dragStarted()),this,SLOT(slot_requestDropZoneVisible()));
            button->setIconSize(QSize(28,28));
            button->setStyleSheet("text-align:left");
            button->setFlat(true);
            deviceGroup->addButton(button,buttonID);
            ui->devicesFrame->layout()->addWidget(button);
            buttonID++;
        }
    }

    //model->refresh();
}

void scanTab::slot_deviceButtonClicked(int buttonIndex){
QModelIndex index = model->index(devices.at(buttonIndex));

    ui->treeView->scrollTo(index);
    ui->treeView->setCurrentIndex(index);
}

void scanTab::slot_whoamiProcessFinished(){

    username = whoamiProcess->readAllStandardOutput();

    slot_updateDeviceList();
}

void scanTab::slot_recursivScanCheckBoxClicked(){
    setupFile->setSectionValue("Settings","RecursivScan",ui->recursivCheckBox->isChecked());
}

void scanTab::slot_virusFoundComboBoxChanged(){
    setupFile->setSectionValue("Settings","VirusFoundComboBox",ui->virusFoundComboBox->currentIndex());
}

bool scanTab::recursivChecked(){
    return ui->recursivCheckBox->isChecked();
}

void scanTab::setStatusMessage(QString message){
QString currentFile;
int start,end;

    while(message.indexOf("Scanning") != -1){
        start = message.indexOf("Scanning");
        end = message.indexOf("\n",start);
        currentFile = message.mid(start,end - start + 1);
        message.replace(currentFile,"");
        currentFile = currentFile.mid(currentFile.lastIndexOf("/") + 1);
        currentFile.replace("\n","");
        ui->currentFileLabel->setText("Scanning : " + currentFile);
    }
    ui->logPlainTextEdit->insertPlainText(message);
    ui->logPlainTextEdit->ensureCursorVisible();
}

void scanTab::clearLogMessage(){
    ui->logPlainTextEdit->setPlainText("");
}

void scanTab::slot_abortScan(){
    ui->currentFileLabel->setText(tr("Scanning aborted ......"));
    emit abortScan();
}

void scanTab::slot_enableForm(bool mode){

    if (mode == false){
        busyLabel = new QLabel(this);
        busyLabel->setStyleSheet("background:transparent");
        busyLabel->setGeometry((this->width() - 80) / 2,(this->height() - 80) / 2,80,80);
        movie = new QMovie(":/icons/icons/gifs/spinning_segments_large.gif");
        busyLabel->setMovie(movie);
        busyLabel->show();
        movie->start();
    } else {
        if (movie != 0){
            movie->stop();
            delete movie;
            delete busyLabel;
        }
    }

    ui->deviceFrame->setEnabled(mode);
    ui->startScanButton->setEnabled(mode);
    ui->stopScanButton->setEnabled(!mode);
    ui->recursivCheckBox->setEnabled(mode);
    ui->virusFoundComboBox->setEnabled(mode);
    ui->treeView->setEnabled(mode);
}

void scanTab::setStatusBarMessage(QString message, QString bgColor){
    ui->currentFileLabel->setText(message);
    ui->currentFileLabel->setStyleSheet("background:"+bgColor);
}

int scanTab::getVirusFoundComboBoxValue()
{
    return ui->virusFoundComboBox->currentIndex();
}

void scanTab::slot_requestDropZoneVisible(){
    emit requestDropZoneVisible();
}

void scanTab::slot_dirtreeSelectionChanged(){
QList <QPersistentModelIndex> list = model->checkedIndexes.values();
QString directories;

    for (int i = 0; i < list.count(); i++){
        if (i < list.count() - 1){
            directories = directories + list[i].data(QFileSystemModel::FilePathRole).toString() + "\n";
        } else {
            directories = directories + list[i].data(QFileSystemModel::FilePathRole).toString();
        }
    }

    setupFile->setSectionValue("Settings","Directories",directories);
}

void scanTab::slot_updateDBPath(QString path)
{
    setupFile->setSectionValue("Directories", "LoadSupportedDBFiles", "checked|" + path);
}

void scanTab::slot_disableScanButton()
{
    ui->startScanButton->setEnabled(false);
}

void scanTab::slot_receiveVersionInformation(QString info)
{
   ui->versionTextBrowser->setHtml(info);
}
