#include "scanoption.h"
#include "ui_scanoption.h"

scanoption::scanoption(QWidget *parent, QString setupFileName, QString section, bool checked, QString label, QString comment) :
    QWidget(parent),
    ui(new Ui::scanoption)
{
    ui->setupUi(this);
    setupFile = new setupFileHandler(setupFileName);
    setupFileHandler * baseSetup = new setupFileHandler(QDir::homePath() + "/.clamav-gui/settings.ini");
    languageset = baseSetup->getSectionValue("Setup","language");
    trans = new translator(languageset);

    setupFileSection = section;
    option = label;
    com = trans->translateit(comment);
    ui->checkBox->setChecked(checked);
    if (checked == true) {
        this->setStyleSheet("background-color:lightgreen");
        setupFile->setSectionValue(setupFileSection,option,com);
    }
    ui->checkBox->setText(com);
    ui->checkBox->setToolTip(option);
}

scanoption::~scanoption()
{
    delete ui;
}


void scanoption::slot_checkboxClicked()
{
    if (ui->checkBox->isChecked() == false) {
        setupFile->removeKeyword(setupFileSection,option);
        this->setStyleSheet("");
    } else {
        setupFile->setSectionValue(setupFileSection,option,com);
        this->setStyleSheet("background-color:lightgreen");
    }
}
