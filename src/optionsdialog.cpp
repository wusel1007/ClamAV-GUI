#include "optionsdialog.h"
#include "ui_optionsdialog.h"

optionsDialog::optionsDialog(QWidget *parent) : QWidget(parent), ui(new Ui::optionsDialog)
{
    ui->setupUi(this);
    setupFile = new setupFileHandler(QDir::homePath() + "/.clamav-gui/settings.ini");
    updateDirectories();
    ui->tabWidget->setTabText(0,tr("Options"));
    ui->tabWidget->setTabText(1,tr("Directories"));
    scanLimits = new scanLimitsTab(this);
    ui->tabWidget->addTab(scanLimits,QIcon(":/icons/icons/scanlimits.png"),tr("Scan Limitations"));
    incExcOptTab = new includeExcludeOptions(this);

    connect(incExcOptTab,SIGNAL(updateClamdConf()),this,SLOT(slot_updateClamdConf()));
    connect(scanLimits,SIGNAL(updateClamdConf()),this,SLOT(slot_updateClamdConf()));

    ui->tabWidget->addTab(incExcOptTab,QIcon(":/icons/icons/includeexclude.png"),tr("Include/Exclude"));
    ui->tabWidget->setIconSize(QSize(24,24));

    getClamscanParametersProcess = new QProcess(this);
    connect(getClamscanParametersProcess,SIGNAL(readyReadStandardError()),this,SLOT(slot_getClamscanProcessHasOutput()));
    connect(getClamscanParametersProcess,SIGNAL(readyReadStandardOutput()),this,SLOT(slot_getClamscanProcessHasOutput()));
    connect(getClamscanParametersProcess,SIGNAL(finished(int)),this,SLOT(slot_getClamscanProcessFinished()));

    QStringList parameters;
    parameters << "--help";

    getClamscanParametersProcess->start("clamscan",parameters);
}

optionsDialog::~optionsDialog()
{
    delete ui;
}

void optionsDialog::slot_updateClamdConf() {
    emit updateClamdConf();
}

void optionsDialog::slot_getClamscanProcessHasOutput()
{
    getClamscanProcessOutput = getClamscanProcessOutput + getClamscanParametersProcess->readAll();
}

void optionsDialog::slot_getClamscanProcessFinished()
{
    QString excludeList =  "--help|--version|--database|--log|--file-list|--move|--copy|--exclude|--exclude-dir|--include|--include-dir|";
    excludeList += "--bytecode-timeout|--statistics|--exclude-pua|--include-pua|--structured-ssn-format|--structured-ssn-count|--structured-cc-count|--tempdir|--follow-dir-symlinks|";
    excludeList += "--structured-cc-mode|--max-scantime|--max-filesize|--max-scansize|--max-files|--max-recursion|--max-dir-recursion|--max-embeddedpe|--max-htmlnormalize|--follow-file-symlinks|";
    excludeList += "--max-htmlnotags|--max-scriptnormalize|--max-ziptypercg|--max-partitions|--max-iconspe|--max-rechwp3|--pcre-match-limit|--pcre-recmatch-limit=#n|--pcre-max-filesize=#n";
    excludeList += "";
    getClamscanProcessOutput = getClamscanProcessOutput + getClamscanParametersProcess->readAll();

    QString value;
    QString line;
    QString keyword;
    QString parameter;
    int commentStart = 0;
    QStringList lines = getClamscanProcessOutput.split("\n");
    QStringList comments;
    QString comment;

    setupFile->removeSection("AvailableOptions");

    for (int x = 0; x < lines.size(); x++) {
        line = lines[x];
        line = line.trimmed();
        if (line.indexOf("Clam AntiVirus:") != -1) {
            QString version = line.mid(line.indexOf("Clam AntiVirus:") + 15);
            if ((version.indexOf("Scanner 1.4.1") != -1) || (version.indexOf("Scanner 1.0.7") != -1)) {
              QFile ca_certFile("/etc/pki/tls/certs/ca-bundle.crt");
              QString message = "WARNING\nThe file \"/etc/pki/tls/certs/ca-bundle.crt\" is missing!\nVersion 1.0.7 and 1.4.1 are known to require this file in this specific locaction and without it \"freshclam\" will not work.\nInstall the appropriate package for your distribution.";
              QFile ca_alternative_location ("/etc/ssl/ca-bundle.pem");
              if (ca_alternative_location.exists() == true) message = "WARNING\nThe file \"/etc/pki/tls/certs/ca-bundle.crt\" is missing!\nVersion 1.0.7 and 1.4.1 are known to require this file in this specific locaction and without it \"freshclam\" will not work.\n\nFound the file in the location \"/etc/ssl/ca-bundle.pem\".\n\nDo the following command as root (sudo) to fix this issue:\n\nln -s /etc/ssl/ca-bundle.pem /etc/pki/tls/certs/ca-bundle.crt";
              ca_alternative_location.setFileName("/et/ssl/certs/ca-certificates.crt");
              if (ca_alternative_location.exists() == true) message = "WARNING\nThe file \"/etc/pki/tls/certs/ca-bundle.crt\" is missing!\nVersion 1.0.7 and 1.4.1 are known to require this file in this specific locaction and without it \"freshclam\" will not work.\n\nFound the file in the location \"/etc/ssl/ca-bundle.pem\".\n\nDo the following command as root (sudo) to fix this issue:\n\nln -s /etc/ssl/certs/ca-certificates.crt /etc/pki/tls/certs/ca-bundle.crt";
              if (ca_certFile.exists() == false) QMessageBox::warning(this,"WARNING",message);
            }
            ui->versionLabel->setText("ClamAV Version:" + version);
        }

        if (line.indexOf("--") == 0) {
            commentStart = line.indexOf(" ");
            keyword = line.left(commentStart);
            parameter = "";
            if (keyword.indexOf("[=") != -1) {
                keyword = keyword.left(keyword.indexOf("["));
                int start = line.indexOf("[") + 2;
                int length = line.indexOf("]") - start;
                parameter = line.mid(start,length);
            } else {
                keyword = keyword.left(keyword.indexOf("="));
            }
            if (excludeList.indexOf(keyword) == -1) {
                if (value == "") value = keyword; else value = value + "\n" + keyword;
                if (parameter != "") {
                    value = value + "=" + parameter;
                    //if ((parameter == "yes/no(*)") ||(parameter == "yes(*)/no")) {
                    //    value = value + "=yes/no";
                    //}
                }
                comment = line.mid(commentStart);
                comment = comment.trimmed();
                while (comment.indexOf("  ") != -1) comment = comment.replace("  "," ");
                if (comment.left(1) == "-") {
                    comment = comment.mid(comment.indexOf(" ") + 1);
                }
                comment = tr(comment.toLocal8Bit());
                comments.append(comment);
                if ((parameter == "yes/no(*)") ||(parameter == "yes(*)/no")) {
                    if (parameter == "yes/no(*)") {
                        setupFile->setSectionValue("AvailableOptions",keyword + "<equal>no",comment);
                        setupFile->setSectionValue("AvailableOptions",keyword + "<equal>yes",comment);
                    } else {
                        setupFile->setSectionValue("AvailableOptions",keyword + "<equal>yes",comment);
                        setupFile->setSectionValue("AvailableOptions",keyword + "<equal>no",comment);
                    }
                } else {
                    setupFile->setSectionValue("AvailableOptions",keyword,comment);
                }
            }
        }
    }

    QStringList parameters = value.split("\n");
    scanoption * option;
    scanoptionyn * optionyn;
    QString label;
    QString yes_no;
    bool flipflop = false;

    for (int x = 0; x < parameters.length(); x++) {
        label = parameters[x];
        if ((label.indexOf("yes(*)/no") != -1) || (label.indexOf("yes/no(*)") != -1)){
            label.indexOf("yes/no(*)") == -1?yes_no="yes":yes_no="no";
            label = label.left(label.indexOf("="));
            if (setupFile->keywordExists("SelectedOptions",label+"<equal>no") == true) {
                label = label + "<equal>no";
            } else {
                if (setupFile->keywordExists("SelectedOptions",label+"<equal>yes") == true) {
                    label = label + "<equal>yes";
                } else {
                    if (yes_no == "yes") {
                        label = label + "<equal>yes";
                    } else {
                        label = label + "<equal>no";
                    }
                }
            }
        }
        if (setupFile->keywordExists("SelectedOptions",label.replace("=","<equal>")) == true) {
            if (label.indexOf("<equal>") == -1) {
                option = new scanoption(nullptr, QDir::homePath() + "/.clamav-gui/settings.ini","SelectedOptions",true,label,comments[x]);
                if (flipflop == false) {
                    ui->optionLayout->addWidget(option);
                    flipflop = true;
                } else {
                    ui->optionLayout_2->addWidget(option);
                    flipflop = false;
                }
            } else {
                optionyn = new scanoptionyn(nullptr, QDir::homePath() + "/.clamav-gui/settings.ini","SelectedOptions",true,label,comments[x]);
                if (flipflop == false) {
                    ui->optionLayout->addWidget(optionyn);
                    flipflop = true;
                } else {
                    ui->optionLayout_2->addWidget(optionyn);
                    flipflop = false;
                }
            }
        } else {
            if (label.indexOf("<equal>") == -1) {
                option = new scanoption(nullptr, QDir::homePath() + "/.clamav-gui/settings.ini","SelectedOptions",false,label,comments[x]);
                if (flipflop == false) {
                    ui->optionLayout->addWidget(option);
                    flipflop = true;
                } else {
                    ui->optionLayout_2->addWidget(option);
                    flipflop = false;
                }
            } else {
                optionyn = new scanoptionyn(nullptr, QDir::homePath() + "/.clamav-gui/settings.ini","SelectedOptions",false,label,comments[x]);
                if (flipflop == false) {
                    ui->optionLayout->addWidget(optionyn);
                    flipflop = true;
                } else {
                    ui->optionLayout_2->addWidget(optionyn);
                    flipflop = false;
                }
            }
        }
    }

}

QString optionsDialog::getCopyDirectory()
{
    return ui->copyDirectoryLineEdit->text();
}

QString optionsDialog::getMoveDirectory()
{
    return ui->moveDirectoryLineEdit->text();
}

void optionsDialog::slot_selectLVDButtonClicked(){
QString rc;
    rc = QFileDialog::getExistingDirectory(this,tr("Select Directory"),QDir::homePath() + "/.clamav-gui/signatures",QFileDialog::ShowDirsOnly);
    if (rc != "") {
        ui->loadVirusDatabaseLineEdit->setText(rc);
        writeDirectories();
        emit systemStatusChanged();
        if (ui->loadVirusDatabaseCheckBox->isChecked() == true) {
            emit databasePathChanged(rc);
            QFile file(rc + "/freshclam.dat");
            if (file.exists() == false) {
                if (QMessageBox::warning(this,tr("Virus definitions missing!"),tr("No virus definitions found in the database folder. Should the virus definitions be downloaded?"),QMessageBox::Yes,QMessageBox::No) == QMessageBox::Yes) {
                    emit updateDatabase();
                }
            }
        }
    }
}

void optionsDialog::slot_selectCDButtonClicked(){
QString rc;
    rc = QFileDialog::getExistingDirectory(this,tr("Select Directory"),QDir::homePath() + "/.clamav-gui/quarantine");
    if (rc != "") ui->copyDirectoryLineEdit->setText(rc);
    writeDirectories();
}

void optionsDialog::slot_selectMDButtonClicked(){
QString rc;
    rc = QFileDialog::getExistingDirectory(this,tr("Select Directory"),QDir::homePath() + "/.clamav-gui/quarantine");
    if (rc != "") ui->moveDirectoryLineEdit->setText(rc);

    writeDirectories();
}

void optionsDialog::slot_selectSRTFButtonClicked(){
    QString rc;
        rc = QFileDialog::getSaveFileName(this,tr("Select File"),QDir::homePath() + "/.clamav-gui/logs");
        if (rc != "") ui->scanReportToFileLineEdit->setText(rc);
        writeDirectories();
    }

void optionsDialog::slot_selectSFFButtonClicked(){
QString rc;
    rc = QFileDialog::getOpenFileName(this,tr("Select File"),QDir::homePath());
    if (rc != "") ui->scanFilesFromFileLineEdit->setText(rc);
    writeDirectories();
}

void optionsDialog::slot_selectTDButtonClicked(){
QString rc;
    rc = QFileDialog::getExistingDirectory(this,tr("Select Directory"),QDir::homePath());
    if (rc != "") ui->tempFilesLineEdit->setText(rc);
    writeDirectories();
}

void optionsDialog::updateDirectories(){
QString value;
QString checked;

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
}

void optionsDialog::writeDirectories(){
QString keyword;
QString value;
QString checked;

    keyword = "LoadSupportedDBFiles";
    value = ui->loadVirusDatabaseLineEdit->text();
    ui->loadVirusDatabaseCheckBox->isChecked() == true?checked="checked":checked="not checked";
    setupFile->setSectionValue("Directories",keyword,checked + "|" + value);

    keyword = "ScanReportToFile";
    value = ui->scanReportToFileLineEdit->text();
    ui->scanReportToFileCheckBox->isChecked() == true?checked="checked":checked="not checked";
    setupFile->setSectionValue("Directories",keyword,checked + "|" + value);

    keyword = "ScanFilesFromFile";
    value = ui->scanFilesFromFileLineEdit->text();
    ui->scanFilesFromFileCheckBox->isChecked() == true?checked="checked":checked="not checked";
    setupFile->setSectionValue("Directories",keyword,checked + "|" + value);

    keyword = "TmpFile";
    value = ui->tempFilesLineEdit->text();
    ui->tempFileCheckBox->isChecked() == true?checked="checked":checked="not checked";
    setupFile->setSectionValue("Directories",keyword,checked + "|" + value);

    keyword = "MoveInfectedFiles";
    value = ui->moveDirectoryLineEdit->text();
    ui->moveDirectoryCheckBox->isChecked() == true?checked="checked":checked="not checked";
    setupFile->setSectionValue("Directories",keyword,checked + "|" + value);

    keyword = "CopyInfectedFiles";
    value = ui->copyDirectoryLineEdit->text();
    ui->copyDirectoryCheckBox->isChecked() == true?checked="checked":checked="not checked";
    setupFile->setSectionValue("Directories",keyword,checked + "|" + value);

    keyword = "FollowDirectorySymLinks";
    value = QString::number(ui->followDirectorySymlinksComboBox->currentIndex());
    ui->followDirectorySymlinksCheckBox->isChecked() == true?checked="checked":checked="not checked";
    setupFile->setSectionValue("Directories",keyword,checked + "|" + value);

    keyword = "FollowFileSymLinks";
    value = QString::number(ui->followFileSymlinksComboBox->currentIndex());
    ui->followFileSymlinksCheckBox->isChecked() == true?checked="checked":checked="not checked";
    setupFile->setSectionValue("Directories",keyword,checked + "|" + value);

    if (ui->loadVirusDatabaseCheckBox->isChecked() == true) {
        if (ui->loadVirusDatabaseLineEdit->text() != "") {
            emit databasePathChanged(ui->loadVirusDatabaseLineEdit->text());
            QFile file(ui->loadVirusDatabaseLineEdit->text() + "/main.cvd");
            if (file.exists() == false) {
                if (QMessageBox::warning(this,tr("Database files missing!"),tr("The virus definition files are missing in the database directory. Start download of the missing files?"),QMessageBox::Yes,QMessageBox::No) == QMessageBox::Yes) {
                    emit updateDatabase();
                }
            }
        }
    } else {
        emit databasePathChanged("/var/lib/clamav");
    }
}

void optionsDialog::slot_updateDirectories(){
    updateDirectories();
}

void optionsDialog::slot_logViewerButtonClicked(){
QFile file;

    if ((ui->scanReportToFileCheckBox->isChecked() == true) & (ui->scanReportToFileLineEdit->text() != "")){
        file.setFileName(ui->scanReportToFileLineEdit->text());
        if (file.exists()){
            logViewObject * logView = new logViewObject(this,ui->scanReportToFileLineEdit->text());
            logView->setModal(true);
            logView->showMaximized();
        }
    }
}
