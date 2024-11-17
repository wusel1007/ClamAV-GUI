#include "schedulescanobject.h"
#include "ui_schedulescanobject.h"

scheduleScanObject::scheduleScanObject(QWidget *parent, QString name, QStringList parameters) : QDialog(parent), ui(new Ui::scheduleScanObject),scanJob(name)
{
    setWindowFlags(((this->windowFlags() | Qt::CustomizeWindowHint) & ~Qt::WindowCloseButtonHint & ~Qt::WindowContextHelpButtonHint) );
    ui->setupUi(this);
    if (name=="Direct Scan") {
        directScan = true;
        ui->headerLabel->setText("Direct Scan");
    } else {
        directScan = false;
    }
    logHighLighter = new highlighter(ui->logMessagePlainTextEdit->document());
    scanProcess = new QProcess(this);
    QString message = "clamscan ";
    for (int i = 0;i < parameters.count();i++){
        message = message + " " + parameters.at(i);
    }
    if (directScan == true) ui->logMessagePlainTextEdit->appendPlainText(message);
    ui->scanJobHeader->setText(tr("Scan-Job: ") + name);
    this->setWindowTitle(tr("Scheduled Scan-Job: ") + name);
    connect(scanProcess,SIGNAL(readyReadStandardError()),this,SLOT(slot_scanProcessHasErrOutput()));
    connect(scanProcess,SIGNAL(readyReadStandardOutput()),this,SLOT(slot_scanProcessHasStdOutput()));
    connect(scanProcess,SIGNAL(finished(int,QProcess::ExitStatus)),this,SLOT(slot_scanProcessFinished(int,QProcess::ExitStatus)));
    scanProcess->start("clamscan",parameters);
    ui->currentFileLabel->setText(tr("Scanning started ......."));

    busyLabel = new QLabel(this);
    busyLabel->setStyleSheet("background:transparent");
    busyLabel->setGeometry((this->width() - 80) / 2,(this->height() - 80) / 2,80,80);
    movie = new QMovie(":/icons/icons/gifs/spinning_segments_large.gif");
    busyLabel->setMovie(movie);
    busyLabel->show();
    movie->start();

    closeWindowTimer = new QTimer(this);
    closeWindowTimer->setSingleShot(true);
    connect(closeWindowTimer,SIGNAL(timeout()),this,SLOT(slot_closeWindowTimerTimeout()));
    closeWindowCounter = 100;
    errorStart = 0;
    infectedStart = 0;
}

void scheduleScanObject::accept(){
    if (directScan == false) emit closeRequest(this); else this->done(0);
}

void scheduleScanObject::closeEvent(QCloseEvent *event){
    this->accept();
    event->ignore();
}

void scheduleScanObject::keyPressEvent(QKeyEvent * event){
    if (event->key() == Qt::Key_Escape) event->ignore(); else event->accept();
}

scheduleScanObject::~scheduleScanObject()
{
    delete ui;
}

void scheduleScanObject::slot_closeButtonClicked(){
    this->accept();
}

void scheduleScanObject::slot_stopButtonClicked(){
    if (scanProcess->state() == QProcess::Running) scanProcess->kill();
    ui->closeButton->setEnabled(true);
    movie->stop();
}

void scheduleScanObject::slot_scanProcessHasStdOutput(){
QString message = scanProcess->readAllStandardOutput();
QString currentFile;
int start,end;

    while(message.indexOf("Scanning") != -1){
        start = message.indexOf("Scanning");
        end = message.indexOf("\n",start);
        currentFile = message.mid(start,end - start + 1);
        message.replace(currentFile,"");
        currentFile = currentFile.mid(currentFile.lastIndexOf("/") + 1);
        currentFile.replace("\n","");
        ui->currentFileLabel->setText(tr("Scanning : ") + currentFile);
    }
    ui->logMessagePlainTextEdit->insertPlainText(message);
    ui->logMessagePlainTextEdit->ensureCursorVisible();
}

void scheduleScanObject::slot_scanProcessHasErrOutput(){
    QString message = scanProcess->readAllStandardOutput();
    QString currentFile;
    int start,end;

        while(message.indexOf("Scanning") != -1){
            start = message.indexOf("Scanning");
            end = message.indexOf("\n",start);
            currentFile = message.mid(start,end - start + 1);
            message.replace(currentFile,"");
            currentFile = currentFile.mid(currentFile.lastIndexOf("/") + 1);
            currentFile.replace("\n","");
            ui->currentFileLabel->setText(tr("Scanning : ") + currentFile);
        }
        ui->logMessagePlainTextEdit->insertPlainText(message);
        ui->logMessagePlainTextEdit->ensureCursorVisible();
}

void scheduleScanObject::slot_scanProcessFinished(int exitCode,QProcess::ExitStatus status){
exitCode = exitCode;
status = status;
QString temp;
int pos,end;

    if (movie != 0){
        movie->stop();
        delete movie;
        delete busyLabel;
    }
    if (status == QProcess::CrashExit) {
        ui->currentFileLabel->setText(tr("Scan Process aborted ....."));
        ui->currentFileLabel->setStyleSheet("background:red");
        emit sendStatusReport(1,tr("Scan-Job: ") + scanJob,tr("Scan Process aborted ....."));
    } else {
        if (exitCode == 0) {
            ui->currentFileLabel->setText(tr("Scan-Process finished ...... no Virus found!"));
            ui->currentFileLabel->setStyleSheet("background:green");
            emit sendStatusReport(0,"Scan-Job: " + scanJob,tr("Scan-Process finished ...... no Virus found!"));
            closeWindowTimer->start(1000);
            countDown = new QRoundProgressBar(this);
            countDown->setGeometry((this->width() - 80) / 2,(this->height() - 80) / 2,80,80);
            countDown->setMaximum(100);
            countDown->setMinimum(0);
            countDown->setValue(100);
            countDown->setBarStyle(QRoundProgressBar::StyleDonut);
            countDown->setFormat("");
            countDown->show();
        } else
            if (exitCode == 1) {
                ui->currentFileLabel->setText(tr("Scan-Process finished ...... a Virus was found!"));
                ui->currentFileLabel->setStyleSheet("background:red");
                emit sendStatusReport(2,"Scan-Job: " + scanJob,tr("Scan Process finished ..... a Virus was found!"));
            } else {
                ui->currentFileLabel->setText(tr("Scan-Process finished ...... an Error occurred!"));
                ui->currentFileLabel->setStyleSheet("background:red");
                emit sendStatusReport(1,"Scan-Job: " + scanJob,tr("Scan Process finished ..... an Error occurred!"));
            }
    }
    ui->closeButton->setEnabled(true);
    ui->stopButton->setEnabled(false);
    if (status != QProcess::CrashExit){
        temp = ui->logMessagePlainTextEdit->toPlainText();
        pos = temp.indexOf("Engine version:");
        if (pos != -1){
            end = temp.indexOf("\n",pos);
            ui->engineVersionLabel->setText(tr("Engine Version: ") + temp.mid(pos + 15,end - pos - 15));
        } else {
            ui->engineVersionLabel->setText(tr("Engine Version: n/a"));
        }
        pos = temp.indexOf("Infected files:");
        if (pos != -1){
            end = temp.indexOf("\n",pos);
            ui->infectedFilesLabel->setText(tr("Infected files: ") + temp.mid(pos + 15,end - pos - 15));
        } else {
            ui->infectedFilesLabel->setText(tr("Infected files: n/a"));
        }
        pos = temp.indexOf("Scanned directories:");
        if (pos != -1){
            end = temp.indexOf("\n",pos);
            ui->scannedDirectoriesLabel->setText(tr("Scanned Directories: ") + temp.mid(pos + 20,end - pos - 20));
        } else {
            ui->scannedDirectoriesLabel->setText(tr("Scanned Directories: n/a"));
        }
        pos = temp.indexOf("Scanned files:");
        if (pos != -1){
            end = temp.indexOf("\n",pos);
            ui->scannedFilesLabel->setText(tr("Scanned Files: ") + temp.mid(pos + 14,end - pos - 14));
        } else {
            ui->scannedFilesLabel->setText(tr("Scanned Files: n/a"));
        }
        pos = temp.indexOf("Total errors::");
        if (pos != -1){
            end = temp.indexOf("\n",pos);
            ui->errorsLabel->setText(tr("Total Errors: ") + temp.mid(pos + 13,end - pos - 13));
        } else {
            ui->errorsLabel->setText(tr("Total Errors: 0"));
        }
    }
    emit scanProcessFinished();
}

void scheduleScanObject::slot_closeWindowTimerTimeout(){
    closeWindowCounter--;
    if (closeWindowCounter == 0){
        this->accept();
    } else {
        countDown->setValue(closeWindowCounter);
        closeWindowTimer->start(100);
    }
}

void scheduleScanObject::slot_totalErrorButtonClicked(){
QTextCursor cursor = ui->logMessagePlainTextEdit->textCursor();
QStringList searchStrings;
QString searchString;
int pos = -1;
int index = 0;

    searchStrings << "Access denied" << "Empty file";

    while ((pos == -1) & (index < searchStrings.count())){
        searchString = searchStrings.at(index);
        index ++;
        pos = ui->logMessagePlainTextEdit->toPlainText().indexOf(searchString,errorStart);
    }

    if (pos > -1){
        errorStart = pos + searchString.length();
        cursor.movePosition(QTextCursor::Start);
        cursor.movePosition(QTextCursor::Right,QTextCursor::MoveAnchor,pos);
        cursor.movePosition(QTextCursor::Right,QTextCursor::KeepAnchor,searchString.length());
        ui->logMessagePlainTextEdit->setTextCursor(cursor);
        ui->logMessagePlainTextEdit->ensureCursorVisible();
        if (ui->logMessagePlainTextEdit->toPlainText().indexOf(searchString,errorStart) == -1) errorStart = 0;
    }
}

void scheduleScanObject::slot_infectedFilesButtonClicked(){
QTextCursor cursor = ui->logMessagePlainTextEdit->textCursor();
QString searchString = "FOUND";
int pos = ui->logMessagePlainTextEdit->toPlainText().toUpper().indexOf(searchString,infectedStart);

    if (pos > -1){
        infectedStart = pos + searchString.length();
        cursor.movePosition(QTextCursor::Start);
        cursor.movePosition(QTextCursor::Right,QTextCursor::MoveAnchor,pos);
        cursor.movePosition(QTextCursor::Right,QTextCursor::KeepAnchor,searchString.length());
        ui->logMessagePlainTextEdit->setTextCursor(cursor);
        ui->logMessagePlainTextEdit->ensureCursorVisible();
        if (ui->logMessagePlainTextEdit->toPlainText().toUpper().indexOf(searchString,infectedStart) == -1) infectedStart = 0;
    }
}
