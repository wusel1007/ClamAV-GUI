#include "partiallogobject.h"
#include "ui_partiallogobject.h"

partialLogObject::partialLogObject(QWidget *parent, QString logText) : QWidget(parent), ui(new Ui::partialLogObject)
{
    ui->setupUi(this);
    logHighlighter = new highlighter(ui->logPlainText->document());
    setLogText(logText);
    start = 0;
    errorStart = 0;
    infectedStart = 0;
}

partialLogObject::~partialLogObject()
{
    delete ui;
}

void partialLogObject::setLogText(QString logText){
QString engine;
QString scannedDirs;
QString scannedFiles;
QString errors;
QString infectedFiles;
int pos;

    ui->logPlainText->setPlainText(logText);
    pos = logText.indexOf("Engine version:");
    if (pos > -1){
        engine = logText.mid(pos + 15,logText.indexOf("\n",pos + 15) - (pos + 15));
    } else {
        engine = "n/a";
    }
    pos = logText.indexOf("Scanned directories:");
    if (pos > -1){
        scannedDirs = logText.mid(pos + 20,logText.indexOf("\n",pos + 20) - (pos + 20));
    } else {
        scannedDirs = "n/a";
    }
    pos = logText.indexOf("Scanned files:");
    if (pos > -1){
        scannedFiles = logText.mid(pos + 14,logText.indexOf("\n",pos + 14) - (pos + 14));
    } else {
        scannedFiles = "n/a";
    }
    pos = logText.indexOf("Total errors:");
    if (pos > -1){
        errors = logText.mid(pos + 13,logText.indexOf("\n",pos + 13) - (pos + 13));
    } else {
        errors = "n/a";
    }
    pos = logText.indexOf("Infected files:");
    if (pos > -1){
        infectedFiles = logText.mid(pos + 15,logText.indexOf("\n",pos + 15) - (pos + 15));
    } else {
        infectedFiles = "n/a";
    }

    ui->engineVersionLabel->setText(tr("Engine Version: ") + engine);
    ui->scannedDirectoriesLabel->setText(tr("Scanned Directories: ") + scannedDirs);
    ui->scannedFilesLabel->setText(tr("Scanned Files: ") + scannedFiles);
    ui->totalErrorsLabel->setText(tr("Total Errors: ") + errors);
    ui->infectedFilesLabel->setText(tr("Infected Files: ") + infectedFiles);
}

QString partialLogObject::getLogText(){
    return "<Scanning startet> " + ui->logPlainText->toPlainText();
}

void partialLogObject::slot_searchButtonClicked(){
QTextCursor cursor = ui->logPlainText->textCursor();
QString searchString = ui->searchTextLineEdit->text();
int pos = ui->logPlainText->toPlainText().toUpper().indexOf(searchString.toUpper(),start);

    if (pos > -1){
        ui->searchButton->setText(tr("continue"));
        start = pos + searchString.length();
        cursor.movePosition(QTextCursor::Start);
        cursor.movePosition(QTextCursor::Right,QTextCursor::MoveAnchor,pos);
        cursor.movePosition(QTextCursor::Right,QTextCursor::KeepAnchor,searchString.length());
        ui->logPlainText->setTextCursor(cursor);
        ui->logPlainText->ensureCursorVisible();
    } else {
        if (QMessageBox::question(this,"INFO",tr("Searchstring not found!\nContinue from the Start of the Log?"),QMessageBox::Yes,QMessageBox::No) == QMessageBox::Yes){
            start = 0;
            pos = ui->logPlainText->toPlainText().toUpper().indexOf(searchString.toUpper(),start);
            if (pos > -1){
                ui->searchButton->setText(tr("continue"));
                start = pos + searchString.length();
                cursor.movePosition(QTextCursor::Start);
                cursor.movePosition(QTextCursor::Right,QTextCursor::MoveAnchor,pos);
                cursor.movePosition(QTextCursor::Right,QTextCursor::KeepAnchor,searchString.length());
                ui->logPlainText->setTextCursor(cursor);
                ui->logPlainText->ensureCursorVisible();
            } else {
                QMessageBox::information(this,tr("INFO"),tr("Searchstring not found!"));
            }
        }
    }

}

void partialLogObject::slot_searchLineEditChanged(){
    start = 0;
    ui->searchButton->setText(tr("search"));
}

void partialLogObject::slot_clearLineEditButtonClicked(){
    ui->searchTextLineEdit->setText("");
    start = 0;
}

void partialLogObject::slot_totalErrorButtonClicked(){
QTextCursor cursor = ui->logPlainText->textCursor();
QStringList searchStrings;
QString searchString;
int pos = -1;
int index = 0;

    searchStrings << "Access denied" << "Empty file";

    while ((pos == -1) & (index < searchStrings.count())){
        searchString = searchStrings.at(index);
        index ++;
        pos = ui->logPlainText->toPlainText().indexOf(searchString,errorStart);
    }

    if (pos > -1){
        errorStart = pos + searchString.length();
        cursor.movePosition(QTextCursor::Start);
        cursor.movePosition(QTextCursor::Right,QTextCursor::MoveAnchor,pos);
        cursor.movePosition(QTextCursor::Right,QTextCursor::KeepAnchor,searchString.length());
        ui->logPlainText->setTextCursor(cursor);
        ui->logPlainText->ensureCursorVisible();
        if (ui->logPlainText->toPlainText().indexOf(searchString,errorStart) == -1) errorStart = 0;
    }
}

void partialLogObject::slot_infectedFilesButtonClicked(){
QTextCursor cursor = ui->logPlainText->textCursor();
QString searchString = "FOUND";
int pos = ui->logPlainText->toPlainText().indexOf(searchString,infectedStart);

    if (pos > -1){
        infectedStart = pos + searchString.length();
        cursor.movePosition(QTextCursor::Start);
        cursor.movePosition(QTextCursor::Right,QTextCursor::MoveAnchor,pos);
        cursor.movePosition(QTextCursor::Right,QTextCursor::KeepAnchor,searchString.length());
        ui->logPlainText->setTextCursor(cursor);
        ui->logPlainText->ensureCursorVisible();
        if (ui->logPlainText->toPlainText().indexOf(searchString,infectedStart) == -1) infectedStart = 0;
    }
}
