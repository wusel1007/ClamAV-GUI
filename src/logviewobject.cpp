#include "logviewobject.h"
#include "ui_logviewobject.h"

logViewObject::logViewObject(QWidget *parent, QString fileName) : QDialog(parent), ui(new Ui::logViewObject)
{
    setWindowFlags(((this->windowFlags() | Qt::CustomizeWindowHint) & ~Qt::WindowCloseButtonHint & ~Qt::WindowContextHelpButtonHint) );
    ui->setupUi(this);
    logFileName = fileName;
    loadLogFile(fileName);
    searchPosition = 0;
    contentChanged = false;
}

logViewObject::~logViewObject()
{
    delete ui;
}

void logViewObject::closeEvent(QCloseEvent *event){
    this->accept();
    event->ignore();
}

void logViewObject::keyPressEvent(QKeyEvent * event){
    if (event->key() == Qt::Key_Escape) event->ignore(); else event->accept();
}

void logViewObject::loadLogFile(QString filename){
QFile file(filename);
QString buffer;
QStringList logs;
QString tabHeader;

    while(ui->logTab->count() > 0){
        ui->logTab->removeTab(0);
    }
    if (file.open(QIODevice::ReadOnly)){
        QTextStream stream(&file);
        buffer = stream.readAll();
        logs = buffer.split("<Scanning startet>");
        for (int i = 1; i < logs.count(); i++){
            partialLogObject * log = new partialLogObject(this,logs[i]);
            tabHeader = logs[i].mid(1,logs[i].indexOf("\n") - 1);
            ui->logTab->addTab(log,QIcon(":/icons/icons/information.png"),tabHeader);
        }
        file.close();
    }
}

void logViewObject::slot_closeButtonClicked(){
QString logText;
partialLogObject * log;

    if (contentChanged){
        if (QMessageBox::information(this,tr("INFO"),tr("Log-File was modified. Do you wanna save the changes?"),QMessageBox::Yes,QMessageBox::No) == QMessageBox::Yes){
            if (ui->logTab->count() > 0){
                for (int i = 0; i < ui->logTab->count(); i++){
                    log = (partialLogObject*)ui->logTab->widget(i);
                    logText = logText + log->getLogText();
                    QFile logFile(logFileName);
                    if (logFile.open(QIODevice::ReadWrite|QIODevice::Truncate|QIODevice::Text)){
                        QTextStream stream(&logFile);
                        stream << logText;
                        logFile.close();
                    }
                }
            } else {
                QFile logFile(logFileName);
                if (logFile.open(QIODevice::ReadWrite|QIODevice::Truncate|QIODevice::Text)){
                    QTextStream stream(&logFile);
                    stream << "";
                    logFile.close();
                }
            }
        }
    }
    this->accept();
}

void logViewObject::slot_clearLogButtonClicked(){
int currentTab= ui->logTab->currentIndex();

    if (currentTab > -1){
        if (QMessageBox::question(this,tr("Clear Log"),tr("Do you realy want to remove this partial log?"),QMessageBox::Yes,QMessageBox::No) == QMessageBox::Yes){
            ui->logTab->removeTab(currentTab);
            contentChanged = true;
            emit logChanged();
        }
    }
}
